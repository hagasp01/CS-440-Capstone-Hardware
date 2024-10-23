#include <SoftwareSerial.h>

// String for the data we receive from LoRa transmissions
String incomingString;

String testData = "{\"Temperature\": 69, \"CO2\": 22, \"LORA Address\": \"example\"}";
// LoRa
SoftwareSerial lora(2,3); // RX, TX pin numbers on arduino board.

void setup() {
  Serial.begin(9600);  // Start serial communication
  lora.begin(9600);   // Start LoRa communication on 9600 Baud
  lora.setTimeout(500); // Max time LoRa will allow for a transmission
  delay(1000);  // Small delay to allow the serial monitor to initialize

}

void loop() {

  //checks if anything has been received yet
    if(lora.available()) {
      //read in the data
      incomingString = lora.readString();
      Serial.println(incomingString);

      //parses the recieved info and gets the actual data portion of it (+RCV=50,5,HELLO,-99,40 where we want HELLO)
      char dataArray[30]; 
      incomingString.toCharArray(dataArray,30);
      char* data = strtok(dataArray, "=");
      data = strtok(NULL, ",");
      char* tranAdd = data;
      Serial.println(tranAdd);
      data = strtok(NULL, ",");
      data = strtok(NULL, ",");
      Serial.println(data);

      //checks if the gateway is requesting data
      if(strcmp(data, "DATA_REQUEST") == 0) {
        Serial.println("SENDING DATA TO GATEWAY");
        //Sends its sensor data back to the gateway
        int size = testData.length();

        lora.println("AT+SEND=1," + String(size) + "," + testData); // LoRa sends AT command with data
        delay(50);
      }
    }

}
