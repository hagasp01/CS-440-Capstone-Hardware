# CS-440-Capstone-Hardware
Drivers and information for the CS-440 Air Quality Monitor project at Gettysburg College.

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
- Error Handling: Implements a checksum to ensure sensor data isn't corrupted in communication between endpoint and Gateway node

SETUP

- Ensure gateway node LoRa is properly setup according to instructions provided in LoRa setup sections of instruction manual
- Plug Arduino into computer with Arduino IDE open
- Make sure Arduino is wired according to Diagram found in Instruction Manual
- Update IP address of local JS server (kHostName[])
- Update the port number of local JS server (kPort)
- Upload Gateway_Node.ino code to Gateway Node Arduino


USAGE

- Switch to Software Serial Output in Arduino IDE to view connection status and received data
- Plug Gateway node arduino into power source once code is uploaded
- Gateway Node will:
  1. Connect to Wifi
  2. Loop through endpoint nodes in LoRa network, requesting and receiving sensor data on a fixed time interval
  3. Connect and send received sensor data to local JS server
 



**Endpoint_Node.ino**

DESCRIPTION

This Arduino source code file configures an Arduino to act as an Endpoint Node in this project's LoRa communication network. Its main function is to measure different air pollutants
and send the measured data as a properly formatted string back to the Gateway node via LoRa communication. It uses an array of MQ-series of gas sensors to read alcohol, CO2, and CO. It also uses a PM 2.5 sensor and a DHT22 sensor for temperature and humidity. It is possible to swap sensors in and out of an endpoint node, this code we require adjustments to accomodate such changes. The code is organized in 3 main functions: sensor initialization, sensor data collection, and LoRa transmission with calculated checksum to the Gateway Node.


FEATURES

- MQ Sensor Calibration: Precalibrated R0 value calculated in clean air before device is deployed
- MQ Sensor Read: Each sensor's analog signal is converted to an interpretable concentration value
- DHT22 Sensor Read: Temperature and Humidity digital measurements extracted from DHT22 Sensor, no conversion needed
- PM2.5 Sensor Read: Particulate matter 2.5 measurements digitally extracted from PM 2.5 Sensor and converted to an interpretable concentration value
- LoRa Communication: Sensor data is appended together in a string to be sent through LoRa transmission to a requesting Gateway Node
- Checksum Validation: Calculates a checksum for the Gateway Node to use as a form of data validation, protecting against transmission corruption

SETUP

- Use MQSensorInit() in an environment with clean air if recalibration is needed for MQ sensors
- Upload Endpoint_Node.ino code to every Endpoint Node Arduino
- Check that Arduino is wired according to Diagram found in Instruction Manual
- Check that variables such as VCC (default sensor voltage) and MQSensorInfo are properly updated if sensors are added and/or removed

USAGE

- Actions taken when request is received from Gateway Node:
  1. Data gathered and calibrated from all sensors
  2. Data formatted into a string
  3. Checksum calculated and appended to string
  4. String sent via LoRa communication back to Gateway Node
-  Switch to Software Serial Output in Arduino IDE to view connection status and sensor data being sent to Gateway node
-  Sensor information in this implementation:

    Sensor    Data Type   Unit      Pollutant Gas Detected
   
     MQ3	    Analog	    mg/L	    Alcohol
     MQ7	    Analog	    ppm	      CO
     MQ135	  Analog	    ppm	      CO2
     PM2.5	  Digital	    µg/m³	    Particulates
     DHT22	  Digital	    °C, %rH	  Temp, Hum.
   
   

