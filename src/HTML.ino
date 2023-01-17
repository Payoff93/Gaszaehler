// ****************************************************************
// Sketch Gaszähler Webinterface Modular(Tab)
// created: Jens Fleischer, 2018-04-08
// last mod: Jens Fleischer, 2021-08-28
// For more information visit: https://fipsok.de
// ****************************************************************
// Hardware: Esp8266
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

void homepage() {     // Funktionsaufruf "homepage();" muss im Setup eingebunden werden
  server.on("/", handleRoot);
  server.on("/detail", handledetail);
  server.on("/ledh", []() {
    ledh = !ledh;
    handledetail();
  });
  server.on("/ledz", []() {
    ledz = !ledz;
    handledetail();
  });
}

void handledetail() {
  if (server.arg(0).length() > 0 && server.arg(0).length() <= 7) {
    value = server.arg(0).toInt();
    speichern();                                  // Benutzer Eingabe in der Datei speichern
    Serial.printf("Zählerstand vom Client: %ld\n", value);
  }
  char buf[43];
  snprintf(buf, sizeof(buf), "[\"%05ld,%02ld9  m³\",\"%d\",\"%d\",\"%s\"]", value / 100, value % 100, ledh, ledz, Fehlerzeit().c_str());
  server.send(200, "application/json", buf);     // Zählerstand und Led Status an Html Seite senden
}

String Fehlerzeit() {   // ermittelt die vergangene Zeit seit dem letztem Verbindungsfehler zu Thingspeak
  if (fehlerZeit > 0) {
    String Time = "";
    uint8_t mm = (millis() / 1000 - fehlerZeit) / 60 % 60;
    uint16_t hh = ((millis() / 1000 - fehlerZeit) / 60) / 60;
    if (hh > 240) fehlerZeit = 0;      // nach 10 Tagen ohne Fehler, wird die Fehleranzeige zurückgesetzt
    if (hh < 10)Time += "0";
    Time += (String)hh + ":";
    if (mm < 10)Time += "0";
    Time += (String)mm;
    return Time;
  }
  return "0";
}

void handleRoot() {   // Html Webseite, der Zählerstand wird aller 5 Sekunden mittels fetch.api aktuallisiert
  String temp = "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0,'><title>Gaszähler</title><script>";
  temp += "window.addEventListener('load',function(){document.querySelector('#grey').addEventListener('click',function(){e()});document.querySelector('#ledz').addEventListener";
  temp += "('click',function(){d('ledz')});document.querySelector('#ledh').addEventListener('click',function(){d('ledh')});document.querySelector('input').addEventListener('blur',";
  temp += "function(){b(inObj=document.querySelector('input'))});var c=document.querySelector('#note');function b(f){!f.checkValidity()?(c.innerHTML=f.validationMessage,";
  temp += "c.classList.add('note')):d('detail',inObj.value)};var a;function e(){document.getElementById('no').style.display=a?'none':'flex';a=!a;c.innerHTML='';";
  temp += "c.classList.remove('note');document.querySelector('form').reset()}function d(g,f){if(Number.isInteger(parseInt(f))){e()}fetch(g,{method:'POST',body:f})";
  temp += ".then(function(h){return h.json()}).then(function(j){document.querySelector('#wert').innerHTML=j[0];var h=document.querySelector('#ledh');var i=document.querySelector";
  temp += "('#ledz');if(j[1]=='0'){h.innerHTML='LED On',h.classList.remove('buttonon')}else{h.innerHTML='LED Off',h.classList.add('buttonon')}if(j[2]=='0'){i.innerHTML=";
  temp += "'LED On',i.classList.remove('buttonon')}else{i.innerHTML='LED Off',i.classList.add('buttonon')}if(j[3]!='0')document.querySelector('#led').innerHTML='Sendefehler vor '";
  temp += "+j[3]+' hh:mm';})}document.addEventListener('DOMContentLoaded',d('detail'));setInterval(d,5000,'detail')});</script><style>body{background-color:tan;display:flex;";
  temp += "flex-flow:column;align-items:center;padding:10px;font-size:18px}#grey,#no{display:flex;flex-flow:column;align-items:center;background-color:#c5c3c3;width:280px;";
  temp += "height:150px;border:.3em solid #aeaeab;box-shadow:5px 10px 5px rgba(0,0,0,0.7);border-radius:.5em}#black{background-color:black;width:14em;height:2em;";
  temp += "border-radius:.2em}#red{border:.27em solid red;position:relative;width:3.7em;height:1.5em;left:6.4em}#wert{color:white;position:relative;top:-2.2em;left:.1em;";
  temp += "letter-spacing:.2em;font-size:1.6em;font-weight:bold}#no{display:none}.note{background-color:#fecdee;padding:.3em;margin-top:1.5em;text-align:center;max-width:17em;";
  temp += "border-radius:.5em;position:relative;top:-13.3em}input,#led{height:30px;margin-top:1em}.button{background-color:#75a27e;height:40px;width:120px;font-size:16px;";
  temp += "box-shadow:5px 5px 5px rgba(0,0,0,0.7)}.buttonon{background-color:#cf848c}</style></head><body class='flexg'>  <div id ='grey'><h1>Zählerstand</h1><div id='black'>";
  temp += "<div id='red'></div></div></div><span id='wert'></span><div id ='no'><form><input placeholder=' Zählerstand eingeben' pattern='[0-9]{1,7}' title='Nur 1 bis";
  temp += " 7 Ziffern sind erlaubt' required></form><div id='led'> Zähler - Led - Heartbeat</div><div><button id='ledz' class='button'>LED On/Off</button> <button id='ledh' ";
  temp += "class='button'>LED On/Off</button></div></div><div id='note'></div></body></html>";
  server.send(200, "text/html", temp);
}
