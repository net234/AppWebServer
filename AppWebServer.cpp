/*************************************************
 *************************************************
    AppWebServer.cpp  is part of of lib AppWebServer - Arduino lib for easy wifi interface with http and css
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

   V1.0    Extracted from Betaporte
   V1.0.1  Add interactive js
   V1.0.2  Stand alone captive portal
   Version B02  01/2020  Ajout des events  (BetaEvents.lib)
   Version B03  13/01/2020   Add csv


   TODO:  refresh stay a 1000 (after auto refresh from wifisetup)
   TODO: mode AP permanent with no capture
   TODO: better use of ACTION/Message Page
   TODO: better deal with new random / random
*/

#include "AppWebServer.h"
//#include <ESP8266WiFi.h>

#include "betaEvents.h"  // needed to be Event User of the Sketch Instance

// pointeur vers l'instance utilisateur
AppWebServer*    AppWebPtr = NULL;

#include "AppWebConfig.h"  //include LittleFS
FileConfig  TWConfig;

#include <ESP8266WebServer.h>    // web server to answer WEB request
#include <ESP8266mDNS.h>         // need to answer   devicename.local  request
#include <DNSServer.h>           // used by Captive Portal to redirect web request to main page


// Server Instance
//#include "user_interface.h"
ESP8266WebServer   Server(SERVER_PORT);    // Serveur HTTP
DNSServer          dnsServer;

namespace TWS {
// Out of instance variables
String  redirectUri;        // uri to redirect after an onSubmit (used by AppWebHttp)
String  localIp;
String  TryStatus;
// Out of instance function
#include "AppWebCaptive.h"    //out of instance functions
#include "AppWebHTTP.h"    //out of instance functions
}

using namespace TWS;


// Objet AppWeb
//// constructeur
//TODO: cant Serial.print here    should throw a system error
AppWebServer::AppWebServer() {
  if (AppWebPtr != nullptr) {
    _error = 998;
  }
  if (EventManagerPtr == nullptr) {
    _error = 997;
  }
  _error = 0;
  AppWebPtr = this;
}
// Destructor
AppWebServer::~AppWebServer() {
  if (AppWebPtr == NULL) return;
  this->end();
  AppWebPtr = NULL;
}


void AppWebServer::end() {
  //  WiFi.mode(WIFI_OFF);
  //  //  softAP = false;
  //  delay(10);
  //  WiFi.SleepBegin();
  //  delay(10);
  Server.close();
  ////  delay(10);
}




