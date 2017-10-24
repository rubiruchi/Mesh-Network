#include "painlessMesh.h"
#include <stdlib.h>
#include <TimeLib.h>
#include <Time.h>

#define   MESH_PREFIX     "MeshNode"
#define   MESH_PASSWORD   "MeshNode"
#define   MESH_PORT       80
#define   LAST_NODE_ID    2786574637
#define   INTERVAL        10000 //sends message every 10000
#define   READING_DATA_INTERVAL   2

// initializes or defines the output pin of the LM35 temperature sensor
int outputPin= A0;
boolean newConnection = false;
String sensorData;
uint32_t last_node_id = LAST_NODE_ID;
unsigned long sendingTime, acknowledgeTime;
time_t startingTime;

void sendMessage() ;

painlessMesh  mesh;
Task taskSendMessage(INTERVAL , TASK_FOREVER, &sendMessage);

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Packet: Received from %u msg=%s\n", from, msg.c_str());
  //acknowledgeTime = millis();
  //Serial.println(acknowledgeTime);
  //long ping = acknowledgeTime - sendingTime;
  //Serial.println(ping);
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
    newConnection = true;
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, AUTH_WPA2_PSK, 1, PHY_MODE_11G, 82, 0, 1);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.scheduler.addTask( taskSendMessage );
  taskSendMessage.enable() ;
  startingTime = now();
  ESP.wdtFeed();
}

void loop() {
  if (difftime(now(), startingTime) >= READING_DATA_INTERVAL)
  {
    ESP.wdtFeed();
    int analogValue = analogRead(outputPin);
    float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
    float sensorValue = millivolts/10;
    sensorData = String(sensorValue, 1);
    startingTime = now();
    //Serial.println(sensorData);
  }
  mesh.update();
}

void sendMessage() {
  String msg = sensorData;
  mesh.sendSingle(last_node_id, msg);
  Serial.println("Sent: " + sensorData);
  //sendingTime = millis();
  //Serial.println(sendingTime);
  //taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));
}
