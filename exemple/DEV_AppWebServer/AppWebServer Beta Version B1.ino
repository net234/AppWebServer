/*************************************************
 *************************************************
    Sketch AppWebServer.ino   validation of lib AppWebServer - Arduino lib for easy wifi interface with http and css
    Copyright 2020 Pierre HENRY net23@frdev.com All - right reserved.


  This file is part of AppWebServer.

    AppWebServer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppWebServer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppWebServer.  If not, see <https://www.gnu.org/licenses/gpl.txt>.



  History
  cleaner version of WebServer (C) V1.2 6/6/2020  NET234 P.HENRY net23@frdev.com

   V1.0    Extracted from Betaporte
   V1.0.1  Add interactive js
   V1.0.2  Stand alone captive portal

**********************************************************************************/



#include <Arduino.h>


#define LED_LIFE      LED_BUILTIN
#define APP_VERSION   "AppWebServer Validate V1.0.4"

#define LED_ON        LOW
//Objet serveur WEB
#include  "AppWebServer.h"
AppWebServer    ServeurWeb;
//int translateKey;
//
//bool onTranslateKeyPtr;
//bool redirectUri;
//bool localIp;
//bool compareNetwork = true;

//Données demandées par les pages web par les tag "[#key#]"
void on_TranslateKey(String & key) {
  if ( key.equals(F("APP_VERSION")) ) {
    key = APP_VERSION;
  } else {
    Serial.print(F("WEB MISS KEY='"));
    Serial.print(key);
    Serial.println(F("'"));
    key += "???";
  }
}

//Donnée de refraichissement demandée par les pages web
bool on_RefreshItem(const String & keyname, String & key) {
  //  Serial.print(F("Got refresh "));
  //  Serial.print(keyname);
  //  Serial.print(F("="));
  //  Serial.println(key);

  if ( keyname.equals("CLOCK") ) {
    long now = millis() / 1000;
    byte min = (now / 60) % 60;
    String akey;
    if (min < 10) akey += '0';
    akey += String(min);
    akey += ':';
    byte sec = now % 60;
    if (sec < 10) akey += '0';
    akey +=  String(sec);
    if (akey != key) {
      key = akey;
      return (true);
    }
    return (false);
  }
  if ( keyname.equals("refresh") ) {
    if (key.toInt() != 300) {
      key = 300;
      return (true);
    }
    return (false);
  }
  if ( keyname.equals("RESULTAT") ) {
    String res = F("Bonjour à tous");
    if (res != key) {
      key = res;
      return (true);
    }
    //    if (key.length() < 5) {
    //      key = F("Le_bruit_de_la_mer_....__________");
    //      return (true);
    //    }
    //    key += key[0];
    //    key.remove(0,1);
    //    return (true);
  }
  return (false);
}


//
bool resetRequested = false;
long refFreeMem;




void setup() {

  // Initialisation Hard des IO
  pinMode(LED_LIFE, OUTPUT);
  // init Serial
  Serial.begin(115200);
  Serial.println(F(APP_VERSION));
  Serial.print(F(" Freemem Start= "));
  Serial.println(ESP.getFreeHeap());

  Serial.setDebugOutput(true);  // affichage du debug mode pour webserve


  //  ServeurWeb.WiFiMode = WIFI_STA;  // mode par defaut
  ServeurWeb.setCallBack_OnTranslateKey(&on_TranslateKey);
  //ServeurWeb.setCallBack_OnRequest(&HttpRequest);
  ServeurWeb.setCallBack_OnRefreshItem(&on_RefreshItem);
  //  ServeurWeb.setCallBack_OnRepeatLine(&on_RepeatLine);
  ServeurWeb.begin();
  refFreeMem = ESP.getFreeHeap();
  Serial.print(F(" Freemem Ref= "));
  Serial.println(refFreeMem);

}



bool track = false;


