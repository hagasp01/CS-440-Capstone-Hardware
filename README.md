# CS-440-Capstone-Hardware
Drivers and information for the CS-440 Air Quality Monitor project at Gettysburg College.

LoRa Information
Frequency: 915MHz
Band: 9600
NetworkID: 5

**Gateway_Node.ino**

DESCRIPTION

This Arduino source code file configures a Wifi-capable Arduino to act as a Gateway Node between the Endpoint node air quality monitors 
and the JS server used to upload sensor data to the project website via HTTP. It requests and receives sensor data from the endpoint nodes using LoRa communication and relies on Software Serial to communicate with the Wifi-capable Arduino. The program uses an RTC to timestamp sensor data and manage request intervals. It implements error handling through an internet checksum to ensure data isn't corrupted in transmission.

FEATURES

- Wifi Connection: Connects to specified Wifi network
- HTTP Communication: Sends sensor data received from endpoint nodes to a local JS server using HTTP POST requests
- RTC Interval Management: Timestamps sensor data and manages LoRa request intervals sent to Enpoint Nodes
- LoRa Communication: Sends LoRa message Requesting Sensor Data and receives LoRa messages containing sensor data
- String Manipulation: Extracts data from LoRa receied string to be sent to local JS server
- Error Handling: Implements a checksum to ensure sensor data isn't corrupted and detects errors in sending LoRa transmissions,
  receiving LoRa transmissions, and sending HTTP POST requests to the server

SETUP

- Ensure gateway node LoRa is properly setup according to instructions provided in LoRa sections
- Plug Arduino into computer with Arduino IDE open
- Make sure Arduino is wire according to Diagram found in Instruction Manual
- Switch to Software Serial Output in Arduino IDE
- Upload Gateway_Node.ino to Gateway Node Arduino

USAGE




**Endpoint_Node.ino**

DESCRIPTION

FEATURES

SETUP

USAGE
