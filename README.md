# SeatTrack: IoT Seat Occupancy Tracker

## Overview

This project focuses on the implementation of an IoT platform using interconnected ESP8266 microcontrollers. The primary goal is to establish a sensor network capable of detecting chair occupancy in real-time and notifying users via MQTT of any environmental changes. Data collected by the sensors are securely stored online in a database, providing the foundation for real-time visualization and analysis. The platform offers remote configuration options, allowing users to adjust notification delays and control the entire network from any location. Leveraging the ESP-Now communication protocol ensures energy efficiency by employing various sleep modes to optimize sensor performance and minimize power consumption as well as maintenance cost.

## Features

- **Communication**:
  - Sensing Nodes ↔ Bridge Node: ESP-Now
  - Bridge Node ↔ MQTT Client: MQTT
  - Visualization via Node-RED

- **Energy Efficiency**:
  - Uses ESP-Now for low power consumption.
  - Implements ESP.deepSleep() to extend battery life.
  - Powered by LiFePO4 batteries for compatibility and longevity.

- **Flexibility**:
  - Easy to use, install, and maintain.
  - Remote configuration of notification delays and network control.
  - Easily add new sensing nodes.

## Hardware Components

- **Sensing Nodes**:
  - ESP8266 microcontroller
  - Push button (simulating a pressure mat)
  - LiFePO4 battery

- **Bridge Node**:
  - ESP8266 microcontroller
  - USB power connection

- **Gateway**:
  - Laptop with MQTT client software (Node-RED)

## Software Components

- **Sensing Nodes**:
  - Button press/release event handling.
  - ESP-Now communication.
  - Power management using sleep modes.
  - Libraries: ESP8266 Arduino Core, ESP-Now library.

- **Bridge Node**:
  - Serial communication with the gateway.
  - ESP-Now communication with sensing nodes.
  - Power management logic.
  - Libraries: ESP8266 Arduino Core, ESP-Now library, JSON library, MQTT messaging library.

- **Gateway**:
  - MQTT client interaction with the sensor network.
  - Node-RED for visualization.
  - Libraries: MQTT library, Node-RED library (node-red, node-red-dashboard).

## Setting Up the System

1. **Sensing Nodes**:
   - Connect push buttons to ESP8266.
   - Upload sensing node code and configure the environment.
   - Connect batteries.

2. **Bridge Node**:
   - Upload bridge node code and configure the environment.
   - Connect the bridge node to the laptop via USB.

3. **Gateway**:
   - Install Node-RED software on the laptop.
   - Optionally, install MQTT client software for enhanced security.

4. **Using the System**:
   - Monitor chair occupancy via MQTT notifications on Node-RED dashboard.
   - Enable/disable the sensor network and adjust configuration settings from the dashboard.

## Testing

- **Unit Tests**:
  - Verify individual components (sensing nodes, bridge node, gateway).
  - Test button events, ESP-Now communication, and MQTT messaging.

- **Integration Tests**:
  - Check communication between sensing nodes and bridge.
  - Ensure interaction between bridge and gateway.

- **Future Work**:
  - Measure battery life and evaluate system reliability under various scenarios.

## Reflection

### Problems Encountered & Solutions

- **Power Consumption**:
  - Implemented various sleep modes to conserve battery life.

- **Communication Reliability**:
  - Fine-tuned ESP-Now parameters and ensured error handling.

## Future Development

- **Enhanced User Interface**:
  - Develop a user-friendly mobile app for easier system control.

- **Integration with Other IoT Devices**:
  - Explore integration possibilities with other smart devices.

- **Expandable Network**:
  - Investigate methods to seamlessly add new sensing nodes to the network.

## References

- [LiFePO4 Battery Information](https://www.evlithium.com/hot-lithium-battery/32650-32700-lifepo4-battery-cell.html)

## License

This project is licensed under the MIT License.

## Contributors

- Oscar BIGNO
- James DUONG
- Romain PERROT

---

For more detailed information, visit our [project website](https://jcduong77.wixsite.com/seattrack) and watch our [video demonstration](#).

© 2023 by Oscar BIGNO, James DUONG, and Romain PERROT.
