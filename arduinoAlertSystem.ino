/*
	- LDR Alarm with Arduino and Carriots -

  This sketch sends streams to Carriots according to the values read by a LDR sensor

	The circuit:
	* 1 arduino uno, 1 ethernet shield, 1 tiny breadboard, 1 resistance, 1 LDR sensor

	Created 27 Dec 2012
	By Carriots
	Modified 10 Apr 2014
	By carriots

*/

#include <SPI.h>
#include <Ethernet.h>

const int ON = 1;    // Constant to indicate that lights are on
const int OFF = 2;  // Constant to indicate that lights are off
const String APIKEY = "YOUR APIKEY HERE"; // Replace with your Carriots apikey
const String DEVICE = "YOUR DEVICE's ID_DEVELOPER HERE"; // Replace with the id_developer of your device

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,27);       // Your IP Address
IPAddress server(51,137,97,16);  // api.carriots.com IP Address

EthernetClient client; // Initialize the library instance

int ledPin = 7;        // Led pin number
int LDRPin = 3;        // LDR sensor pin number

int lights = OFF;      // Current status
int newLights = OFF;   // New status


/**************************************************************************/
/*
    The setup routine runs once when you press reset
*/
/**************************************************************************/
void setup() {

  pinMode(ledPin, OUTPUT);        // Initialize the digital pin as an output

  Serial.begin(9600);             // Start serial port
  Serial.println(F("Starting"));

  Ethernet.begin(mac,ip);         // Start the Ethernet connection
  delay(1000);                    // Give the Ethernet shield a second to initialize
}


/**************************************************************************/
/*
    The loop routine runs over and over again forever
*/
/**************************************************************************/
void loop() {

  int val = analogRead(LDRPin);   // Read the value from the sensor
  Serial.println(val);

  if (val > 990) {                // This is the value limit between day or night with our LDR sensor. Maybe you need adjust this value.
    newLights = OFF;              // Now it's night. We have to turn on the LED
    digitalWrite(ledPin, HIGH);   // Turn the LED on (HIGH is the voltage level)
  }
  else {
    newLights = ON;               // Now it's day. We have to turn off the LED
    digitalWrite(ledPin, LOW);    // Turn the LED off by making the voltage LOW
  }

  if (lights != newLights) {        // Check if we have a change in status
    Serial.println(F("Send Stream"));
    lights = newLights;             // Status update and send stream
    sendStream();
  }

  delay(500);

  // If there's incoming data from the net connection, send it out the serial port
  // This is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (!client.connected()) {
      client.stop();
  }
}


/**************************************************************************/
/*
    Send stream to Carriots
*/
/**************************************************************************/
void sendStream()
{
  String txt = "";         // Text to send
  if ( lights == OFF ) {   // Alarm OFF
     txt = "OFF";
  } else {                 // Alarm ON
     txt = "ON";
  }
  Serial.println(txt);     // For debugging purpose only

  if (client.connect(server, 80)) {   // If there's a successful connection
    Serial.println(F("connected"));
    // Build the data field
    String json = "{\"protocol\":\"v2\",\"device\":\""+DEVICE+"\",\"at\":\"now\",\"data\":{\"light\":\""+txt+"\"}}";

    // Make a HTTP request
    client.println("POST /streams HTTP/1.1");
    client.println("Host: api.carriots.com");
    client.println("Accept: application/json");
    client.println("User-Agent: Arduino-Carriots");
    client.println("Content-Type: application/json");
    client.print("carriots.apikey: ");
    client.println(APIKEY);
    client.print("Content-Length: ");
    int thisLength = json.length();
    client.println(thisLength);
    client.println("Connection: close");
    client.println();

    client.println(json);
  }
  else {
    // If you didn't get a connection to the server:
    Serial.println(F("connection failed"));
  }


}
