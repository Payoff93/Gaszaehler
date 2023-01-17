// ****************************************************************
// Sketch Gaszähler Modular(Tab)
// created: Jens Fleischer, 2018-03-30
// last mod: Jens Fleischer, 2021-08-28
// For more information visit: https://fipsok.de
// ****************************************************************
// Hardware: Esp8266
// D2 = GPIO4  Anschluss Reedrelais vom GPIO4 auf GND
// 10k Pullup-Widerstand von VCC auf GPIO4
// Versorgung über USB
// Software: Esp8266 Arduino Core 2.4.2 / 2.5.2 / 2.6.3 / 2.7.4
// Getestet auf: Nodemcu
/******************************************************************
  Copyright (c) 2018 Jens Fleischer. All rights reserved.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This file is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*******************************************************************/

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <ESP8266WebServer.h>     // http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#server
#include <ArduinoOTA.h>           // http://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html
#include <FS.h>                   // http://arduino-esp8266.readthedocs.io/en/latest/filesystem.html

const byte inputPIN = D7;         // Reedrelais
const byte HeartbeatLed = D4;     // OnBoardLed Esp 12E
const byte CounterLed = D4;       // OnBoardLed NodeMcu 1.0
unsigned long value, oldvalue;
bool ledz = HIGH, ledh = LOW;    // Merker für ZählerLed AN/Aus , Merker für HeartbeatLed AN/Aus
unsigned long fehlerZeit;         // Zeitpunkt des letzten Fehlers

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.printf("\n\nSketchname: %s\nBuild: %s\t\tIDE: %d.%d.%d\n%s\n\n",
                (__FILE__), (__TIMESTAMP__), ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10, ESP.getFullVersion().c_str());
  pinMode(HeartbeatLed, OUTPUT);
  pinMode(CounterLed, OUTPUT);
  digitalWrite(HeartbeatLed, HIGH);
  Connect();    // ruft die Funktion zum Verbinden mit dem Wlan auf
  homepage();   // ruft die Funktion zum bereitstellen einer Webseite auf
  Serial.println(SPIFFS.begin() ? "SPIFFS gestartet!" : "Sketch wurde mit \"no SPIFFS\" kompilliert!\n");
  server.begin();
  Serial.println("HTTP Server gestartet!\n");

  ArduinoOTA.onStart([]() {                         // Zählerstand speichern bevor OTA Update
    speichern();
  });
  ArduinoOTA.begin();

  File f = SPIFFS.open("/value.txt", "r");          // Zählerstand beim Start aus Datei lesen
  String str = f.readStringUntil('\n');
  value = str.toInt();
  oldvalue = value;
  f.close();
}

void loop() {
  static unsigned long letzterAufruf = 0;
  ArduinoOTA.handle();
  server.handleClient();
  zaehlerauslesen();  // Funktionsaufruf
  char buf[12];
  snprintf(buf, sizeof(buf), "%li.%02li", value / 100, value % 100);
  if (!thingspeak(buf)) {
    fehlerZeit = millis() / 1000;
  }
  ledh ? digitalWrite(HeartbeatLed, millis() % 500 >= 250) : digitalWrite(HeartbeatLed, HIGH);    // LED zeigt an das der Sketch läuft
  unsigned long aktuelleMillis = millis();
  if (aktuelleMillis - letzterAufruf >= 60000UL * 30 && value != oldvalue) {       // Zählerstand speichern() jede 0.5 Stunde, wenn der Wert sich geändert hat
    speichern();
    oldvalue = value;
    letzterAufruf = aktuelleMillis;
  }
}

void zaehlerauslesen() {
  bool aktueller = 1;
  static bool vorheriger = 1;
  static unsigned long letzteMillis = 0;
  unsigned long aktuelleMillis = millis();
  if (aktuelleMillis - letzteMillis >= 100) {       // Das Reedrelais wird 10 mal pro Sekunde abgefragt
    aktueller = digitalRead(inputPIN);
    if (!aktueller && aktueller != vorheriger) {
      Serial.println(value);
      value++;
    }
    vorheriger = aktueller;
    letzteMillis = aktuelleMillis;
  }
  !ledz ? digitalWrite(CounterLed, HIGH) : !vorheriger ? digitalWrite(CounterLed, millis() % 500 >= 250) : digitalWrite(CounterLed, HIGH);    // LED blinkt wenn Reedrelais geschlossen
}

void speichern() {
  File f = SPIFFS.open("/value.txt", "w");
  f.printf("%li\n", value);
  f.close();
}
