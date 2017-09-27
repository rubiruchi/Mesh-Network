#include "painlessMesh.h"

#define   MESH_PREFIX     "MeshNode"
#define   MESH_PASSWORD   "MeshNode"
#define   MESH_PORT       80
#define   LAST_NODE_ID    2786574637

String sensor_data = "test";
uint32_t last_node_id = LAST_NODE_ID;

void sendMessage() ;

painlessMesh  mesh;
Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  mesh.sendSingle(from, "Received");

  
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

  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.scheduler.addTask( taskSendMessage );
  taskSendMessage.enable() ;
}

void loop() {
  mesh.update();
}

void sendMessage() {
  String msg = sensor_data;
  //msg += mesh.getNodeId();
  mesh.sendSingle(last_node_id, msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}
