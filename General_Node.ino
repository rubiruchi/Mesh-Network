#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <String.h>

#define NODE_NUMBER 2
#define SCAN_PERIOD 5000

//Those are the variables that ate going to end the conflict between the station configs and the AP configs
extern "C" void* eagle_lwip_getif(uint8_t);
extern "C" void netif_set_up(void*);
extern "C" void netif_set_down(void*);

String previous_node = "0";
String default_ssid_name = "MeshNode";
const uint16_t node_port = 80 + NODE_NUMBER;
unsigned int next_node_port = 80;

char incomingPacket[255];
char  replyPacket[] = "Message received";
char* received_data;

boolean foundConnection = false;
boolean connectedClientTest = false;
boolean node_connected = false;

//Initializing client and server
WiFiUDP Udp;
WiFiClient client;
WiFiServer server(node_port);

//Variables that will be used to configure the network
String ssid;
String password;
IPAddress local_IP(192,168,4,NODE_NUMBER);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

//Variables that describe the stronger wifi signal
int stronger_WifiSignal = -200;
String stronger_WifiSSID = "";
IPAddress connectedNode_staticIP;

//Function that will scan available network and get the closer node
boolean getStrongerSignal ()
{
  //Get the stronger signal and connects to network
  boolean foundMeshNetwork = false;
  
  Serial.print("\nScan start ... ");
  WiFi.scanNetworks(true);
  delay(SCAN_PERIOD);
  
  // print out Wi-Fi network scan result uppon completion
  int n = WiFi.scanComplete();
  
  if(n > 0)
  {
    Serial.printf("%d network(s) found\n", n);
    for (int i = 0; i < n; i++)
    {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i+1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
      
      //Checks which one has the best signal, as long as it is one of the nodes 
      if (stronger_WifiSignal < WiFi.RSSI(i) && WiFi.SSID(i).startsWith("MeshNode") && !(WiFi.SSID(i).endsWith(previous_node)))
      { 
        foundMeshNetwork = true;
        stronger_WifiSignal = WiFi.RSSI(i);
        stronger_WifiSSID = WiFi.SSID(i);
      }
    }
    WiFi.scanDelete();
    if (!foundMeshNetwork)
      return false;
    return true;
  }
  else if (n = 0)
    Serial.printf ("No network found\n"); 
  WiFi.scanDelete();
  return false;
    
} //getStrongerSignal ends

void setup()
{
  ssid = default_ssid_name;
  ssid.concat(NODE_NUMBER);
  password = ssid;
  
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  
  //Configuring softAP 
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready IP config" : "Failed! IP config");
  
  Serial.print("Setting soft-AP ... "); 
  boolean result = false;
  while (!result)
  {
    result = WiFi.softAP(ssid.c_str(), password.c_str());
    if(result == true)
      Serial.println("Ready");
    else
      Serial.println("Failed!");
      delay(100);
  }
  
  //Debugging
  IPAddress this_node_ip = WiFi.softAPIP();   // Obtain the IP of the Server 
  Serial.print("Server IP is: ");             // Print the IP to the monitor window 
  Serial.println(this_node_ip);
  
  //Initializng server
  server.begin();
  delay (2000);
  Udp.begin(node_port);
  
} //end of setup

void loop()
{
  Serial.println("Checking for packets");
  delay (1000);
  // listen for incoming clients
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    previous_node = Udp.remoteIP().toString().substring(10);
    Serial.println(previous_node);
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    received_data = incomingPacket;
    
    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyPacket);
    Udp.endPacket();
    node_connected = true;
  }
  else
  {
    Serial.println("No packet was received.");
    node_connected = false;
  }
  
  //if a node has sent data to this node, a connection to the next one will be established
  //in this case, this part is not being used, since the tests are being done with two nodes
  //this one is the server
  if (node_connected)
  {
      if (!WiFi.isConnected())
      {
        foundConnection = getStrongerSignal();
        if (foundConnection)
        {
          WiFi.begin(stronger_WifiSSID.c_str(), stronger_WifiSSID.c_str());
          delay (2000);
          //Configuring IP address that will be used to send data to server
          unsigned int connectedNodeNumber = int (stronger_WifiSSID.charAt(8) - '0');
          next_node_port = next_node_port + connectedNodeNumber;
          connectedNode_staticIP = IPAddress(192,168,1,connectedNodeNumber);
          if (WiFi.status() != WL_CONNECTED)
          {
            Serial.print(".");
          }
          Serial.println("Wifi connected");
        }
      } //end if isConnected
      else 
      {
        // if you get a connection, report back via serial:
        Serial.println("\nStarting connection with the server...");
        
        // before doing HTTP request, disable SoftAP interface:
        netif_set_down(eagle_lwip_getif(1));
        
        Udp.beginPacket(Udp.remoteIP(), next_node_port);
        Udp.write(received_data);
        Udp.endPacket();
        
        //now enable SoftAP interface again
        netif_set_up(eagle_lwip_getif(1));
        
      }//end else isConnected 
  }//end if node_connected
}//end of loop
