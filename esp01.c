#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

// WiFi credentials
const char* ssid = "ESPWIFI";
const char* password = "parola01";
bool ledState = false;
String temperature = "";

// Web server on port 80
ESP8266WebServer server(80);

void setup() {
  //Serial setup
	Serial.begin(115200);

  //  WiFi setup

  // WiFi as a client

	// // Connect to WiFi
	// WiFi.begin(ssid, password);
	// while (WiFi.status() != WL_CONNECTED) {
	// 	delay(1000);
	// 	Serial.println("Connecting to WiFi...");
	// }
  // Serial.println(WiFi.localIP());
	// Serial.println("Connected to WiFi");

  // WiFi as an Access Point (dedicated hotspot)


  Serial.println("");
  Serial.println("Setting AP (Access Point)…");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Route for root / web page
  server.on("/", HTTP_GET, handleRoot);
  server.on("/led", HTTP_GET, handleLED);
  server.on("/message", HTTP_GET, handleMessage);

  // Start server
  server.begin();
  Serial.print('!'); //To confirm wifi and server setup from esp8266
}

void loop() {
  server.handleClient();
}

void getLED() {
	Serial.print('L');
	while(Serial.available() == 0) {} //waiting for arduino response
	ledState = Serial.read(); //reads character 0 or 1
  // Serial.println(ledState);
}

void getTemp() {
	Serial.print('T');
	while(Serial.available() == 0) {} //waiting for arduino response
	temperature = Serial.readStringUntil('.');
	char decimals[3];
	Serial.readBytes(decimals, 2);
	decimals[2] = '\0';
	temperature += String(".") + decimals;
  // Serial.println(temperature);
}

void handleRoot() {
	getLED();
	getTemp();
	String ledStatus = ledState ? "On" : "Off";
	String message = "<h1>Arduino Uno Web Interface</h1>";
	message += "<p>LED is " + ledStatus + "</p>";
	message += "<p>Temperature: " + temperature + " °C</p>";
	message += "<p>Go to /led?state=(on/off) for LED control over the Web interface</p>";
  message += "<p>Go to /message?message=(message) for sending a message over the Web interface</p>";
	server.send(200, "text/html", message);
}

void handleLED() {
	if (server.arg("state") == "on") {
		Serial.print('A');
		while(Serial.available() == 0) {} //waiting for confirmation
		int success = Serial.read();
		if(!success) {
			server.send(201, "text.plain", "NO"); //LED function failed on arduino
		}
	} else if (server.arg("state") == "off") {
		Serial.print('S');
		while(Serial.available() == 0) {} //waiting for confirmation
		int success = Serial.read();
		if(!success) {
			server.send(201, "text/plain", "NO"); //LED function failed on arduino
		}
	} else {
		server.send(301, "text/plain", "Needs state GET parameter");
	}
	server.send(200, "text/plain", "OK");
}

void handleMessage() {
	if (server.arg("message").length()) {
    Serial.print('M'); //Initiates message storage function
    Serial.write(server.arg("message").length()); //sends the literal message length
    Serial.print(server.arg("message")); //sends the message as a string

    while(Serial.available() == 0) {} //waits for status code from arduino
    int status = Serial.read();
    if(!status) {
      server.send(201, "text/plain", "NO"); //message function failed on arduino
    }
	} else {
    server.send(301, "text/plain", "Needs message GET parameter");
  }
	
	server.send(200, "text/plain", "OK");
}