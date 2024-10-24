#include <SoftwareSerial.h>

#define LED 13
#define LED2 12

String incomingString;

SoftwareSerial lora(2,3);

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  Serial.begin(9600);
  lora.begin(9600);
  lora.setTimeout(500);
}

void loop()
{
  if (lora.available()) {

    incomingString = lora.readString();
    Serial.println(incomingString);

    char dataArray[30]; 
    incomingString.toCharArray(dataArray,30);
    char* data = strtok(dataArray, "=");
    data = strtok(NULL, ",");
    char* tranAdd = data;
    Serial.println(tranAdd);
    data = strtok(NULL, ",");
    data = strtok(NULL, ",");
    Serial.println(data);
    
    if(strcmp(tranAdd,"2") == 0) {
      if (strcmp(data,"HI") == 0) {
        digitalWrite(LED, LOW);
        delay(50);
      }

      if (strcmp(data,"LO") == 0) {
        digitalWrite(LED, HIGH);
        delay(50);
      }
    }

    if(strcmp(tranAdd,"3") == 0) {
      if (strcmp(data,"HI") == 0) {
        digitalWrite(LED2, LOW);
        delay(50);
      }

      if (strcmp(data,"LO") == 0) {
        digitalWrite(LED2, HIGH);
        delay(50);
      }
    }
  }
}