void AppWebServer::begin(const String devicename , const int debuglevel  ) {
  // FS
  if (!TWFS.begin()) {
    D1_println(F("TW: FS en erreur  !!!!!"));
  }
  // init random seed
  randomSeed(micros());
  createRandom();        //fill up first _random;
  // Recuperation du fichier de config
  TWConfig.read();
  _defaultWebFolder = TWConfig.defaultWebFolder;
  if (_defaultWebFolder.length() == 0) _defaultWebFolder = F("/web");
  _captiveWebFolder = TWConfig.captiveWebFolder;
  if (_captiveWebFolder.length() == 0) _captiveWebFolder = F("/web/wifisetup");  // todo: should be "/captive" ??
  _captiveAP = true;  //TODO  a valid AP mode without captive

  // if no name
  if (TWConfig.deviceName.length() == 0) TWConfig.deviceName = devicename;
  _debugLevel = debuglevel;
  Serial.setDebugOutput(_debugLevel > 2);



  // grab WiFi actual status
  D_println(F("tws: Read wifi current status "));
  D_print(F("tws: WIFI Mode "));
  D_println(WiFi.getMode());
  D_print(F("tws: SoftAP SSID "));
  D_println(WiFi.softAPSSID());
  D_print(F("tws: SoftAP IP "));
  D_println(WiFi.softAPIP());

  // controle de la configuration du WiFi et de la configuration demandée
  //reconfig eventuelle de l'ip AP
  //if ( (WiFi.getMode() & WIFI_AP) && (WiFi.softAPIP() != IPAddress(10, 10, 10, 10)) ) {
  if ( WiFi.softAPIP() != IPAddress(10, 10, 10, 10) ) {
    WiFi.persistent(false);
    WiFiMode_t mode = WiFi.getMode();
    D_print(F("WS: reconfig APIP 10.10.10.10"));
    IPAddress local_IP(10, 10, 10, 10);
    IPAddress mask(255, 255, 255, 0);
    bool result = WiFi.softAPConfig(local_IP, local_IP, mask);
    D_print(F(" ==> ")); D_println(result);
    D_print(F("SW: SoftAP IP = "));
    D_println(WiFi.softAPIP());
    WiFi.mode(mode);
    WiFi.persistent(true);
  }

  // TODO: deal correctly with AP non captive !!!!!
  _captivePortalActive = WiFi.getMode() & WIFI_AP;


  _deviceName = WiFi.softAPSSID();              //device name from WiFi
  //TODO: revoir les persistant true/false au boot
  //      probalement creer une function saveWiFiDeviceNameInFlash
  if ( TWConfig.deviceName != _deviceName) {
    D_print(F("SW: need to init WiFi same as config   !!!!! "));
    setDeviceName(TWConfig.deviceName);  //check devicename validity
    WiFi.softAP(_deviceName);
    //WiFi.persistent(true);
    if (TWConfig.deviceName != WiFi.softAPSSID()) {
      D_print(F("SW: need to need to rewrite config   !!!!! "));
      TWConfig.deviceName = WiFi.softAPSSID();  //put back devicename in config if needed
      TWConfig.changed = true;
      TWConfig.save();
    }
  }

  if ( TWConfig.bootForceAP > 0 && !(WiFi.getMode() & WIFI_AP) ) {
    D1_println(F("WEB: Force mode Captive AP !!!"));
    startCaptivePortal(60);
  }


  D_print(F("TW: softap="));
  D_println(WiFi.softAPSSID());
  // mise en place des call back web
  Server.onNotFound(HTTP_HandleRequests);
  D_println(F("TW: Serveur.begin"));
  Server.begin();
  D_print(F("TW: AP WIFI Mode "));
  D_println(WiFi.getMode());
  IPAddress myIP = WiFi.softAPIP();
  D_print(F("TW: AP IP address: "));
  D_println(myIP);

  //  if (WiFi.getMode() != WIFI_OFF ) {
  //    bool result = MDNS.begin(_deviceName);
  //    Serial.print(F("TWS: MS DNS ON : "));
  //    Serial.print(_deviceName);
  //    Serial.print(F(" r="));
  //    Serial.println(result);
  //  }

  return ;
}

// set device name
// Check for valid name otherwhise DEFAULT_DEVICENAME "*" is used
// if device name terminate with *  we add some mac adresse number
//   usefull if you setup different device at the same place
// device name is used as APname and as DNSname

void AppWebServer::setDeviceName(const String devicename) {
  _deviceName = devicename;
  // Check a valid hostname
  // configation du nom du reseau AP : LITTLEWEB_XXYY  avec les 2 dernier chifre hexa de la mac adresse
  if (_deviceName.length() > 30) _deviceName.remove(30);   // chop at 30char
  _deviceName.trim();
  if (_deviceName.length() < 4 ) _deviceName = F(DEFAULT_DEVICENAME "*");
  if (_deviceName.endsWith(F("*"))) {
    _deviceName.remove(_deviceName.length() - 1);
    _deviceName += WiFi.macAddress().substring(12, 14);
    _deviceName += WiFi.macAddress().substring(15, 17);
  }
  _deviceName.replace(' ', '_');
  _deviceName.replace('.', '_'); // "." not ok with mDNS
}



