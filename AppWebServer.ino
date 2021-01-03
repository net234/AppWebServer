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

   TODO:  refresh stay a 1000 (after auto refresh from wifisetup)
   TODO: mode AP permanent with no capture
   TODO: better use of ACTION/Message Page
   TODO: better deal with new random / random
**********************************************************************************/



#include <Arduino.h>


#define LED_LIFE      LED_BUILTIN
#define LED2         16
#define BP0 0
#define BP0_DOWN      LOW
#define APP_VERSION   "AppWebServer Validate V1.1"

#define LED_ON        LOW


#include "betaEvents.h"
EventTracker MyEvent(LED_LIFE);   // instance de eventManager


//Objet serveur WEB
#include  "AppWebServer.h"
AppWebServer    MyWebServer;



/* Evenements du Manager (voir betaEvents.h)
  evNill = 0,      // No event
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  //  evDepassement1HZ,
  evLEDOn,
  evLEDOff,
  evInChar,
  evInString,
*/

/* Liste des evenements specifique WEB
  // evenement recu
  evWEBTrySetup = 50,         // A new config setup need to tryed
  evWEBTimerEndOfTrySetup,    // Time out for the try
  evWEBDoReset,               // Reset requested by the user
*/




// Liste des evenements specifique a ce projet
enum tUserEventCode {
  // evenement recu
  evBP0Down = 100,    // BP0 est appuyé
  evBP0Up,            // BP0 est relaché
  evBP0MultiDown,     // BP0 est appuyé plusieur fois de suite
  evBP0LongDown,      // BP0 est maintenus appuyé plus de 3 secondes
  evBP0LongUp,        // BP0 est relaché plus de 1 secondes
  // evenement action
};




bool sleepOk = true;
int  multi = 0; // nombre de clic rapide
uint32_t refFreeMem;

void setup() {
  // IO Setup
  pinMode(BP0, INPUT_PULLUP);
  pinMode(LED2, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_VERSION));

  delay(1000);

  // Start instance
  MyEvent.begin();

  Serial.print(F(" Freemem Start= "));
  Serial.println(MyEvent.freeRam());

  //  ServeurWeb.WiFiMode = WIFI_STA;  // mode par defaut
  MyWebServer.setCallBack_OnTranslateKey(&on_TranslateKey);
  MyWebServer.setCallBack_OnStartRequest(&on_HttpRequest);
  MyWebServer.setCallBack_OnRefreshItem(&on_RefreshItem);
  //  ServeurWeb.setCallBack_OnRepeatLine(&on_RepeatLine);
  MyWebServer.begin(F(APP_VERSION));

  //Check if WEB in flash is the good on  (dont forget to UPLOAD the web on the flash with LittleFS)
  if ( !String(F(APP_VERSION)).startsWith( MyWebServer.getWebName() ) ) {
    Serial.print("Invalid WEB pages '");
    Serial.print(MyWebServer.getWebName());
    Serial.println("'");
    delay(3000);
    ESP.reset();
  }




  refFreeMem = MyEvent.freeRam();
  Serial.print(F(" Freemem Ref= "));
  Serial.println(refFreeMem);

  Serial.println("Bonjour ....");
}




bool BP0_Status = !BP0_DOWN;
byte BP0_Multi = 0;
bool LED2_Status = LED_ON;    // led is on
uint32_t BP0_lastClick = 0;    // last click on board FLASH button en millis



bool trackMem = false;


void loop() {
  MyWebServer.handleEvent();     // handle http service and wifi

  MyEvent.getEvent(sleepOk);
  MyEvent.handleEvent();
  switch (MyEvent.currentEvent.code)
  {

    case ev10Hz:
      jobBP0();  // detection poussoir
      break;


    case ev1Hz:
      if (trackMem) {
        Serial.print(F(" Freemem = "));
        Serial.print(MyEvent.freeRam());
        Serial.print(F(" / "));
        Serial.print( refFreeMem - MyEvent.freeRam() );
        Serial.print(F(" Max block = "));
        Serial.print(ESP.getMaxFreeBlockSize());
        Serial.print(F(" Frag = "));
        Serial.println(ESP.getHeapFragmentation());
      }
      break;

    case ev24H:
      Serial.println("---- 24H ---");
      break;

    case evBP0Down:
      Serial.println(F("BP0 Down"));
      BP0_lastClick = millis();
      MyEvent.setMillisecLED(500, 50);   // LED_LIFE 2HZ ratio 50%
      switchLed2();                      // toogle LED_2
      break;

    case evBP0Up:
      Serial.println(F("BP0 Up"));
      MyEvent.setMillisecLED(1000, 10);   // LED_LIFE 1HZ ratio 10%
      break;

    case evBP0LongDown:
      Serial.println(F("BP0 Long Down"));
      if (multi == 2) {
        Serial.println(F("OPEN AP"));
        MyWebServer.startCaptivePortal(60);
      }

      if (multi == 5) {
        Serial.println(F("RESET"));
        MyEvent.pushEvent(evWEBDoReset);
      }
      break;

    case evBP0LongUp:
      BP0_Multi = 0;
      Serial.println(F("BP0 Long Up"));
      break;

    case evBP0MultiDown:
      multi = MyEvent.currentEvent.param;
      Serial.print(F("BP0 Multi Clic:"));
      Serial.println(multi);

      break;


    case evWEBDoReset:
      Serial.print(F("Reset requested by user"));
      delay(100);
#ifdef  __AVR__
      wdt_enable(WDTO_120MS);
#else
      ESP. restart();
#endif
      while (1)
      {
        delay(1);
      }
      break;


    case evInChar:
      switch (MyEvent.inChar)
      {
        case '0': delay(10); break;
        case '1': delay(100); break;
        case '2': delay(200); break;
        case '3': delay(300); break;
        case '4': delay(400); break;
        case '5': delay(500); break;





        case 't':
          trackMem = !trackMem;
          break;




      }


      break;


    case evInString:

      if (MyEvent.inputString.equals(F("SLEEP"))) {
        sleepOk = !sleepOk;
        Serial.print(F("Sleep=")); Serial.println(sleepOk);
      }

      if (MyEvent.inputString.equals(F("FREE"))) {
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
      }

      if (MyEvent.inputString.equals(F("RESET"))) {
        Serial.println(F("RESET"));
        MyEvent.pushEvent(evWEBDoReset);
      }

      if (MyEvent.inputString.equals(F("WIFIOFF"))) {
        Serial.println("setWiFiMode(WiFi_OFF)");
        WiFi.mode(WIFI_OFF);
      }

      if (MyEvent.inputString.equals(F("WIFISTA"))) {
        Serial.println("setWiFiMode(WiFi_STA)");
        WiFi.mode(WIFI_STA);
      }

      if (MyEvent.inputString.equals(F("WIFIAP"))) {
        Serial.println("setWiFiMode(WiFi_AP)");
        WiFi.mode(WIFI_AP);
      }


      if (MyEvent.inputString.equals(F("STATION"))) {
        Serial.println("Start Station");
        WiFi.begin();
      }

      if (MyEvent.inputString.equals(F("AP"))) {
        Serial.println("Start AP");
        WiFi.softAP(MyWebServer._deviceName);
      }
      // TODO: a full raz config
      if (MyEvent.inputString.equals(F("RAZCONFIG"))) {
        Serial.println("raz config file");
        MyWebServer.razConfig();
      }


  }
}