void loop() {
  //  // run AP mode on BP0
  //  if (digitalRead(0) == LOW) {
  //    ServeurWeb.configureWiFi(true);
  //  }
  ServeurWeb.handleEvent();     // handle http service and wifi

  //Check ServeurWeb Status
  static    long parsec = 0;
  parsec++;
  static long timer = millis();
  if (millis() - timer > 1000) {
    timer += 1000;
    if (resetRequested) {
      delay(1000);
      ESP.reset();
      while (1) {};
    }
    if (track) {
      Serial.print("loop = ");
      Serial.print(parsec);
      Serial.print(F(" Freemem = "));
      Serial.print(ESP.getFreeHeap());
      Serial.print(F(" / "));
      Serial.println( refFreeMem - ESP.getFreeHeap() );

    }
    parsec = 0;

  }

  //  if (ServeurWeb.WiFiModeChanged) {
  //    //{ twm_WIFI_OFF = 0, twm_WIFI_STA, twm_WIFI_AP,twm_WIFI_APSETUP };
  //    switch (ServeurWeb.getWiFiMode()) {
  //      case twm_WIFI_OFF:
  //        Serial.println(F("TW wifi Mode OFF"));
  //        break;
  //      case twm_WIFI_STA:
  //        Serial.println(F("TW wifi Mode Station"));
  //        break;
  //      case twm_WIFI_AP:
  //        Serial.println(F("TW wifi Mode AP"));
  //        break;
  //      case twm_WIFI_APSETUP:
  //        Serial.println("TW wifi Mode AP Setup");
  //        break;
  //      default:
  //        Serial.print(F("TW mode ?:"));
  //        Serial.println(ServeurWeb.getWiFiMode());
  //    } //switch
  //  }// if Mode changed




  //  if (ServeurWeb.WiFiStatusChanged) {
  //    //WIFI_OFF, WIFI_OK, WIFI_DISCONNECTED, WIFI_TRANSITION
  //    switch (ServeurWeb.getWiFiStatus()) {
  //      case tws_WIFI_TRANSITION:
  //        Serial.println(F("TS wifi en transition"));
  //        break;
  //      case tws_WIFI_OFF:
  //        Serial.println(F("TW wifi off"));
  //        digitalWrite(LED_LIFE, !LED_ON);
  //        break;
  //      case tws_WIFI_DISCONNECTED:
  //        Serial.println(F("TW wifi Deconnecte"));
  //        digitalWrite(LED_LIFE, !LED_ON);
  //        break;
  //      case tws_WIFI_OK:
  //        digitalWrite(LED_LIFE, LED_ON);
  //        Serial.println("TW wifi station Connected");
  //        break;
  //      default:
  //        Serial.print(F("TW Status?:"));
  //        Serial.println(ServeurWeb.getWiFiStatus());
  //    } //switch
  //  }// if status changed

  if (Serial.available())
  {
    char inChar = (char)Serial.read();
    switch (inChar) {
      //      case 'B':
      //        Serial.println("Wifi.mode(OFF)");
      //
      //        WiFi.mode(WIFI_OFF);
      //
      //        delay(100);
      //        break;
      //      case 'C':
      //        Serial.println("Wifi.mode(AP)");
      //        WiFi.mode(WIFI_AP);
      //        break;
      //      case 'D':
      //        Serial.println("Wifi.mode(STA)");
      //        WiFi.mode(WIFI_STA);
      //        break;
      //      case 'E':
      //        persistentStat = !persistentStat;
      //        Serial.print("persistent ");
      //        Serial.println(persistentStat);
      //        Serial.setDebugOutput(persistentStat);
      //        break;
      //      case 'G':
      //        Serial.println("Hostname = ''");
      //        ServeurWeb.setHostname("");
      //        break;

      //      case 'H':
      //        Serial.println("Hostname = APPWEB");
      //        ServeurWeb.setHostname("APPWEB");
      //        break;

      //      case 'I':
      //        Serial.println("forceSleepBegin");
      //
      //        WiFi.forceSleepBegin();
      //        delay(100);
      //        break;



      //      case 'G':
      //        debugStat = !debugStat;
      //        Serial.print("Debug ");
      //        Serial.println(debugStat);
      //        Serial.setDebugOutput(debugStat);
      //        break;

      //      case 'L':
      //        listenInterval = (listenInterval + 1) % 11;
      //        Serial.print("Light listenInterval ");
      //        Serial.println(listenInterval);
      //
      //        WiFi.setSleepMode (WIFI_LIGHT_SLEEP, listenInterval);
      //        break;
      //      case 'M':
      //        listenInterval = (listenInterval + 1) % 11;
      //        Serial.print("Modem listenInterval ");
      //        Serial.println(listenInterval);
      //
      //        WiFi.setSleepMode (WIFI_MODEM_SLEEP, listenInterval);
      //        break;

      case '0':
        Serial.println("setWiFiMode(WiFi_OFF)");
        WiFi.mode(WIFI_OFF);
        break;
      case '1':
        Serial.println("setWiFiMode(WiFi_STA)");

        WiFi.mode(WIFI_STA);
        break;
      case '2':
        Serial.println("setWiFiMode(WiFi_AP)");
        WiFi.mode(WIFI_AP);
        break;

      case '3':
        Serial.println("setWiFiMode(WiFi_AP)");
        WiFi.mode(WIFI_AP_STA);
        break;

      case 'S':
        Serial.println("Start Station");
        WiFi.begin();
        break;

      case 'A':
        Serial.println("Start AP");
        WiFi.softAP(ServeurWeb._deviceName);
        break;



      case 'W':
        Serial.println("setWiFiMode(WiFi_STA with SSID & PASS)");
        //    ServeurWeb.setWiFiMode(WIFI_STA, "mon_wifi", "ultrasecret");
        break;

      case 'R':
        Serial.println("reset.");
        delay(3000);
        ESP.reset();
        while (1) {};
        break;
      case 'T':
        track = !track;
        break;
      case 'E':
        Serial.println("raz config file");
        ServeurWeb.razConfig();
        break;

      case 'L':
        Serial.print("long delay ");
        for (int N = 1; N <= 30; N++) {
          delay(1000);
          Serial.print('.');
        }
        Serial.println();
        break;

    }
  }
  delay(1);
}// loop
