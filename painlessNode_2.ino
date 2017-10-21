#include "painlessMesh.h"
#include <stdlib.h>
#include <TimeLib.h>
#include <Time.h>

#define   MESH_PREFIX     "MeshNode"
#define   MESH_PASSWORD   "MeshNode"
#define   MESH_PORT       80
#define   LAST_NODE_ID    2786574637

// initializes or defines the output pin of the LM35 temperature sensor
int outputPin= A0;
String sensorData;
uint32_t last_node_id = LAST_NODE_ID;
time_t sendingTime;

void sendMessage() ;

painlessMesh  mesh;
Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

String floatToString(float val) {
  int i;
  char buff[10];
  String valueString = "";
  for (i = 0; i < 10; i++) 
  {
    dtostrf(val, 4, 6, buff);  //4 is mininum width, 6 is precision
    valueString += buff;
  }
  return valueString;
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  unsigned long ping = now() - sendingTime;
  Serial.printf("Ping: %lu \n", ping);
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
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
}

void loop() {
  mesh.update();
  int analogValue = analogRead(outputPin);
  float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
  float sensorValue = millivolts/10;
  sensorData = floatToString(sensorValue);
  Serial.print("in DegreeC=   ");
  Serial.println(sensorData);
}

void sendMessage() {
  String msg = sensorData;
  mesh.sendSingle(last_node_id, msg);
  sendingTime = now();
  taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));
}

