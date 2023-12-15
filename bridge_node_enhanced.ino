#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <espnow.h>

/*
  Implementation:
    This is typical implementation for a bridge node that receives and publishes data.
    In our case, the pressure on a chair and the informations received via MQTT.

  Communication: 
    This ESP8266 (Bridge Node) connects to other ESP8266 (Sensing Nodes) through ESP-Now.
    This ESP8266 (Bridge Node) connects to client through MQTT.

  Goal:
    It publishes the state of a button and transfer informations obtained through MQTT.
*/

// --------------------------------------------------------------------------------------------
//        UPDATE CONFIGURATION TO MATCH YOUR ENVIRONMENT
// --------------------------------------------------------------------------------------------

// MAC Address of the Sensing Nodes
uint8_t sensing_list[][6] = {
  //{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Example Sensing Node
  {0x08, 0x3A, 0x8D, 0xE3, 0xDF, 0x26}, // First Sensing Node
  {0x34, 0x94, 0x54, 0x97, 0x5A, 0xD8}, // Second Sensing Node
  // ... add more rows as needed
};

// Init the number of Sensing Nodes use for the implementation
int amount = 2;

// MQTT connection details
#define MQTT_HOST "test.mosquitto.org" // or "broker.hivemq.com"
//subcribe to topic on laptop: 
//mosquitto_sub -h test.mosquitto.org -t h00448974/evt/status/fmt/json -q 0
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:hwu:esp8266:h00448974"
#define MQTT_USER ""
#define MQTT_TOKEN ""
#define MQTT_TOPIC "h00448974/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "h00448974/cmd/display/fmt/json"

// WiFi connection information
char ssid[] = "RALT_RIoT";  // your network SSID (name)
char pass[] = "raltriot";  // your network password

// Init loop interval time
int dataInterval = 10;

// Init Deep sleep (in microseconds)
int disabledNetwork = 60000000; // Deep sleep to disable the network

// --------------------------------------------------------------------------------------------
//        SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS LINE 
//        APART FOR THESE LINE: 
//          struct_receive_data sensors[sensing_node_amount];
//
//          status["Sensor_1"] = sensors[0].state;
//          status["Sensor_2"] = sensors[1].state;
// --------------------------------------------------------------------------------------------

// Temporary array to store the MAC Address of the Sensing Nodes one at a time
uint8_t tempRow[6];

// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, espClient);

// Variables to hold data
StaticJsonDocument<100> jsonDoc;
StaticJsonDocument<100> receivedJsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[50];

// Store sleep data
int dataSleep = 0;

// Check sensor number for deepSleep
int lastSensor = 1;

// Check from which sensor the data was received from
int sensorID;

// Structure of Sent Data
typedef struct struct_send_data {
  int sleep;
} struct_send_data;

// Create a struct_send_data called espDeepSleep
struct_send_data espDeepSleep;

// Callback when data is sent to Sensing Nodes
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");

    if (dataSleep == disabledNetwork) {
      if (lastSensor == amount) {
        delay(1000);
      
        // Exit deep sleep
        ESP.deepSleep(dataSleep); 
      }
      
      lastSensor += 1;
    }
  }
  else {
    Serial.println("Delivery fail");
  }
}

// Structure of Received Data
typedef struct struct_receive_data {
    int id;
    int state;
} struct_receive_data;

// Create a struct_receive_data called myData to temporarily store received data
struct_receive_data myData;

// Create a struct_receive_data called sensors that store each Sensing Node
struct_receive_data sensors[2];

// Callback when data is received from Sensing Nodes
void OnDataReceived(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  // Print the MAC adress of the corresponding Sensor Node
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %d: %u bytes\n", myData.id, len);

  sensorID = myData.id;
  sensorID = sensorID - 1;
  
  sensors[sensorID].id = myData.id;
  sensors[sensorID].state = myData.state;

  // Update the structure with the new incoming data
  Serial.printf("Message received: %d from %d", myData.state, myData.id);
  Serial.println();
  
}

// Callback when data is received from MQTT client
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");

  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.println((char *)payload);

  // Parse the received JSON payload
  DeserializationError error = deserializeJson(receivedJsonDoc, payload, length);
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Check if the command is an "Interval" command
  if (receivedJsonDoc.containsKey("Interval")) {
    dataInterval = receivedJsonDoc["Interval"].as<int>();
  }

  // Check if the command is an "Sleep" command
  if (receivedJsonDoc.containsKey("Sleep")) {
    dataSleep = receivedJsonDoc["Sleep"].as<int>();
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  
  Serial.setTimeout(2000);
  while (!Serial) { }
  Serial.println();
  Serial.print("ESP8266 Bridge Node: ");
  // Dispaly the Bridge Node Mac address
  Serial.println(WiFi.macAddress());

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init WiFi connection
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  
  // Init MQTT broker connection
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);
  } 
  else {
    Serial.println("MQTT Failed to connect!");
    ESP.reset();
  }

  // Init ESP-Now
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else {
    Serial.println("ESP-Now initialized");
  }

  // Set the ESP-Now role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Send data
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  for (int i = 0; i < amount; i++) {
    for (int j = 0; j < 6; j++) {
      tempRow[i] = sensing_list[i][j];
    }
    esp_now_add_peer(tempRow, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  }
  
  // Receive Data
  esp_now_register_recv_cb(OnDataReceived);
}

void loop() {
  mqtt.loop();
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.loop();
    } 
    else {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }

  // Print Message to console in JSON format
  status["Sensor_1"] = sensors[0].state;
  status["Sensor_2"] = sensors[1].state;
  status["total"] = sensors[0].state + sensors[1].state;
  serializeJson(jsonDoc, msg, 50);
  //Serial.println(msg);
  if (!mqtt.publish(MQTT_TOPIC, msg)) {
    Serial.println("MQTT Publish failed");
  }

  // Check if the deep sleep time has been changed
  if (dataSleep != 0) {
    espDeepSleep.sleep = dataSleep;

    // Send message via ESP-NOW to Sensing Nodes
    for (int i = 0; i < amount; i++) {
      for (int j = 0; j < 6; j++) {
        tempRow[i] = sensing_list[i][j];
      }
      esp_now_send(tempRow, (uint8_t *) &espDeepSleep, sizeof(espDeepSleep));
    }
  }

  // Pause - but keep polling MQTT for incoming messages
  for (int i = 0; i < dataInterval; i++) {
    mqtt.loop();
    delay(1000);
  }
}