//------------------------ function local ----------------------------------------

// deal with bp0 to generate evBP0Down, evBP0UP, evBP0LongDown , evBPLongUp , evBP0MultiDown
void jobBP0() {
  if ( BP0_Status == digitalRead(BP0) ) return;  // no change on BP0

  // changement d'etat BP0
  BP0_Status = !BP0_Status;
  if ( BP0_Status == BP0_DOWN ) {

    MyEvent.pushEvent(evBP0Down);
    MyEvent.pushDelayEvent(3000, evBP0LongDown); // set an event BP0 long down
    MyEvent.removeDelayEvent(evBP0LongUp);       // clear the event BP0 long up
    if ( ++BP0_Multi > 1) {
      MyEvent.pushEvent(evBP0MultiDown, BP0_Multi);
    }
  } else {

    MyEvent.pushEvent(evBP0Up);
    MyEvent.pushDelayEvent(1000, evBP0LongUp);   // set an event BP0 long up
    MyEvent.removeDelayEvent(evBP0LongDown);     // clear the event BP0 long down
  }
}

void switchLed2() {
  LED2_Status = ! LED2_Status;
  digitalWrite(LED2, LED2_Status);

  Serial.print("Led is ");
  Serial.println(getLedStatus());

}

String getLedStatus() {
  // ledState to say if led is on or off
  if ( LED2_Status == LED_ON ) {
    return ("LED ON");
  }
  return ("LED OFF");
}



//------------------------- call back WEB --------------------------------------------

////// === call back to handle request (grab the "Swith the led button") ============================


void on_HttpRequest(const String &filename, const  String &submitValue) {
  if  ( submitValue ==  "switchLed" ) {
    switchLed2();
  }
}


////// ==== callback to display data on page /////////////

//on_TranslateKey is call on each tag "[#key#]" the tag will be replaced by the value you put in 'key' parameter
//webdemo request 2 key
void on_TranslateKey(String & key) {

  if ( key.equals("APP_VERSION") ) {

    //  APP_VERSION appname wich is displayed on almost every page
    key = APP_VERSION;

  } else if ( key.equals("ledStatus") ) {
    key = getLedStatus();

  } else if ( key.equals("lastClickTime") ) {

    // lastClickTime in second
    key = String( (millis() - BP0_lastClick) / 1000);
  } else {

    // to track typo on key
    Serial.print("Missing key '");
    Serial.print(key);
    Serial.println("'");
    key = "?" + key + "?"; // to make a visual on the page
  }
}


// call back to display dynamic data on the web page  (class=refresh) ----------------------------------
bool on_RefreshItem(const String & keyname, String & key) {
  if ( keyname.equals("ledStatus") ) {
    String aText = getLedStatus();
    if (aText != key) {
      key = aText;
      return (true);  // text is chaged display it
    }
    return (false);  // text is same do nothing
  }

  if ( keyname.equals("lastClickTime") ) {
    String aText = String( (millis() - BP0_lastClick) / 1000);
    if (aText != key) {
      key = aText;
      return (true);  // text is chaged display it
    }
    return (false);  // text is same do nothing
  }


  // timer for the refresh rate  300 is good to display seconds
  if ( keyname.equals("refresh") ) {
    if (key.toInt() != 300) {
      key = 300;
      return (true);
    }
    return (false);
  }

  // track unanswred refresh
  Serial.print(F("Got unknow refresh "));
  Serial.print(keyname);
  Serial.print(F("="));
  Serial.println(key);


  return (false);
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
