#include <OneWire.h>
#include <DallasTemperature.h>

#include <Arduino.h>
#include <EEPROM.h>
#include <string.h>

// Pins
//GPIO27 == Pin 27 on the esp32 (left side, count towards the usb port down)
//GPIO13 == Pin 15 on the esp32 (left side, count towards the usb port down)
#define DS18B20_PIN 27
#define LED_PIN 13

// Variables
float temperature = 0;
bool ledState = false;
int eeprom_addresses[] = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450};
int current_message_addr = 0; //which message are we currently on

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
	Serial.begin(115200);
	pinMode(LED_PIN, OUTPUT);

	Serial.setTimeout(60*10*1000); //10 minutes for wifi connection and time between inputs
	while(Serial.available() == 0) {} //waiting for UART input from ESP8266
  //waiting for setup to be done on wifi module
	Serial.readStringUntil('!'); //from ESP8266 code, it only appears when connected

  EEPROM.begin(500);
}

void loop() {
	uartCommand();
}

void readTemperature() {
	sensors.requestTemperatures();
	temperature = sensors.getTempCByIndex(0); // Get DS18B20 temperature as per documentation
}

void uartCommand() {
	if (Serial.available() > 0) {
		char command = Serial.read();
		if (command == 'A') {
			try {
				ledState = true;
				digitalWrite(LED_PIN, HIGH);
				Serial.write(1);
			} catch (int e) {
				Serial.write(0);
			}
		} else if (command == 'S') {
			try {
				ledState = false;
				digitalWrite(LED_PIN, LOW);
				Serial.write(1);
			} catch (int e) {
				Serial.write(0);
			}
		} else if (command == 'T') {
			readTemperature();
			Serial.print(temperature, 2);
		} else if (command == 'L') {
			Serial.write(ledState);
		} else if (command == 'M') {
			writeMessage();
		}
	}
}

void writeMessage() {
	int message_size = Serial.read();
	if (message_size <= 50) {
		char message[50];
		Serial.readBytes(message, message_size);
		sendToEEPROM(message);
		Serial.write(1); //success
	} else {
		Serial.write(0); //failed
	}
}

void sendToEEPROM(char *message) {
	if (current_message_addr >= 9) {
		shiftEEPROMMessages();
		current_message_addr = 9;
	}
	
	int curr_addr = eeprom_addresses[current_message_addr++];
	for(int i=0;i<strlen(message);i++) {
		EEPROM.write(curr_addr++, message[i]);
		EEPROM.commit();
	}
}

void shiftEEPROMMessages() {
	for(int i=0;i<9;i++) { //iterating through message blocks
		for(int k=0;k<50;k++) { //iterating through each character
			EEPROM.write(i*50+k, EEPROM.read((i+1)*50+k));
			EEPROM.commit();
		}
	}
}