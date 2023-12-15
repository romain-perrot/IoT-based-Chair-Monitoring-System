#include <ESP8266WiFi.h>
#include <espnow.h>

/*
  Implementation: 
    This code includes the setup for a button.
    This is typical implementation for a sensing node that detects environmental state.
    In our case, the pressure on a chair.

  Communication: 
    This ESP8266 (Sensing Node) connects to an other ESP8266 (Bridge Node) through ESP-Now.

  Goal:
    It sends the state of a button and receive informations about the duration of deep sleep.
*/

// --------------------------------------------------------------------------------------------
//        UPDATE CONFIGURATION TO MATCH YOUR ENVIRONMENT
// --------------------------------------------------------------------------------------------

// MAC Address of the Bridge Node
uint8_t bridgeAddress[] = {0x08, 0x3A, 0x8D, 0xEF, 0x07, 0xD0};

// Sensing Node id
#define SENSOR_ID 1

// Add GPIO pins used to connect devices
#define buttonPin 5

// Init Deep sleep (in microseconds)
int dataSleep = 2000000; // Deep sleep time between checks
int disabledNetwork = 60000000; // Deep sleep to disable the network

// --------------------------------------------------------------------------------------------
//        SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS LINE
// --------------------------------------------------------------------------------------------

// Init data state
int buttonState;

// Structure of Sent Data
typedef struct struct_send_data {
  int id;
  int state;
} struct_send_data;

// Create a struct_send_data called buttonData
struct_send_data buttonData;

// Callback when data is sent to Bridge Node
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
    
    Serial.printf("Entering deep sleep for %d microseconds\n", dataSleep);
    ESP.deepSleep(dataSleep); // deepSleep mode: WAKE_RF_DEFAULT, WAKE_RFCAL, WAKE_NO_RFCAL
  }
  else {
    Serial.println("Delivery fail");
  }
}

// Structure of Received Data
typedef struct struct_receive_data {
  int sleep;
} struct_receive_data;

// Create a struct_receive_data called espDeepSleep
struct_receive_data espDeepSleep;

// Callback when data is received from Bridge Node
void OnDataReceived(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  // Print the MAC adress of the Bridge Node
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);

  memcpy(&espDeepSleep, incomingData, sizeof(espDeepSleep));
  Serial.printf("Deep sleep time: %f\n", espDeepSleep.sleep);
  // Update the structure with the new incoming data
  dataSleep = espDeepSleep.sleep;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  Serial.setTimeout(2000);
  while (!Serial) { }
  Serial.println();
  Serial.print("ESP8266 Sensing Node: ");
  // Dispaly the Sensing Node Mac address
  Serial.println(WiFi.macAddress());

  // Init button pin
  pinMode(buttonPin, INPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

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
  esp_now_add_peer(bridgeAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // Receive Data
  esp_now_register_recv_cb(OnDataReceived);
}

void loop() {
  // Check if network not disabled
  if (dataSleep != disabledNetwork) {
    // Read the state of the button
    buttonState = digitalRead(buttonPin);

    // Check if the push button is pressed.
    // If it is, the buttonState is HIGH:
    if (buttonState == HIGH) {
      Serial.printf("Sensor %d, button pressed, sending data...\n", SENSOR_ID);

      // Update the structure before sending the new data
      buttonData.id = SENSOR_ID;
      buttonData.state = 1;

      // Send message via ESP-NOW
      esp_now_send(bridgeAddress, (uint8_t *) &buttonData, sizeof(buttonData));
    }
    // If not, the buttonState is LOW:
    else if (buttonState == LOW) {
      Serial.printf("Sensor %d, button unpressed, sending data...\n", SENSOR_ID);

      // Update the structure before sending the new data
      buttonData.id = SENSOR_ID;
      buttonData.state = 0;

      // Send message via ESP-NOW
      esp_now_send(bridgeAddress, (uint8_t *) &buttonData, sizeof(buttonData));
    }
    delay(1000);
  }
  else {
    Serial.println("Network disabled:");
    Serial.println("Sensing Node deactivated...");

    delay(1000);

    // Init deep sleep for disabled network
    ESP.deepSleep(dataSleep);
  }
}
