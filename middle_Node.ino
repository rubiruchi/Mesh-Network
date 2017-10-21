#include "painlessMesh.h"

#define   MESH_PREFIX     "MeshNode"
#define   MESH_PASSWORD   "MeshNode"
#define   MESH_PORT       80
#define   LAST_NODE_ID    2786574637

uint32_t last_node_id = LAST_NODE_ID;

painlessMesh  mesh;

void receivedCallback(uint32_t from, String &msg) {
  String reply = "Received";
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  mesh.sendSingle(from, reply);
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
  mesh.getNodeList();
}

void loop() {
  mesh.update();
}

