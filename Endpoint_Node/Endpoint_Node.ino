// String for the data we receive from LoRa transmissions
String incomingString;
// LoRa
SoftwareSerial lora(2,3); // RX, TX pin numbers on arduino board.

void setup() {
  Serial.begin(9600);  // Start serial communication
  lora.begin(9600);   // Start LoRa communication on 9600 Baud
  lora.setTimeout(500); // Max time LoRa will allow for a transmission
  delay(1000);  // Small delay to allow the serial monitor to initialize

}

void loop() {

  //checks if anything has been received yet, this says that the size of available data is larger than 0. 
    if(lora.available() > 0) {
      //read in the data
      incomingString = lora.readString();
      Serial.println(incomingString);

      //parses the recieved info and gets the actual data portion of it (+RCV=50,5,HELLO,-99,40 where we want HELLO)
      char dataArray[30]; 
      incomingString.toCharArray(dataArray,30);
      char* data = strtok(dataArray, "=");
      data = strtok(NULL, ",");
      data = strtok(NULL, ",");
      data = strtok(NULL, ",");

      //checks if the gateway is requesting data
      if(strcmp(data, "DATA_REQUEST") == 0 && tranAdd == 1) {
        //Sends its sensor data back to the gateway
        lora.println("AT+SEND=1,11,SENSOR_DATA"); // LoRa sends AT command with data
        delay(50)
      }
    }

}
