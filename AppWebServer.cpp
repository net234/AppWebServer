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
    D1_print(F("Error: Only one instance for AppWebServer"));
    while (true) delay(100);
  }
  if (EventManagerPtr == nullptr) {
    D1_print(F("Error: Missing instance of BetaEvents object"));
    while (true) delay(100);
  }
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

  if ( TWConfig.bootForceAP > 0 && !(WiFi.getMode() & WIFI_AP) ) {
    D_println(F("TWS: Force mode Captive AP !!!"));
    WiFi.persistent(false);  // to go back on initial mode in case of reset
    WiFi.enableAP(true);   // wifi est non persistant
    WiFi.persistent(true);
    timerCaptivePortal = TWConfig.bootForceAP * 60;
  }

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
}



void AppWebServer::handleEvent() {
  // deal with event receved
  switch (EventManagerPtr->currentEvent.code)
  {
    case evNill: break;
    case evWEBTrySetup:              jobWEBTrySetup(); break;          // A new config setup need to try
    case evWEBTimerEndOfTrySetup:
      D1_print(F("WS: TrySetup Abort : timeout"));
      jobWEBTrySetupAbort();

      break;     // Abort try setup (time out)
  }

  // Check if mode changed
  WiFiMode_t wifimode = WiFi.getMode();
  if (  _WiFiMode != wifimode) {
    _WiFiMode = wifimode;
    // grab WiFi actual mode
    D1_print(F("WF: Wifi mode change to "));
    D1_println(_WiFiMode);
    D_println(F("SW: Read wifi current mode and config "));
    D_print(F("SW: SoftAP SSID "));
    D_println(WiFi.softAPSSID());
    D_print(F("SW: SoftAP IP "));
    D_println(WiFi.softAPIP());

    D_print(F("SW: Station SSID "));
    D_println(WiFi.SSID());

    //    D_print(F("SW: Station password "));
    //    D_println(WiFi.psk());

    D_print(F("SW: Station IP "));
    D_println(WiFi.localIP());
    //wifi_softap_dhcps_stop();
    captiveDNSStop();

    //delay(5);    // to allow MSDNS and Captive to stop

    if (_WiFiMode & WIFI_AP) {


      if (WiFi.softAPIP() != IPAddress(10, 10, 10, 10) ) {

        //    if (_WiFiMode == WIFI_AP) {
        //      WiFi.softAP(_deviceName);
        //    }
        //        delay(100);
        D1_println(F("WS: need reconfig APIP 10.10.10.10   !!!!!!!!!!"));
        D1_println(F("reset."));
        delay(3000);
        ESP.reset();
        //        IPAddress local_IP(10, 10, 10, 10);
        //        IPAddress mask(255, 255, 255, 0);
        //        bool result = WiFi.softAPConfig(local_IP, local_IP, mask);
        //        D_print(F("SW: softapconfig = ")); D_println(result);
        //        D_print(F("SW: SoftAP IP = "));
        //        D_println(WiFi.softAPIP());

      }

      D_println(F("SW: Captive start"));

      captiveDNSStart();

    }
    //  ETS_UART_INTR_DISABLE();
    //  WiFi.disconnect(); //  this alone is not enough to stop the autoconnecter
    //  ETS_UART_INTR_ENABLE();
    D_println(F("SW: -- end Wifi mode change"));
  }




  static int oldStatus = -1;
  int status = WiFi.status();
  //    0 : WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses
  //    1 : WL_NO_SSID_AVAILin case configured SSID cannot be reached
  //    3 : WL_CONNECTED after successful connection is established
  //    4 : WL_CONNECT_FAILED if password is incorrect
  //    6 : WL_DISCONNECTED if module is not configured in station mode
  if (status != oldStatus) {
    TryStatus = status;  // chaine pour reporting

    D1_print(F("WF: Status : "));
    D1_println(status);
    //    D_println(F("SW: Read wifi current mode and config "));
    //    D_print(F("SW: SoftAP SSID "));
    //    D_println(WiFi.softAPSSID());
    //    D_print(F("SW: SoftAP IP "));
    //    D_println(WiFi.softAPIP());
    //
    //    D_print(F("SW: Station SSID "));
    //    D_println(WiFi.SSID());
    //
    //    D_print(F("SW: Station password "));
    //    D_println(WiFi.psk());
    //
    //    D_print(F("SW: Station IP "));
    //    D_println(WiFi.localIP());

    oldStatus = status;
    if (status == WL_CONNECTED) {


      //MDNS.addService("http", "tcp", 80);



      TWS::localIp = WiFi.localIP().toString();  // recuperation de l'ip locale
      if (trySetupPtr) jobWEBTrySetupValidate();
      bool result = MDNS.begin(_deviceName);
      D_print(F("STA: MS DNS ON : "));
      D_print(_deviceName);
      D_print(F(" r="));
      D_println(result);

    } else {
      MDNS.end();  // only if connected
    }

    if (trySetupPtr && status == WL_CONNECT_FAILED ) jobWEBTrySetupAbort();

  }


  MDNS.update();
  Server.handleClient();
  handleCaptivePortal();


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
//
//
//void AppWebServer::setCallBack_OnRepeatLine(bool (*onrepeatline)(const int num)) {     // call back pour gerer les Repeat
//  onRepeatLinePtr = onrepeatline;
//}

bool AppWebServer::razConfig() {                             // efface la config enregistree
  return (TWConfig.erase());
}

String AppWebServer::createRandom() {
  _random = random(1000000, 9999999);
  return (_random);
}
