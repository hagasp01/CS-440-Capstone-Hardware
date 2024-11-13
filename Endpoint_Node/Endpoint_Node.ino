/* Sensor Code by Barry Tang  Oct 23, 2024*/
/* Network Code by Spencer Hagan Oct 23, 2024*/

#include <DHT22.h>
#include <SoftwareSerial.h>
//define pin data
#define DHTpin 5 // SDA, or almost any other I/O pin

DHT22 dht22(DHTpin); 
int data;

int digitalValue;
int apin = A0;
float sensor_volt; //Define variable for sensor voltage
float RS_air; //Define variable for sensor resistance
float R0; //Define variable for R0
float sensorValue; //Define variable for analog readings
int Vcc = 5; // Define default Vcc as 5V
String out_str = ""; // output string to LoRa

// {dpin, apin, RS/R0 in air, RL, R0, m, b}
float MQsensorInfo[3][7] = {
  {2, 19, 60, 2000, -1, -0.7, -0.28}, //MQ3
  {3, 18, 12.6, 10000, -1, -0.74, 1.377}, //MQ7
  {4, 17, 3.5, 2000, -1, -0.255, 0.718}, //MQ135
};
// String for the data we receive from LoRa transmissions
String incomingString;

// LoRa
SoftwareSerial lora(2,3); // RX, TX pin numbers on arduino board.

void setup() {
  Serial.begin(9600);  // Start serial communication
  Serial1.begin(9600);
  lora.begin(9600);   // Start LoRa communication on 9600 Baud
  lora.setTimeout(500); // Max time LoRa will allow for a transmission

  //pmsSerial.begin(9600);
  MQsensorInit(0);
  MQsensorInit(1);
  MQsensorInit(2);

  delay(1000);  // Small delay to allow the serial monitor to initialize

}

/* 
  type is an integer valueindicates the type of sensor.
  type == 0 -> MQ3 alcohol
  type == 1 -> MQ7 CO
  type == 2 -> MQ135 CO2
 */
void MQsensorInit(int type) {
  float sensorVal, VL, RS, R0;
  float aR;
  int apin = (int)MQsensorInfo[type][1];
  float RL = MQsensorInfo[type][3];
  float ratio = MQsensorInfo[type][2];
  sensorVal = 0;
  for (int x = 0; x < 10; x++) {
    aR = analogRead(apin);
    sensorVal = sensorVal + aR;
  }
  sensorVal = sensorVal/10.0;
  VL = sensorVal/1023.0 * Vcc;
  RS = RL * (Vcc/VL - 1);
  R0 = RS/ratio;
  MQsensorInfo[type][4] = R0;
}

/* 
  type is an integer valueindicates the type of sensor.
  type == 0 -> MQ3 alcohol
  type == 1 -> MQ7 CO
  type == 2 -> MQ135 CO2
 */
void MQread(int type) {
  float VL, RS, R0, y, m, b; 
  String name, unit;
  int apin = (int)MQsensorInfo[type][1];
  float sensorVal = analogRead(apin);
  float RL = MQsensorInfo[type][3];

  VL = sensorVal/1023.0 * Vcc;

  RS = RL * (Vcc/VL - 1);
  R0 = MQsensorInfo[type][4];
  y = RS/R0;
  
  m = MQsensorInfo[type][5];
  b = MQsensorInfo[type][6];
  float x_log = (log10(y)-b)/m; 
  float x = pow(10, x_log); // get concentration

  if (type == 0) {
    name = "Alcohol";
    unit = "mg/L";
    Serial.println("-----------------------------------");
    Serial.print("Alcohol Concentration (mg/L): ");
    Serial.println(x);
  } else if (type == 1) {
    name = "CO";
    unit = "ppm";
    Serial.println("-----------------------------------");
    Serial.print("CO Concentration (ppm): ");
    Serial.println(x);
  } else if (type == 2) {
    name = "CO2";
    unit = "ppm";   
    Serial.println("-----------------------------------");
    Serial.print("CO2 Concentration (ppm): ");
    Serial.println(x);
  } else {
    return;
  }
  out_str += name; out_str += ","; out_str += unit; out_str += ",";
  out_str += x; out_str += ";";
}

void DHTread() {
  // Serial.println(dht22.debug()); //optionnal

  float t = dht22.getTemperature();
  float h = dht22.getHumidity();
  out_str += "temperature"; out_str += ","; out_str += "Celcius"; out_str += ",";
  out_str += t; out_str += ";";
  out_str += "humidity"; out_str += ","; out_str += "%rH"; out_str += ",";
  out_str += h; out_str += ";";

  if (dht22.getLastError() != dht22.OK) {
    Serial.print("last error :");
    Serial.println(dht22.getLastError());
  }
  Serial.println("-----------------------------------");
  Serial.print("h=");Serial.print(h,1);Serial.print("\t");
  Serial.print("t=");Serial.println(t,1);
}

void PMread() {
  data = data * 0.33;  // 0.33 is the calibration coef.
  out_str += "PM2.5"; out_str += ","; out_str += "ug/m^3"; out_str += ",";
  out_str += data; out_str += ";";
  Serial.println("-----------------------------------");
  Serial.print("PM 2.5 (ug / m^3): ");
  Serial.println(data); 
  
}

boolean readPMSdata(Stream *s) {

  if (!s->available()) {
    return false;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0xA5) {
    s->read();
    return false;
  }

  // Now read all 4 bytes
  if (s->available() < 4) {
    // Serial.println("Less than four bytes!");
    return false;
  }

  uint8_t buffer[4];
  uint16_t sum = 0;
  s->readBytes(buffer, 4);

  uint16_t raw_sum;
  for (uint8_t i = 0; i < 3; i++) {
    raw_sum += buffer[i];
  }

  //raw sum to check with checksum
  raw_sum = raw_sum & ((1 << 7) - 1);
  // get checksum ready
  uint16_t checksum = buffer[3];
  // Process data
  sum = (buffer[1] << 7) + buffer[2];  // leftshift DATAH by 7bits
  data = sum;

  if (raw_sum != checksum) {
    return false;
  } 

  return true;
}

void loop() {
  // Serial.println(MQsensorInfo[0][4], DEC);
  MQread(0);
  MQread(1);
  MQread(2);
  DHTread();
   if (readPMSdata(&Serial1)) {
     PMread();
   }

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
        int size = out_str.length();

        lora.println("AT+SEND=1," + String(size) + "," + out_str); // LoRa sends AT command with data
        delay(50);
      }
    }
  Serial.println(out_str);
  out_str = "";
  delay(1000);
}