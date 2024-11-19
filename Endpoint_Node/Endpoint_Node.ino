/* Sensor Code by Barry Tang  Nov 19, 2024
  all MQ sensors are precalibrated and their R0 values are found 
  before hand by calibrating them in clean outdoor air.
  Note the R0 for MQ135 is found by extropolating the sensitivity
  curve on its datasheet assuming CO2 ppm = 400 in outdoor
  clean air.
*/

/* Network Code by Spencer Hagan Oct 23, 2024*/

#include <DHT22.h>
#include <SoftwareSerial.h>
//define pin data
#define DHTpin 5 // SDA, or almost any other I/O pin

DHT22 dht22(DHTpin); 
int digitalValue;
int apin = A0;
float sensor_volt; //Define variable for sensor voltage
float RS_air; //Define variable for sensor resistance
float R0; //Define variable for R0
float sensorValue; //Define variable for analog readings
int Vcc = 4.7; // Define default Vcc as 5V
String out_str = "{"; // output string to LoRa

// {dpin, apin, RS/R0 in clean air, RL, R0, m, b}
float MQsensorInfo[3][7] = {
  {2, A5, 60, 2000, 202, -0.7, -0.28}, //MQ3
  {3, A4, 25, 10000, 3500, -0.642, 1.267}, //MQ7
  {4, A3, 3.5, 36000, 500000, -0.345, 0.7}, //MQ135
};

/*
  type is an integer valueindicates the type of sensor.
  type == 0 -> MQ3 alcohol
  type == 1 -> MQ7 CO
  type == 2 -> MQ135 CO2
  type == 3 -> PM2.5
  type == 4 -> Temperature
  type == 5 -> Humidity
  @Barry Tang
*/
String targets[6] = {"Alcohol", "CO", "CO2", "PM2.5", "Temperature", "Humidity"};
String units[6] = {"mg/L", "ppm", "ppm", "ug/m3", "Celcius", "%rH"};

// String for the data we receive from LoRa transmissions
String incomingString;

// LoRa
SoftwareSerial lora(2,3); // RX, TX pin numbers on arduino board.

/*
  returns a checksum to be check at gateway node.
  @Joe
*/
int makeChecksum(String data) {
  int checksum = 0;
  for (int i = 0; i < data.length(); i++) {
    checksum += data[i];
  }
  return checksum;
}

void setup() {
  Serial.begin(9600);  // Start serial communication
  Serial1.begin(9600); // Start serial1 communication for PM2.5 sensor
  lora.begin(9600);   // Start LoRa communication on 9600 Baud
  lora.setTimeout(500); // Max time LoRa will allow for a transmission
  delay(1000);  // Small delay to allow the serial monitor to initialize
}

/* 
  This method should only be run when the sensor is in 
  clean air for calibration of R0.

  For the devices we use a pre-calibrated value of R0,
  so this method should not be called in the device.
  @Barry Tang
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
  This method reads the analog signal from one of the 
  three MQ sensors and convert it to a meaningful concentration
  value.

  Prints the readings to Serial Monitor and appends to 
  out_str for Lora transmission.
  @Barry Tang
 */
void MQread(int type) {
  float VL, RS, R0, y, m, b, x_log, x; 
  int apin = (int) MQsensorInfo[type][1];
  float sensorVal = analogRead(apin);
  float RL = MQsensorInfo[type][3];

  VL = sensorVal/1023.0 * Vcc;
  RS = RL * (Vcc/VL - 1);
  R0 = MQsensorInfo[type][4];
  y = RS/R0;
  
  m = MQsensorInfo[type][5];
  b = MQsensorInfo[type][6];
  x_log = (log10(y)-b)/m; 
  x = pow(10, x_log); // get concentration

  if (type == 0) {
    Serial.println("-----------------------------------");
    Serial.print("Alcohol Concentration (mg/L): ");
    Serial.println(x);
  } else if (type == 1) {
    Serial.println("-----------------------------------");
    Serial.print("CO Concentration (ppm): ");
    Serial.println(x);
  } else if (type == 2) {
    Serial.println("-----------------------------------");
    Serial.print("CO2 Concentration (ppm): ");
    Serial.println(x);
  } else {
    return;
  }
  out_str += getOutputString(type, x);
}

/*
  This method reads the temperature and humidity values
  using the <DHT22.h> library. 

  Prints the readings to Serial Monitor and appends to 
  out_str for Lora transmission.
  @Barry Tang
*/
void DHTread() {
  // Serial.println(dht22.debug()); //optionnal
  float t = dht22.getTemperature();
  float h = dht22.getHumidity();
  out_str += getOutputString(4, t);
  out_str += getOutputString(5, h);

  if (dht22.getLastError() != dht22.OK) {
    Serial.print("last error :");
    Serial.println(dht22.getLastError());
  }
  Serial.println("-----------------------------------");
  Serial.print("h=");Serial.print(h,1);Serial.print("\t");
  Serial.print("t=");Serial.println(t,1);
}

/*
  Prints the PM2.5 reading to Serial Monitor and appends to 
  out_str for Lora transmission.
  @Barry Tang
*/
void PMread() {
  float pm25 = readPMSdata(&Serial1)/3.;  // 0.33 is the calibration coef.
  out_str += getOutputString(3, pm25);
  Serial.println("-----------------------------------");
  Serial.print("PM 2.5 (ug / m^3): ");
  Serial.println(pm25); 
}

/*
  Read the stream from PM2.5 sensor and convert it into a meaningful
  concentration value.
  @Barry Tang
*/
float readPMSdata(Stream *s) {
  float pm25;
  if (!s->available()) {
    pm25 = -999*3; // error
    return pm25;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0xA5) {
    s->read();
    pm25 = -999*3; // error
    return pm25;
  }

  // Now read all 4 bytes
  if (s->available() < 4) {
    pm25 = -999*3; // error
    return pm25;
  }

  uint8_t buffer[4];
  s->readBytes(buffer, 4);

  uint16_t raw_sum;
  for (uint8_t i = 0; i < 3; i++) {
    raw_sum += buffer[i];
  }

  raw_sum = raw_sum & ((1 << 7) - 1); // clear bits higher than the right 7 bits
  uint16_t checksum = buffer[3];    // get checksum ready
  if (raw_sum != checksum) {
    pm25 = -999*3; // error
    return pm25;
  } 

  pm25 = (buffer[1] << 7) + buffer[2];  // leftshift DATAH by 7bits
  return pm25;
}

/*
  output a string that contains the concentration of a target
  in the LoRa 
  @Barry Tang
*/
String getOutputString(int type, float dat) {
  String name = targets[type];
  String unit = units[type];
  String output = "";
  output += name; output += ","; 
  output += unit; output += ",";
  output += dat; output += ";";
  return output;
}

void loop() {
  // Serial.println(MQsensorInfo[0][4], DEC);
  MQread(0);
  MQread(1);
  MQread(2);
  PMread();
  DHTread();

  //checks if anything has been received yet @Spencer
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

        int checksum = makeChecksum(out_str);
        String final_msg = out_str + "CHK:" + String(checksum);
        int size = final_msg.length();
        // !!! should there be a closing right bracket in the final_msg? --barry

        lora.println("AT+SEND=1," + String(size) + "," + final_msg); // LoRa sends AT command with data
        delay(50);
      }
    }
  Serial.println(out_str);
  out_str = "{";
  delay(1000);
}
