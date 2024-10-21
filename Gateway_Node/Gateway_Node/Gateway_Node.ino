#include <SoftwareSerial.h>
#include <RTC.h>

// Number of monitors on the network that the gateway will get data from + 1 (for the Gateway's address)
const int numMonitors = 3;

String lora_RX_address;

// String for the data we receive from LoRa transmissions
String incomingString;
// LoRa
SoftwareSerial lora(2,3); // RX, TX pin numbers on arduino board.

void setup() {
  Serial.begin(9600);  // Start serial communication
  lora.begin(9600);   // Start LoRa communication on 9600 Baud
  lora.setTimeout(500); // Max time LoRa will allow for a transmission
  delay(1000);  // Small delay to allow the serial monitor to initialize

  //setup the start time for the RTC (Real Time Clock) on the arduino.
  RTC.begin();
  RTCTime startTime(21, Month::OCTOBER, 2024, 13, 43, 00, 
    DayOfWeek::MONDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(startTime);
}

void loop() {
  RTCTime timeStamp;
  RTCTime startTime;
  RTCTime timeOut;

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

      //The LoRa transmission is parsed to get the data, timestamp is appended, and sent to the database.
    }
  }
}
