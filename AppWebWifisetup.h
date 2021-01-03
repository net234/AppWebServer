/*************************************************
 *************************************************
    AppWebServer.h   part of of lib AppWebServer - Arduino lib for easy wifi interface with http and css
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
   V1.0.2  Stand alone captive portal C) 07/2020 P.HENRY NET234
   V1.0.3  1/12/2020  rewriting


  problems
   dhcp dont work on AP with other ip than 192.164.4.1   (fixed on 12/2020)
   unable to save in wifi flash config another AP ip than 192.164.4.1  (normal : ip not saved in flash  fixed by saving in .conf)
   mdns doesnt respond on AP   (fixed 06/12/2020)


**********/



// Scan les reseaux wifi le fill up des page comportant un arg appweb=show_wifi
#define MAXNETWORK 30
//const uint8_t MAXNETWORK = 30;
#define DELAY_TRYWIFISETUP   30


int network[MAXNETWORK + 1];
int networkSize = 0;
int currentLine = 0;

// call back pour sort network
int compareNetwork (const void * a, const void * b) {
  return ( WiFi.RSSI(*(int*)b) - WiFi.RSSI(*(int*)a) );  // this is why I dont like C
}

// Fill up network array with current network
// !! will open station mode even if it was closed !!
// TODO ?  close STA mode if it was open here ?
//WiFiMode_t wifiModeBeforeScan;
void scanNetwork() {
  //wifiModeBeforeScan = WiFi.getMode();
  if (!(WiFi.getMode() & WIFI_STA))  WiFi.persistent(false);  // TODO close sta
  networkSize = WiFi.scanNetworks();  // WiFi.enableSTA() is called !!
  if (networkSize > MAXNETWORK) networkSize = MAXNETWORK;
  //Serial.print(F("scanNetworks done"));
  if (networkSize == 0) return;

  //sort networks
  for (int i = 0; i < networkSize; i++)  network[i] = i;
  qsort(network, networkSize, sizeof(network[0]), compareNetwork);
  // remove duplicates SSID
  for (int i = 0; i < networkSize - 1; i++) {
    for (int j = i + 1; j < networkSize; j++) {
      if ( WiFi.SSID(network[i]) == WiFi.SSID(network[j]) )  {
        networkSize--;
        for (int k = j; k < networkSize; k++) network[k] = network[k + 1];
        j--;
      }
    }
  }
}

// reset wifi mode as it was before scan network
void closeScanNetwork() {
  WiFi.scanDelete();  // free some memory
  D_print(F("Scan network END Freemem = "));
  D_println(ESP.getFreeHeap());
  //  if (!WiFi.getPersistent()) {
  //    WiFi.persistent(true);
  //    WiFi.mode(wifiModeBeforeScan);
  //  }
}

#define RSSIdbToPercent(x) ( 2 * (WiFi.RSSI(x) + 100) )

//gestion du repeat line specifique "APPWEB_SCANSETWORK"
bool repeatLineScanNetwork(const int num) {
  if ( !Server.arg(F("submit")).equals(F("scanWiFi")) ) return (false);
  currentLine = num;
  D_print(F("Scan network Freemem = "));
  D_println(ESP.getFreeHeap());
  if (currentLine == 0) scanNetwork();
  bool result = (num < networkSize && RSSIdbToPercent(network[num]) > 25 );
  if (!result) closeScanNetwork();
  return (result);
}


////////////////// TrySSetup ///////////////////////////////
///////// gestion du wifisetup/configure.html //////////////
//  index.html         choix des pages de configuration  (auto refresh 30 seconde vers /index.html)
//
//  configure.html     soumet un formulaire de configuration  (auto refresh 30 seconde vers index.html)
//        si le formulaire est ok on redirect vers message.html
//  message.html    affiche un message d'attente (auto refresh 5 secondes)
//        quand le trySetup est fini on redirect vers resultat.html
//  resultat.html    affiche le resultat (auto refresh 30 seconde vers index.html)
//
//

struct trySetup_t {
  String SSID;
  String PASS;
  String oldSSID;
  String oldPASS;  // to put back in current config if fail
  String deviceName;  // to put back in current config if fail
  bool isTrying = false;
};

trySetup_t* trySetupPtr = nullptr; // a pointer to tryconf

/////////////============submit appweb_wifisetup (page configure.html)==============////////////////////
// gestion du submit appweb_wifisetup
void do_appweb_wifisetup() {
  // check if form if valid
  String aSSID = Server.arg(F("STATION_SSID"));
  aSSID.trim();
  if (aSSID.length() <= 2 && aSSID.length() > 30) {
    return;
  }

  String aHostname = Server.arg(F("HOSTNAME"));
  aHostname.trim();
  if (aHostname.length() <= 2 && aHostname.length() > 30) {
    return;
  }
  // form is valid
  // we redirect to test.html page
  // we create a trySetup object that will be tryed on the end of display of the redirected page

  if (!trySetupPtr)  {
    D_println(F("New tryseup !!!"));
    trySetupPtr = new trySetup_t;
  }
  trySetupPtr->oldSSID = WiFi.SSID();
  trySetupPtr->oldPASS = WiFi.psk();
  trySetupPtr->deviceName = aHostname;
  trySetupPtr->SSID = aSSID;
  trySetupPtr->PASS = Server.arg(F("PASS"));
  trySetupPtr->PASS.trim();
  trySetupPtr->isTrying = false;
  //TODO: make a redirectTo(uri) function
  AppWebPtr->ACTION_redirect = "";
  AppWebPtr->ACTION_TITLE = F("WiFiSetup");
  AppWebPtr->ACTION_NAME =  F("Connection au WiFi ");
  AppWebPtr->ACTION_NAME += aSSID;
  AppWebPtr->ACTION_TEXT =  F("...");
  TWS::redirectUri = F("message.html");
  // delay 100ms the trySetup to allow redirect to be done
  EventManagerPtr->pushDelayEvent(100, evWEBTrySetup);
}

