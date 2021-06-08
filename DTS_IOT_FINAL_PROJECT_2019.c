#include <WiFi.h>
#include <ESP32_Servo.h>
#include "Thingsboard.h"
//ESP32

//Water Flow
int WFlow = 26;
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

//Water Level 1
int resval1 = 0;  // holds the value
int respin1 = 13; // sensor pin used

//Water Level 2
int resval2 = 0;  // holds the value
int respin2 = 12; // sensor pin used

//Water Level 3
int resval3 = 0;  // holds the value
int respin3 = 14; // sensor pin used

//Rain Drop
int rainsense= 27; // analog sensor input pin 0
int countval= 0; // counter value starting from 0 and goes up by 1 every second
int hujan;

//Buzzer
int buzzer= 2; // digital output pin 10 - buzzer output

//Servo Defined
Servo gate1;  // create servo object to control a servo
Servo gate2;  // create servo object to control a servo

//Web Server Port
WiFiServer servers(80);

//Servo Control
String header;

// Auxiliar variables to store the current output state
String output1State = "off";
String output2State = "off";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

//Wifi Connect
thingsboard_Init();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
servers.begin();

//Water Flow
  pinMode(WFlow, INPUT_PULLUP);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(WFlow), pulseCounter, FALLING);

//Water Level
  pinMode (respin1, INPUT);
  pinMode (respin2, INPUT);
  pinMode (respin3, INPUT);

//Rain Drop
  pinMode(rainsense, INPUT);
  
//buzzer as output
  pinMode(buzzer, OUTPUT);

//Servo Pin
  gate1.attach(4);  // attaches the servo on pin 13 to the servo object
  gate2.attach(5);  // attaches the servo on pin 13 to the servo object
}

void loop() {
//Sensor Reading
//Water Level Read
  resval1 = analogRead(respin1);
  resval2 = analogRead(respin2);
  resval3 = analogRead(respin3);
//Rain Drop Read
  int rainSenseReading = analogRead(rainsense);
  if (rainSenseReading <3500){ 
      hujan = 1;
   }
   else if (rainSenseReading >=3500) { // if not raining 
      hujan = 0;
   }
//Water Flow Read
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
  }
  
//Actuator Run
//Gate 2
if (flowRate <= 3) {
    gate2.write(0);
  }
  else if (flowRate > 3) {
    gate2.write(180);
  }
//Gate 1
 if (countval >= 5){ 
      Serial.print("Heavy rain");
      gate1.write(180);
   }
   //raining for long duration rise buzzer sound
   // there is no rain then reset the counter value
   if (rainSenseReading <3500){ 
      countval++; // increment count value
   }
   else if (rainSenseReading >=3500) { // if not raining 
      gate1.write(0);
      countval = 0; // reset count to 0
   }
//Buzzer
if (resval1 <= 2048) {
    digitalWrite (buzzer, LOW);
  }
  else if (resval1 > 2048) {
    digitalWrite (buzzer, HIGH);
  }

//Send Data to Thingsboard
  String data;
  const size_t bufferSize = JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();
  float waterlevel1 = (resval1*4)/4095; 
  float waterlevel2 = (resval2*4)/4095; 
  float waterlevel3 = (resval3*4)/4095; 
  root["Water Level 1"] = String(waterlevel1);
  root["Water Level 2"] = String(waterlevel2);
  root["Water Level 3"] = String(waterlevel3);
  root["Debit"] = String(flowRate);
  root["Status"] = String(hujan);
  root.printTo(data);
  thingsboard_Publish(data);
  client.loop();

//Web Server Control Gate
WiFiClient client = servers.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("Gate 1 on");
              output1State = "on";
              gate1.write(180);
            } else if (header.indexOf("GET /1/off") >= 0) {
              Serial.println("Gate1 off");
              output1State = "off";
              gate1.write(0);
            } else if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("GPIO 2 on");
              output2State = "on";
              gate2.write(180);
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("GPIO 2 off");
              output2State = "off";
              gate1.write(0);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 1  
            client.println("<p>Gate 1 - State " + output1State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output1State=="off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 2  
            client.println("<p>Gate 2 - State " + output2State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output2State=="off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
//Serial Monitor Print
  Serial.println("Water Level 1: ");
  Serial.println (resval1);
  Serial.println("Water Level 2: ");
  Serial.println (resval2);
  Serial.println("Water Level 3: ");
  Serial.println (resval3);
  Serial.print("Flow rate: ");
  Serial.print(int(flowRate));  // Print the integer part of the variable
  Serial.print("L/min");
  Serial.print("\t");
  Serial.print("Rain intensity: ");
  Serial.println (rainSenseReading);
  delay(1000);
}