void AppWebServer::handleEvent() {
  // deal with event receved
  switch (EventManagerPtr->currentEvent.code)
  {
    case evNill: break;
    case evWEBTrySetup:              jobWEBTrySetup(); break;          // A new config setup need to try
    case evWEBTimerEndOfTrySetup:
      D_print(F("WIF: : timeout TrySetup Station "));
      jobWEBTrySetupAbort();
      break;     // Abort try setup (time out)

    case evWEBTimerEndOfCaptive:
      D_print(F("WIF: timeout Captive portal"));
      stopCaptivePortal();
      break;
  }


  // Check mode from WiFi module
  // Check if mode changed
  uint8_t WiFiMode = WiFi.getMode();

  // TODO: in futur need to check carefully this with sleepmode
  static uint8_t previousWiFiMode = 0x80;


  if (  WiFiMode != previousWiFiMode) {
    // grab WiFi actual mode
    D1_print(F("WIF: --->Wifi mode change to "));
    D1_println(WiFiMode);
    D_println(F("WIF: Read wifi current mode and config "));
    D_print(F("WIF: SoftAP SSID "));
    D_println(WiFi.softAPSSID());
    D_print(F("WIF: SoftAP IP "));
    D_println(WiFi.softAPIP());

    D_print(F("WIF: Station SSID "));
    D_println(WiFi.SSID());
    //    WiFi.printDiag();

    //    D_print(F("SW: Station password "));
    //    D_println(WiFi.psk());


    // ON will be send before any  Station on or AP on
    if ( (previousWiFiMode & WIFI_AP_STA) == WIFI_OFF) {
      EventManagerPtr->pushEvent(evWEBWiFiOn);
      //      WiFi.forceSleepWake();
      //      delay(1);
    }

    if ( (WiFiMode & WIFI_STA)  && !(previousWiFiMode & WIFI_STA)  ) {
      EventManagerPtr->pushEvent(evWEBStationOn);
    }

    if ( (WiFiMode & WIFI_AP)  && !(previousWiFiMode & WIFI_AP)  ) {
      EventManagerPtr->pushEvent(evWEBAccessPointOn);
      captiveDNSStart();
    }

    if ( !(WiFiMode & WIFI_STA)  && (previousWiFiMode & WIFI_STA)  ) {
      EventManagerPtr->pushEvent(evWEBStationOff);
      captiveDNSStop();
    }


    if ( !(WiFiMode & WIFI_AP)  && (previousWiFiMode & WIFI_AP)  ) {
      EventManagerPtr->pushEvent(evWEBAccessPointOff);
    }


    // OFF will be send after any  Station off or AP off
    if ( (WiFiMode & WIFI_AP_STA) == WIFI_OFF) {
      EventManagerPtr->pushEvent(evWEBWiFiOff);
      WiFi.forceSleepBegin();
      delay(1);
    }

    //delay(5);    // to allow MSDNS and Captive to stop

    if (WiFiMode & WIFI_AP) {

      // seems useless !!!!  unless wifi goes powered off
      if (WiFi.softAPIP() != IPAddress(10, 10, 10, 10) ) {


        D_print(F("WIF: SoftAP IP "));
        D_println(WiFi.softAPIP());

        D1_println(F("WIF: need reconfig APIP 10.10.10.10   !!!!!!!!!!"));
        //D1_println(F("reset."));
        //delay(3000);
        //ESP.reset();
        IPAddress local_IP(10, 10, 10, 10);
        IPAddress mask(255, 255, 255, 0);
        bool result = WiFi.softAPConfig(local_IP, local_IP, mask);
        D_print(F("WIF: softapconfig = ")); D_println(result);
        D_print(F("WIF: SoftAP IP = "));
        D_println(WiFi.softAPIP());
        D_println(F("WIF: Captive start"));
      }



    }

    //  ETS_UART_INTR_DISABLE();
    //  WiFi.disconnect(); //  this alone is not enough to stop the autoconnecter
    //  ETS_UART_INTR_ENABLE();
    previousWiFiMode = WiFiMode;

    D_println(F("WIF: -- end Wifi mode change"));
  }




  static uint8_t previousWiFiStatus = 100;
  int WiFiStatus = WiFi.status();
  //    0 : WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses
  //    1 : WL_NO_SSID_AVAILin case configured SSID cannot be reached
  //    3 : WL_CONNECTED after successful connection is established
  //    4 : WL_CONNECT_FAILED if password is incorrect
  //    6 : WL_DISCONNECTED if module is not configured in station mode
  if (previousWiFiStatus != WiFiStatus) {
    TryStatus = WiFiStatus;  // chaine pour reporting

    D1_print(F("WIF: Status : "));
    D1_println(WiFiStatus);

    if ( (WiFiStatus == WL_CONNECTED) && (WiFiMode & WIFI_STA) ) {


      IPAddress WifiIP = WiFi.localIP();
      TWS::localIp = WifiIP.toString();  // recuperation de l'ip locale
      bool result = MDNS.begin(_deviceName, WifiIP);
      D_print(F("STA: MS DNS ON:"));
      D_print(_deviceName);
      D_print(F(" IP:"));
      D_print(TWS::localIp);
      D_print(F(" r="));
      D_println(result);

      if (trySetupPtr) {
        jobWEBTrySetupValidate();
      }

      EventManagerPtr->pushEvent(evWEBStationConnectedWiFi);

    }

    if ( (WiFiStatus == WL_DISCONNECTED) && (WiFiMode & WIFI_STA) ) {

      MDNS.end();  // only if connected
      EventManagerPtr->pushEvent(evWEBStationDisconnectedWiFi);
    }

    if (trySetupPtr && WiFiStatus == WL_CONNECT_FAILED ) jobWEBTrySetupAbort();
    previousWiFiStatus = WiFiStatus;

  }
  Server.handleClient();
  if (captiveDNS) dnsServer.processNextRequest();
  if (MDNS.isRunning()) MDNS.update();
}