// gestion du submit appweb_reset
void do_appweb_reset() {
  AppWebPtr->ACTION_redirect = "";
  AppWebPtr->ACTION_TITLE = F("Reset");
  AppWebPtr->ACTION_NAME =  F("Reset");
  AppWebPtr->ACTION_TEXT =  "";
  TWS::redirectUri = F("message.html");
  // delay 100ms the reset to allow redirect to be done
  EventManagerPtr->pushDelayEvent(100, evWEBDoReset);
}
////////////////// gestion de wifisetup/test.html //////////



/////////////////// specific job for WiFiSetup
// called on an evWEBTrySetup
void jobWEBTrySetup() {
  D_println(F("tryConfigWifisetup"));
  if (!trySetupPtr || trySetupPtr->isTrying) return;
  D_println(F("---> start try"));
  trySetupPtr->isTrying = true;
  // if no wifi configured : WiFi.SSID() == "" we test this wifi
  // if (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
  //       //trying to fix connection in progress hanging
  EventManagerPtr->pushDelayEvent(DELAY_TRYWIFISETUP * 1000, evWEBTimerEndOfTrySetup); // should be done in 30 sec
  WiFi.persistent(false);
  //  ETS_UART_INTR_DISABLE();
  //  wifi_station_disconnect();
  //  ETS_UART_INTR_ENABLE();
  WiFi.enableSTA(false); // to force a save of config in WiFi module
  D_print(F("WIFI begin result="));
  //WiFi.begin(ssid, password, channel, bssid, connect)
  bool result = WiFi.begin(trySetupPtr->SSID, trySetupPtr->PASS); //,0,NULL,true);
  D_print(result);
  //WiFi.persistent(true);
}


void jobWEBTrySetupValidate() {

  if (trySetupPtr && trySetupPtr->isTrying ) {
    EventManagerPtr->removeDelayEvent(evWEBTimerEndOfTrySetup);
    D1_println(F("WF: Moving to STATION"));
    WiFi.enableSTA(false);
    WiFi.persistent(true);
    //WiFi.enableAP(false);    // AP will be stop with evWEBTimerEndOfCaptive
    // stop captive in 2 sec if needed
    if (AppWebPtr->_captivePortalActive) EventManagerPtr->pushDelayEvent(2000, evWEBTimerEndOfCaptive);

    WiFi.begin(trySetupPtr->SSID, trySetupPtr->PASS);  // save STATION setup in flash



    if ( trySetupPtr->deviceName != AppWebPtr->_deviceName) {
      D_print(F("SW: need to init WiFi same as new config   !!!!! "));
      D_print(trySetupPtr->deviceName);
      D_print(F("!="));
      D_println(AppWebPtr->_deviceName);
      AppWebPtr->setDeviceName(trySetupPtr->deviceName);  //check devicename validity
      WiFi.softAP(AppWebPtr->_deviceName);   // save in flash
      if (TWConfig.deviceName != WiFi.softAPSSID()) {
        D_print(F("SW: need to need to rewrite flah config   !!!!! "));
        D_print(TWConfig.deviceName);
        D_print(F("!="));
        D_println(WiFi.softAPSSID());
        TWConfig.deviceName = WiFi.softAPSSID();  //put back devicename in config if needed
        TWConfig.changed = true;
        TWConfig.save();
      }

    }

    AppWebPtr->ACTION_redirect = F("resultat.html");
    AppWebPtr->ACTION_TITLE = F("WiFiSetup");
    AppWebPtr->ACTION_NAME =  F("Connection au WiFi ");
    AppWebPtr->ACTION_NAME += trySetupPtr->SSID;
    AppWebPtr->ACTION_TEXT =  F("Liaison effectuée");

    delete trySetupPtr;
    trySetupPtr = nullptr;

  }
}


void jobWEBTrySetupAbort() {
  if (trySetupPtr) {
    // bad password or time out
    D1_println(F("TrySetup Abort"));
    D_println(F("Remove try config"));
    //set back old credential if any
    if (trySetupPtr->SSID.length() > 0) {
      WiFi.enableSTA(false);
      WiFi.begin(trySetupPtr->oldSSID, trySetupPtr->oldPASS);  // put back STA old credential if any
    }
    WiFi.persistent(true);


    AppWebPtr->ACTION_redirect = F("resultat.html");
    AppWebPtr->ACTION_TITLE = F("WiFiSetup");
    AppWebPtr->ACTION_NAME =  F("Connection au WiFi ");
    AppWebPtr->ACTION_NAME += trySetupPtr->SSID;
    AppWebPtr->ACTION_TEXT =  F("Liaison impossible ");
    if (trySetupPtr->SSID.length() > 0) {
      AppWebPtr->ACTION_TEXT += "WiFi ";
      AppWebPtr->ACTION_TEXT += trySetupPtr->oldSSID;
      AppWebPtr->ACTION_TEXT += " reconfiguré";
    }
    delete trySetupPtr;
    trySetupPtr = nullptr;
  }

}
