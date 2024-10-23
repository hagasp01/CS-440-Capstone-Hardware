#include <WiFi.h>
#include <ArduinoHttpClient.h>   
#include <SoftwareSerial.h>
#include <RTC.h>


// Replace with your Wi-Fi credentials
const char* ssid = "Verizon-RC400L-24";
const char* password = "9b18963e";

// JS server IP address and port
const char kHostname[] = "192.168.1.163"; // JS server's local! IP
const int kPort = 5000;                     // JS server port
const char kPath[] = "/";                   // Path on the JS server

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

// Number of monitors on the network that the gateway will get data from + 1 (for the Gateway's address)
const int numMonitors = 3;

String lora_RX_address;

// String for the data we receive from LoRa transmissions
String incomingString;
// LoRa
SoftwareSerial lora(2,3); // RX, TX pin numbers on arduino board.

void setup() {
  Serial.begin(9600); 
 

  lora.begin(9600);   // Start LoRa communication on 9600 Baud - Spencer
  lora.setTimeout(500); // Max time LoRa will allow for a transmission - Spencer 

  delay(1000); 

  //Start Spencer
  //setup the start time for the RTC (Real Time Clock) on the arduino.
  RTC.begin();
  RTCTime startTime(21, Month::OCTOBER, 2024, 13, 43, 00, 
    DayOfWeek::MONDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(startTime);
  //End Spencer 

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
}

void loop() {
//Start Spencer
 RTCTime timeStamp;
  RTCTime startTime;
  RTCTime timeOut;
  //End Spencer

  WiFiClient wifiClient;
  HttpClient httpClient(wifiClient, kHostname, kPort); // Use JS server IP and port

  String jsonString = "{\"Temperature\": 69, \"CO2\": .22, \"LORA Address\": \"example\"}";
  
    //Start Spencer- slight modifications from Connor
      // Gateway will send a request to the current monitor to send its sensor data back.
  //  It will then wait for a response, if one is received it is parsed, checked, and sent to database.
  //  If a response is not recieved in a given time, then it will move to the next monitor 
  //   (poss. to add a message notifying admin that a monitor may be failing)
  for (int currMonitor = 2; currMonitor <= numMonitors; ++currMonitor) {
    lora_RX_address = String(currMonitor);
    Serial.println(lora_RX_address);

    Serial.println("Requesting Data from " + lora_RX_address);
    //send a request to the current monitor for its sensor data.
    lora.println("AT+SEND=3,12,DATA_REQUEST"); // LoRa sends AT command for data
    delay(1000);
    
    //retrieve the current time, this timestamp is used on the website.
    //  timestamp says when the current sensor data was measured.
    RTC.getTime(timeStamp);
    Serial.println(timeStamp);
    delay(10000);

    //checks if anything has been received yet 
    if(lora.available()) {

      //read in the data
      incomingString = lora.readString();
      Serial.println(incomingString);
      //End Spencer 
        Serial.println("Sending POST request to JS server...");

  // Send the POST request
  int err = httpClient.post(kPath, "/recieve-json", incomingString);
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

  httpClient.stop();  // Stop the client
    }
}
}
