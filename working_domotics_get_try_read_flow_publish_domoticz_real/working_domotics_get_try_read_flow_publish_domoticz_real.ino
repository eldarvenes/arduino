#include <ESP8266WiFi.h>
#include "WiFiManager.h"

const char* ssid     = "Granden_2.4Ghz";
const char* password = "velkommen";

const char* host = "192.168.80.183";

byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 29;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;


WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);



  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.printDiag(Serial);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}



void loop() {


 if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    
    detachInterrupt(sensorInterrupt);
        
    
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;   
    
    oldTime = millis();    
    
    flowMilliLitres = (flowRate / 60) * 1000;    
    
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;

       Serial.print("Flow rate: ");
       flowRate = flowRate * 60;
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/timen");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    //Serial.print("Output Liquid Quantity: ");        
    //Serial.print(totalMilliLitres);
    //Serial.println("ml"); 
    //Serial.print("\t");       // Print tab space
    //Serial.print(totalMilliLitres/1000);
    //Serial.print("ml");
    

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
  
  delay(5000);
 

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 8080;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url_tmp = "/json.htm?type=command&param=udevice&idx=63&nvalue=0&svalue=";
  String url = url_tmp + int(flowRate);

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("Respond:");
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
