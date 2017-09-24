#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <String.h>

#define NODE_NUMBER 1
#define SCAN_PERIOD 5000

//Those are the variables that ate going to end the conflict between the station configs and the AP configs
extern "C" void* eagle_lwip_getif(uint8_t);
extern "C" void netif_set_up(void*);
extern "C" void netif_set_down(void*);


//Variables for debugging
const char* ipDebug = "";

String default_ssid_name = "MeshNode";
const uint16_t node_port = 80 + NODE_NUMBER;

unsigned int next_node_port = 80;
char incomingPacket[255];
char  replyPacket[] = "Message received";
char received_data[] = "Ola";
boolean foundConnection = false;
boolean node_was_connected = false;

//Initializing client and server
WiFiUDP Udp;
WiFiClient client;
WiFiServer server(node_port);
IPAddress connectedNode_staticIP;

//Configuring network
String ssid;
String password;
IPAddress local_IP(192,168,4,NODE_NUMBER);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

//Variables that describe the stronger wifi signal
int stronger_WifiSignal = -200;
String stronger_WifiSSID = "";

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
      if (stronger_WifiSignal < WiFi.RSSI(i) && WiFi.SSID(i).startsWith("MeshNode"))
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
  Serial.begin(115200);
  Serial.println();

  ssid = default_ssid_name;
  ssid.concat(NODE_NUMBER);
  password = ssid;

  WiFi.disconnect();
  
  //Setting wifi mode
  WiFi.mode(WIFI_AP_STA);
  
  IPAddress station_ip (192,168,4,NODE_NUMBER);
  IPAddress station_gateway(192,168,4,9);
  IPAddress station_subnet (255,255,255,0);
  WiFi.config(station_ip,station_gateway,station_subnet);
  delay (2000);

} //end of setup

void loop()
{
  boolean Client_Sending_Data_Connected = false;
  delay (1000);
  Serial.println ("Loop begins and print data");
  Serial.println (received_data);
  
//  if (Client_Sending_Data_Connected)
//  {
      if (!WiFi.isConnected())
      {
        if (node_was_connected)
        {
          WiFi.disconnect();
        }
        node_was_connected = false;
        foundConnection = getStrongerSignal();
        if (foundConnection)
        {
          Serial.println(stronger_WifiSSID);
          WiFi.begin(stronger_WifiSSID.c_str(), stronger_WifiSSID.c_str());
          delay(5000);
          if (WiFi.status() != WL_CONNECTED)
            Serial.print(".");
          else
          {
            unsigned int connectedNodeNumber = int(stronger_WifiSSID.charAt(8) - '0');
            next_node_port = next_node_port + connectedNodeNumber;
            connectedNode_staticIP = IPAddress(192,168,4,connectedNodeNumber);
            Serial.println("Wifi connected");
            Serial.println("IP address:");
            Serial.println(connectedNode_staticIP.toString());
            Udp.begin(node_port);
          }
        }
      } //end if isConnected
      else 
      {
        node_was_connected = true;
        Serial.println("\nStarting connection with the server...");
          // before doing HTTP request, disable SoftAP interface:
          netif_set_down(eagle_lwip_getif(1));
          
          Udp.beginPacket(connectedNode_staticIP, next_node_port);
          Udp.write(received_data);
          Udp.endPacket();

          //Acknownledge
          int packetSize = Udp.parsePacket();
          if (packetSize)
          {
            Serial.printf("Received %d bytes from %s, port %d \n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
            int len = Udp.read(incomingPacket, 255);
            if (len > 0)
            {
              incomingPacket[len] = 0;  
            }  
            Serial.printf("UDP packet contents: %s \n", incomingPacket);
          }
                    
          // now enable SoftAP interface again:
          netif_set_up(eagle_lwip_getif(1));
  
       }  //end else isConnected 
   //}  //end if sending_data_connected 
} //end of loop
