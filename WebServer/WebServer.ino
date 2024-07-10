#include <WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float numTemperature;
float numHumidity;

//Replace with your network credentials.
const char* ssid = "HUAWEI";
const char* password = "sam6612599";

//Set web server port number to 80.
WiFiServer server(80);

//Variable to store the HTTP request.
String header;

//Auxiliar variables to store the current output state.
String output1State = "Off";
String output2State = "Off";

//Assign output variables to GPIO pins.
int pinBuzzer = 2;
int pinRelay = 5;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;
//************************************************************************************************************************
void setup() {
  Serial.begin(115200);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinBuzzer, LOW);
  digitalWrite(pinRelay, LOW);

  Serial.print(">> Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(">> WiFi is connected.");
  Serial.println(">> IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  dht.begin();
}
//************************************************************************************************************************
void loop() {
  WiFiClient client = server.available();                                       //Listen for incoming clients.

  if (client)                                                                   //If a new client connects:
  {
    Serial.println("********************************************************************");
    Serial.println(">> New Client.");                                           //Print a message out in the serial port.
    String currentLine = "";                                                    //Make a String to hold incoming data from the client.
    currentTime = millis();
    previousTime = currentTime;

    numTemperature = dht.readTemperature();
    numHumidity = dht.readHumidity();

    while (client.connected() && currentTime - previousTime <= timeoutTime) {   //Loop while the client's connected.
      currentTime = millis();
      if (client.available()) {                                                 //If there's bytes to read from the client, read a byte, then print it out the serial monitor.
 
        char c = client.read();             
        Serial.write(c);
        header += c;
        
        if (c == '\n') {                                                        //If the byte is a newline character.
          
          //If the current line is blank, you got two newline characters in a row
          //that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            //HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            //and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            //Turns the GPIOs On and Off.
            if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("GPIO 1 On.");
              output1State = "On";
              digitalWrite(pinBuzzer, HIGH);
            } else if (header.indexOf("GET /1/off") >= 0) {
              Serial.println("GPIO 1 Off.");
              output1State = "Off";
              digitalWrite(pinBuzzer, LOW);
            } else if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("GPIO 2 On.");
              output2State = "On";
              digitalWrite(pinRelay, HIGH);
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("GPIO 2 Off.");
              output2State = "Off";
              digitalWrite(pinRelay, LOW);
            }

            //Display the HTML web page.
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            //CSS to style the on/off buttons.
            //Feel free to change the background-color and font-size attributes to fit your preferences.
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            //Web Page Heading.
            client.println("<body><h1>ESP32 Web Server</h1>");

            //Display Temperature & Humidity.

            client.println("<table border=\"2\" width=\"456\" cellpadding=\"10\" align=\"center\"><tbody><tr><td>");
            client.println("<h3>DHT11</h3>");
            client.println("<h3>Temperature = ");
            client.println(numTemperature);
            client.println("&deg;C</h3><h3>Humidity = ");
            client.println(numHumidity);
            client.println("% </h3></td></tr></tbody></table></body></html>");

            //Display current state, and ON/OFF buttons for GPIO 1.
            client.println("<p><h3>GPIO 1 - State " + output1State + "</h3></p>");
            //If the output1State is off, it displays the ON button.
            if (output1State == "Off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            //Display current state, and ON/OFF buttons for GPIO 2.
            client.println("<p><h3>GPIO 2 - State " + output2State + "</h3></p>");
            //If the output2State is off, it displays the ON button.
            if (output2State == "Off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            //The HTTP response ends with another blank line.
            client.println();
            //Break out of the while loop.
            break;
          } else { //If you got a newline, then clear currentLine.
            currentLine = "";
          }
        } else if (c != '\r') {            //If you got anything else but a carriage return character, add it to the end of the currentLine.
          
          currentLine += c;
        }
      }
    }
    
    //Clear the header variable.
    header = "";
    //Close the connection.
    client.stop();
    Serial.println(">> Client disconnected.");
    Serial.println("********************************************************************");
    
    Serial.println("");
  }
}
//************************************************************************************************************************
