// ****************************************************************
// Sketch Thingspeak Modular(Tab) (Antwort nicht blockierend)
// created: Jens Fleischer, 2018-04-08
// last mod: Jens Fleischer, 2018-05-05
// For more information visit: https://fipsok.de
// ****************************************************************
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

WiFiClient client;

bool thingspeak(const char* value) {       // Funktionsaufruf "thingspeak();" muss in loop eingebunden sein
  static bool error = false, response = true;
  const uint16_t httpPort = 80;
  const char* host = "184.106.153.149";
  const char* APIKEY = {"WRITEAPIKEY"};                 // trage deinen ApiKey von https://thingspeak.com
  const unsigned long interval = 1000UL * 60;              // Interval in Sekunden einstellen
  static unsigned long letzteMillis = 0 - interval / 2;     // Sendebeginn einstellen ("- interval" = sofort, "0" = nach Ablauf von Interval)
  const uint16_t timeout = 2000;                            // Zeitraum für Fehlerrückgabe in Millisekunden einstellen
  const String path = (String)"&field1=" + value;     // "field1 bis 8" anpassen

  /*************** Thinkspeak senden *********************/

  unsigned long aktuelleMillis = millis();
  if (aktuelleMillis - letzteMillis >= interval || error) {       // senden im Interval und erneut im Fehlerfall
    error = false;
    letzteMillis = aktuelleMillis;
    if (!client.connect(host, httpPort)) {
      Serial.println("Thingspeak Verbindung fehlgeschlagen !");
      error = true;
      return false;
    }
    else {
      client.printf("POST /update?key=%s%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", APIKEY, path.c_str(), host);
      response = 0;
      return true;
    }
  }

  /************** Thinkspeak auf Anwort prüfen *****************/

  if (!response) {
    if (millis() - letzteMillis > timeout) {
      Serial.println("Thingspeak Client Timeout !");
      error = true;
      return false;
    }
    if (client.available()) {               // einlesen der Antwort
      Serial.printf("Thingspeak Antwort nach: %4ld ms\n", millis() - letzteMillis); // zeigt die Zeit bis zur Antwort --> passe den Timeout entsprechend an
      String line = client.readStringUntil('\n');
      if (line.startsWith("HTTP/1.1 200 OK")) {    // wenn Antwort mit "HTTP/1.1 200 OK" beginnt war das Senden erfolgreich
        client.stop();
        response = true;
      }
    }
  }
  return true;
}
