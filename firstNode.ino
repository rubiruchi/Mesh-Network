#include "ESP8266WiFi.h"
#include <String.h>

#define NODE_NUMBER 1
#define SCAN_PERIOD 5000

//Variables for debugging
const char* ipDebug = "";

String received_data = "";
String default_ssid_name = "MeshNode";
String sensor_data = "Ola";
const int port_number = 80;

boolean foundConnection = false;
boolean connectedClientTest = false;

//Initializing client and server
WiFiClient client;
WiFiServer server(port_number);
IPAddress connectedNode_staticIP;

//Configuring network
String ssid;
String password;
IPAddress local_IP(192,168,4,NODE_NUMBER);
IPAddress gateway(192,168,4,8);
IPAddress subnet(255,255,255,0);
//String connectedNode_staticIP;

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
  
  //Setting wifi mode
  WiFi.mode(WIFI_AP_STA);
  
  //Configuring softAP
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
  
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  WiFi.disconnect();
  
} //end of setup

void loop()
{
  boolean Client_Sending_Data_Connected = false;
  delay (1000);
  received_data = sensor_data;
  
//if a node has sent data to this node, a connection to the next one will be established
//  if (Client_Sending_Data_Connected)
//  {
    if (!WiFi.isConnected())
    {
      foundConnection = getStrongerSignal();
      if (foundConnection)
      {
        Serial.println(stronger_WifiSSID);
        WiFi.begin(stronger_WifiSSID.c_str(), stronger_WifiSSID.c_str());
        delay(5000);
        
        //Configuring IP address that will be used to send data to server
        //connectedNode_staticIP = IPAddress(192,168,1,connectedNodeNumber);
        if (WiFi.status() != WL_CONNECTED)
        {
          //delay(2000);
          Serial.print(".");
        }
        else
        {
          int connectedNodeNumber = int (stronger_WifiSSID.charAt(8) - '0');
          connectedNode_staticIP = IPAddress(192,168,4,connectedNodeNumber);
          //ipDebug = connectedNode_staticIP.toString().c_str();
          Serial.println("Wifi connected");
          Serial.println("IP address:");
          Serial.println(connectedNode_staticIP.toString());
        }
      }
    } //end if isConnected
    else 
    {
      Serial.println("\nStarting connection with the server...");
      // if you get a connection, report back via serial:
     
      /* 
              boolean connectedClientTest;
        int counter = 0;
        do {
            connectedClientTest = client.connect(connectedNode_staticIP, port_number);
            if (connectedClientTest) { break; }
            client.stop();
            delay(1000);
            Serial.println("retry");
            counter++;
        } while (!connectedClientTest && counter <5);
        
        */
      
      connectedClientTest = client.connect(connectedNode_staticIP, port_number);
      delay(1000);
      if (connectedClientTest) {
        Serial.println("Client connected");
        client.println("Node sending message to server");
        client.println(received_data);
        client.stop();
        connectedClientTest = false;
        //WiFi.disconnect();
      }
      else
      {
        Serial.println ("It did not get a connection");
        //client.stop();
      }
     // }//end else isConnected 
  }//end if sending_data_connected
  
}//end of loop