String AppWebServer::getWebName() {
  return TWConfig.webName;
}


void AppWebServer::setCallBack_OnTranslateKey(void (*ontranslatekey)(String & key))  {
  onTranslateKeyPtr =  ontranslatekey;
}

void AppWebServer::setCallBack_OnStartRequest(void (*onstartrequest)(const String & filename, const String & submitValue))  {
  onStartRequestPtr =  onstartrequest;
}


void AppWebServer::setCallBack_OnRefreshItem(bool (*onrefreshitem)(const String & keyname, String & key)) {
  onRefreshItemPtr = onrefreshitem;
}


void AppWebServer::setCallBack_OnRepeatLine(bool (*onrepeatline)(const String &keyname, const int num)) {     // call back pour gerer les Repeat
  onRepeatLinePtr = onrepeatline;
}

bool AppWebServer::razConfig() {                             // efface la config enregistree
  return (TWConfig.erase());
}

String AppWebServer::createRandom() {
  _random = random(1000000, 9999999);
  D_print(F("WEB: New random "));
  D_println(_random);
  return (_random);
}

// TODO: pass all this stuff with persitent(false) as default !!!!!
// TODO: give result back
void   AppWebServer::startCaptivePortal(const int timeoutInSeconds) {
  _portalTimeoutInSeconds = timeoutInSeconds;
  EventManagerPtr->pushDelayEvent(_portalTimeoutInSeconds * 1000, evWEBTimerEndOfCaptive);
  WiFi.persistent(false);  // to go back on initial mode in case of reset
  WiFi.enableAP(true);   // wifi est non persistant
  WiFi.persistent(true);
  _captivePortalActive = true;
  D1_println(F("WEB: Active captivePortal"));
}

void   AppWebServer::stopCaptivePortal() {
  EventManagerPtr->removeDelayEvent(evWEBTimerEndOfCaptive);
  captiveDNSStop();  // just in case
  WiFi.persistent(true);
  WiFi.enableAP(false);   // wifi est non persistant
  _captivePortalActive = false;
  D1_println(F("WEB: stop captivePortal"));
}

void AppWebServer::printEvWEB(const uint8_t eventcode)  {
  switch (eventcode) {
    case evWEBTrySetup:
      Serial.println(F("Event: evWEBTrySetup"));
      break;
    case evWEBTimerEndOfTrySetup:
      Serial.println(F("Event: evWEBTimerEndOfTrySetup"));
      break;
    case evWEBTimerEndOfCaptive:
      Serial.println(F("Event: evWEBTimerEndOfCaptive"));
      break;
    case evWEBWiFiOn:
      Serial.println(F("Event: evWEBWiFiOn"));
      break;
    case evWEBWiFiOff:
      Serial.println(F("Event: evWEBWiFiOff"));
      break;
    case evWEBStationOn:
      Serial.println(F("Event: evWEBStationOn"));
      break;
    case evWEBStationOff:
      Serial.println(F("Event: evWEBStationOff"));
      break;
    case evWEBAccessPointOn:
      Serial.println(F("Event: evWEBAccessPointOn"));
      break;
    case evWEBAccessPointOff:
      Serial.println(F("Event: evWEBAccessPointOff"));
      break;
    case evWEBStationConnectedWiFi:
      Serial.println(F("Event: evWEBStationConnectedWiFi"));
      break;
    case evWEBStationDisconnectedWiFi:
      Serial.println(F("Event: evWEBStationDisonnectedWiFi"));
      break;
    case evWEBdevicenameChanged:
      Serial.println(F("Event: evWEBdevicenameChanged"));
      break;
  }
}
