#include <WiFi.h>
#include <ArduinoHttpClient.h>  

// Replace with your Wi-Fi credentials
const char* ssid = "Verizon-RC400L-24";
const char* password = "9b18963e";

// JS server IP address and port
const char kHostname[] = "192.168.1.124"; // If on the same network, add serv's local IP
const int kPort = 5000;                     // JS server port
const char kPath[] = "/";                   // Path on the JS server

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

void setup() {
  Serial.begin(9600);  // Start serial communication
  delay(1000);  // Small delay to allow the serial monitor to initialize

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
  WiFiClient wifiClient;
  HttpClient httpClient(wifiClient, kHostname, kPort); // Use JS server IP and port

  String jsonString = "{\"name\": \"Connor\", \"age\": 22, \"city\": \"New York\"}";

  Serial.println("Sending POST request to JS server...");

  // Send the POST request
  int err = httpClient.post(kPath, "/recieve-json", jsonString);
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

  // Wait a while before making another request
  delay(10000); // Delay 10 seconds before the next request
}
