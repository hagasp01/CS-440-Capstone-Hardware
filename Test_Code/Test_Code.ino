/* Network Test Code by Spencer Hagan Nov 11, 2024*/
#include <SoftwareSerial.h>

// String for the data we receive from LoRa transmissions
String incomingString;

// LoRa
SoftwareSerial lora(2,3); // RX, TX pin numbers on arduino board.

void setup() {
  Serial.begin(9600);  // Start serial communication
  Serial1.begin(9600);
  lora.begin(9600);   // Start LoRa communication on 9600 Baud
  lora.setTimeout(500); // Max time LoRa will allow for a transmission

  delay(1000);  // Small delay to allow the serial monitor to initialize
  Serial.println("Waiting for transmissions...");
}

void loop() {
  Serial.println(".");
  delay(1000);
  Serial.println(".");
  delay(1000);
  Serial.println(".");

  //checks if anything has been received yet
    if(lora.available()) {
      Serial.println("DATA RECEIVED!!");
      //read in the data
      incomingString = lora.readString();
      Serial.println(incomingString);
    }
  delay(1000);
}