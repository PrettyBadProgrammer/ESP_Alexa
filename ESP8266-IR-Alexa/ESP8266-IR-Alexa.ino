
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
#define LAMP_1 "base speaker"
#define bluetooth "speaker bluetooth mode"
#include <IRsend.h>
#define LED_BUILTIN 2
#include <IRremoteESP8266.h>
#define IR_SEND_PIN D2
IRsend irsend(IR_SEND_PIN);

bool virtual_state;
int track_val = 0;

int count = 0;

int x = 0;
int Volume = 0;

fauxmoESP fauxmo;
// Replace with your network credentials
const char* ssid     = "Password";
const char* password = "SSID";

// Set web server port number to 80
WiFiServer server(3999);

String header;

// Auxiliar variables to store the current output state
String output5State = "off";




// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH);
  
  irsend.begin();
  Serial.begin(115200);
  // Initialize the output variables as outputs



  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();



 fauxmo.createServer(true); // not needed, this is the default value
  fauxmo.setPort(80); // This is required for gen3 devices

  // You have to call enable(true) once you have a WiFi connection
  // You can enable or disable the library at any moment
  // Disabling it will prevent the devices from being discovered and switched
  fauxmo.enable(true);
  // You can use different ways to invoke alexa to modify the devices state:
  // "Alexa, turn lamp two on"

  // Add virtual devices
  fauxmo.addDevice(LAMP_1);
  fauxmo.addDevice(bluetooth);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    // Callback when a command from Alexa is received. 
    // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
    // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
    // Just remember not to delay too much here, this is a callback, exit as soon as possible.
    // If you have to do something more involved here set a flag and process it in your main loop.
        
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);   
    if ( (strcmp(device_name, LAMP_1) == 0) ) {
      // this just sets a variable that the main loop() does something about
      Serial.println("RELAY 1 switched by Alexa");
      //digitalWrite(RELAY_PIN_1, !digitalRead(RELAY_PIN_1));

       /*float DecVolume = (float)value/256.0;
        Volume = DecVolume*30;*/
      
      
      virtual_state = state;
      if(value == track_val){
          virtual_state = false;
        }
      if(virtual_state == true){
        if(value == 255){
          virtual_state = true;
          value = 0;
        }
        if(value == 0){
          virtual_state = false;
        }
        if(value != 0){
          float DecVolume = (float)value/256.0;
        Volume = DecVolume*30;
        track_val = value;
        }
        
      }
      if(virtual_state == false){
         irsend.sendNEC(0x40BD28D7, 32);
      }
       
          
      
      
    }
    if((strcmp(device_name, bluetooth) == 0)){
      
      if(state == true){
        irsend.sendNEC(0x40BD30CF, 32);
      }
      if(state == false){
         
         irsend.sendNEC(0x40BDB04F, 32);
      }
       
    }

  });
  
}

void loop(){
  unsigned long currentMillis = millis();
  unsigned long previousMillis = 0;
   
    if (currentMillis - previousMillis >= 1000) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    
    // if the LED is off turn it on and vice-versa:
    if(x<Volume)
    {
    while(x!= Volume)
    {
    irsend.sendNEC(0x40BD48B7, 32);
    Serial.println(x);
    x++;
    }
    x=Volume;
    }
    if(x>Volume){
      while(x!= Volume){
        irsend.sendNEC(0x40BDC837 , 32);
    Serial.println(x);
    x--;
      }
      x=Volume;
      
    }
  }
  
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
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
            if (header.indexOf("GET /led_state/off") >= 0) {
              Serial.println("GPIO D0 on");
              output5State = "on";
              irsend.sendNEC(0x40BD28D7, 32); // Sony TV power code
            
            } else if (header.indexOf("GET /led_state/on") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD28D7, 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/mute") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BDA857 , 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/bluetooth") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD30CF, 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/optical") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BDB04F , 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/volup") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD48B7 , 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/voldown") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BDC837 , 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/bassup") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD807F , 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/bassdown") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD40BF , 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /led_state/trebleup") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BDC03F, 32); // Sony TV power code
              
            } 
             else if (header.indexOf("GET /led_state/trebledown") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD20DF , 32); // Sony TV power code
              
            } 
           
            else if (header.indexOf("GET /computer_state/on") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD28D7, 32); // Sony TV power code
              
            } 
            else if (header.indexOf("GET /computer_state/off") >= 0) {
              Serial.println("GPIO D0 off");
              irsend.sendNEC(0x40BD28D7, 32); // Sony TV power code
              
            } 
          
            
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
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }


   fauxmo.handle();


}
