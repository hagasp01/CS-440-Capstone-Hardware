//Begin code from Connor Whitlow
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <SoftwareSerial.h>
#include <RTC.h>

// Replace with your Wi-Fi credentials
const char* ssid = ""; //This is the name of your WiFi
const char* password = ""; 

// Server IP address and port
const char kHostname[] = "";    // Server's local IP
const int kPort = ;                      // Enter the port the server is listening on
const char kPath[] = ;  // Path on the server

//These numbers should be altered if you are going to send requests from one network to another
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

// Number of monitors on the network that the gateway will get data from 1 (for the Gateway's address) 
// For clarity- the Gateway Node is labeled as 1, so name your endpoints 2...x
// NumMonitors = 2 will not loop and just repeatedly request Monitor 2. 
// NumMonitors = 3 will alternate between 2 and 3. 
const int numMonitors = 2;
int consecutiveMonitorErrors[numMonitors];
int consecutiveChecksumErrors[numMonitors];
String lora_RX_address;
//Populate this array with the locations of your sensors. Index 0 in this array corresponds to the location of endpoint 2; 1 corresponds to 3; etc.
const char* locations[] = {};

// String for the data we receive from LoRa transmissions
String incomingString;
// LoRa
SoftwareSerial lora(2, 3);  // RX, TX pin numbers on arduino board.
//End code from Connor Whitlow 

// Calcuate checksum from data string - Joe
int calculateChecksum(String data) {
  int checksum = 0;
  for (int i = 0; i < data.length(); i++) {
    checksum += data[i];
  }
  return checksum;
}

void setup() {
  //Begin Code from Spencer Hagan
  Serial.begin(9600);


  lora.begin(9600);      // Start LoRa communication on 9600 Baud - Spencer
  lora.setTimeout(500);  // Max time LoRa will allow for a transmission - Spencer

  delay(1000);

  //setup the start time for the RTC (Real Time Clock) on the arduino.
  RTC.begin();
  RTCTime startTime(21, Month::OCTOBER, 2024, 13, 43, 00,
                    DayOfWeek::MONDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(startTime);
  //End code from Spencer Hagan

  //Begin Code from Connor Whitlow
  Serial.println("Connecting to Wi-Fi...");

  // Connect to the Wi-Fi
  WiFi.begin(ssid, password);

  // Wait until the connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print success message and IP address
  Serial.println();
  Serial.println("Connected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());  // Display the IP address assigned
  //End code from Connor Whitlow
}

void loop() {
  //Begin code from Spencer Hagan
  RTCTime timeStamp;
  RTCTime startTime;
  RTCTime timeOut;
  //End code from Spencer Hagan

  //Begin code from Connor Whitlow
  WiFiClient wifiClient;
  HttpClient httpClient(wifiClient, kHostname, kPort);  // This line will construct a clinet object using the locations specified
  
  
  //String jsonString = "{\"Temperature\": 69, \"CO2\": .22, \"LORA Address\": \"example\"}";

  //Start Spencer- slight modifications from Connor regarding formatting, specifics of data. Actual code for requesting was from Spencer
  // Gateway will send a request to the current monitor to send its sensor data back.
  //  It will then wait for a response, if one is received it is parsed, checked, and sent to database.
  //  If a response is not recieved in a given time, then it will move to the next monitor
  //   (poss. to add a message notifying admin that a monitor may be failing)
  for (int currMonitor = 2; currMonitor <= numMonitors; ++currMonitor) {
    lora_RX_address = String(currMonitor);
    Serial.println(lora_RX_address);

    Serial.println("Requesting Data from " + lora_RX_address);
    //send a request to the current monitor for its sensor data.
    lora.println("AT+SEND=" + lora_RX_address + ",12,DATA_REQUEST");  // LoRa sends AT command for data
    delay(1000);

    //retrieve the current time, this timestamp is used on the website.
    // timestamp says when the current sensor data was measured.
    RTC.getTime(timeStamp);
    Serial.println(timeStamp);
    delay(28000);

    //checks if anything has been received yet
    if (lora.available()) {

      //read in the data
      incomingString = lora.readString();
      if (incomingString.length() > 0) {
        //Checksum Verification - Start Joe
        
        // Make sure incoming string has checksum
        int chkIndex = incomingString.indexOf("CHK:");

        if (chkIndex != -1) {

          //Remove RSSI numbers from received string
          int rssiIndex = incomingString.indexOf(",-");
          String message = incomingString.substring(0, rssiIndex);

          message = message.substring(message.indexOf(",") + 1, message.length());
          message = message.substring(message.indexOf(",") + 1, message.length());

          chkIndex = message.indexOf("CHK:");
          Serial.println("Message:" + message);
          String dataString = message.substring(0, chkIndex);

          int receivedChecksum = message.substring(chkIndex + 4).toInt();

          // Calculate checksum for data part and verify
          int calculatedChecksum = calculateChecksum(dataString);
          Serial.println(calculatedChecksum);
          // End Joe

          //after parsing data out and checksums
          String locationName = locations[currMonitor - 2];  //Location array and appending by Connor 
          dataString = dataString + ("\"Location\":\"" + locationName + "\",");
          dataString = dataString + ("\"Timestamp\":\"" + String(timeStamp) + "\"}");
          Serial.println(dataString);
          //End code from Spencer Hagan

          //Begin Code from Connor Whitlow
          // Send the POST request
          if (calculatedChecksum == receivedChecksum) {
            Serial.println("Data received correctly: " + dataString);
            Serial.println("Sending POST request to JS server...");

            //Posts to the server with a header indicating data is in JSON format
            int err = httpClient.post(kPath, "application/json", dataString);
            if (err == 0) {
              Serial.println("POST request sent successfully");

              // Get the response status code
              err = httpClient.responseStatusCode();
              if (err >= 0) {
                Serial.print("Got status code: ");
                Serial.println(err);

                // Skip response headers and print the body
                err = httpClient.skipResponseHeaders();
                if (err >= 0) {
                  int bodyLen = httpClient.contentLength();
                  Serial.print("Content length is: ");
                  Serial.println(bodyLen);
                  Serial.println();
                  Serial.println("Body returned follows:");

                  unsigned long timeoutStart = millis();
                  char c;
                  //Changing network delay can have significant bearing on the success of ths loop
                  while ((httpClient.connected() || httpClient.available()) && ((millis() - timeoutStart) < kNetworkTimeout)) {
                    if (httpClient.available()) {
                      c = httpClient.read();  // Read response body
                      Serial.print(c);
                      timeoutStart = millis();
                    } else {
                      delay(kNetworkDelay);
                    }
                  }
                } else {
                  Serial.print("Failed to skip response headers: ");
                  Serial.println(err);
                }
              } else {
                Serial.print("Getting response failed: ");
                Serial.println(err);
              }
            } else {
              Serial.print("POST request failed: ");
              Serial.println(err);
            }
            consecutiveChecksumErrors[currMonitor] = 0;
            httpClient.stop();  // Stop the client
          } else {
            Serial.println("Checksum not correct, data corrupted!");
            consecutiveChecksumErrors[currMonitor] += 1;
            //error logging for consecutive checksum errors
            if (consecutiveChecksumErrors[currMonitor] > 3) {
              String errorMessage = "Monitor " + String(currMonitor) + " is experiencing consecutive checksum errors.";
              httpClient.post(kPath, "/error-log", errorMessage);
            }
          }
        }
      }
    }
  }
}
