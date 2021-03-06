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
   V1.0.2  Stand alone captive portal
   Version B02  01/2020  Ajout des events  (BetaEvents.lib)
   Version B03  13/01/2021   
     Add csv   they are parsed as html
     add a trim on refesh send back
     add redirectRequestTo(aUri)  method
     error 404 set in D1 error
     add  getRepeatLineNumber() methode
   Version B04   30/01/2021
     add JSON fileType
     add server arg(s) methode
     add getRemoteIp()  ---> Server.client().remoteIP()

   TODO:  refresh stay a 1000 (after auto refresh from wifisetup)
   TODO: mode AP permanent with no capture
   TODO: better use of ACTION/Message Page
   TODO: better deal with new random / random
   TODO: change getRepeatLineNumber() with first line starting to 1 instead of 0
   TODO: provide a way to answer a String as payload of request (faster for json)
**********/

#pragma once
#include <arduino.h>
//#include "ESP8266.h"
#include <ESP8266WiFi.h>

//#define DEBUG_ON

//#ifndef EventManagerPtr
//#error BetaEvents.h is missing in yous sketch !!!!
//#endif


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

// Liste des evenements specifique WEB
// TODO: check at compile time if evAwsTrySetup = evWEB
enum EventCodeWEB_t:uint8_t {
  // evenement recu
  evWEBTrySetup = 50,           // A new config setup need to tryed
  evWEBTimerEndOfTrySetup,      // Time out for the try
  evWEBTimerEndOfCaptive,       // Timer to stop captive
  evWEBDoReset,                 // WEB user requested a Reset (from WiFiSetup)
  evWEBWiFiOn,                  // WiFi turned to ON
  evWEBWiFiOff,                 // WiFi turned to OFF
  evWEBStationOn,               // Station turned to ON
  evWEBStationOff,              // Station turned to OFF
  evWEBAccessPointOn,           // AccessPoint turned to ON
  evWEBAccessPointOff,          // AccessPoint turned to OFF
  evWEBStationConnectedWiFi,    // Station Connected with Local WiFi
  evWEBStationDisconnectedWiFi, // Station Disconnected from Local WiFi
  evWEBdevicenameChanged,       // User recorded succesfully a new device name
};




#define SERVER_PORT 80                    // default port for http
// DNS server
#define DNS_PORT  53

//#define SERVER_APSETUPTIMEOUT (3 * 60 )       // Timeout to release mode WIFI_APSETUP
#define DEFAULT_DEVICENAME "APPWEB_"

//typedef enum WiFiMode
//{
//    WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3,
//    /* these two pseudo modes are experimental: */ WIFI_SHUTDOWN = 4, WIFI_RESUME = 8
//} WiFiMode_t;



// Main Object limited to one instance
class AppWebServer {
  public:
    AppWebServer();                               // constructor mono instance grab current wifi configuration
    ~AppWebServer();                              // destructor mono instance
    void begin(const String devicename = "", const int debuglevel = 0 );                                  // main server start as was configured
    void end();                                     // main server stop
    bool razConfig();                               // efface la config enregistree
    String getWebName();                            // return webName from the web config file (appWebServer.ini)
    //    TW_WiFiMode_t    getWiFiMode();  // Wifi Mode expected by user
    //    void             setWiFiMode(WiFiMode_t const mode, const char* ssid = NULL , const char* ssidpassword = NULL);
    //    void             setWiFiMode(WiFiMode_t const mode);

    //    TW_WiFiStatus_t  getWiFiStatus();                  // Wifi Status is scanned during handleClient()
    void setDeviceName(const String devicename);
    String createRandom();                                   // create a new random string in _random
    void   startCaptivePortal(const int timeoutInSeconds = 60);                      // create a temporary portal
    void   stopCaptivePortal();
    //    void configureWiFi(const bool active = false);  // active the AP mode to request wifi credential from the user
    // //   void softAPconnect(const bool active,const bool persistent = false,const char* = NULL);
    //
    void handleEvent();     // handle http service (to call in top of loop)
	void redirectRequestTo(const String aURI);  //  redirect a request with a redirect error 302
    void setCallBack_OnStartRequest(void (*onstartrequest)(const String & filename, const String & submitValue));          // call back pour gerer les request
    void setCallBack_OnTranslateKey(void (*onTranslateKey)(String &key));  // call back pour Fournir les [# xxxxx #]
    void setCallBack_OnRefreshItem(bool (*onRefreshItem)(const String &keyname, String &key));  // call back pour fournir les class='refresh'
    void setCallBack_OnRepeatLine(bool (*onRepeatLine)(const String &repeatname, const uint16_t lineNumber));     // call back pour gerer les Repeat
	uint16_t getRepeatLineNumber();
    void printEvWEB(const uint8_t eventcode);
	uint16_t getArgs();
	String getArg(const String argname);
	String getArg(const uint16_t argnum);
	String getArgName(const uint16_t argnum);
	IPAddress getRemoteIP();
    //    String currentUri();                            // return the last requested URI (actual page in calllback)
    //    // var
    // PRESET ACTION KEY
    String PAGENAME;      // [#PAGENAME appweb_message#] (present on request if in first line of .html page)
    String ACTION_redirect = "/index.html";
    String ACTION_TITLE;  // default [#ACTION_TITLE#] key replacement
    String ACTION_NAME;   // default [#ACTION_NAME#] key replacement
    String ACTION_TEXT;   // default [#ACTION_TEXT#] key replacement

    // TODO: a passer en private
    String  _deviceName;             // AP Name  amd  mDns Name in station mode
    String  _defaultWebFolder;       // Web base Path for non captive move
    String  _captiveWebFolder;       // Web base Path for captive mode
    bool    _standAlone = false;     // device work only on AP mode (in progress)
    bool    _captiveAP = false;      // used to swith between standard mode
    String  _random;                 // random number changed on each request without submit  TODO: finish this
    // String  page_id;
    bool _captivePortalActive = false;
    int16_t _portalTimeoutInSeconds = 60;
    int16_t _error = 999;

  private:

    byte _debugLevel = 0;  // 0 = none / 1 = minimal / 2 = variable request / 3 = wifi debug (must be set before begin)
    //int16_t _timerCaptiveAP = 0;  // timer limitation du mode AP en seconde
    //    String  _redirectUri;   //  request will be redirected to this URI if set after onRequest call back


    //bool WiFiStatusChanged = false;   // Flag set when wifi_Status change
    //bool WiFiModeChanged = false;   // Flag set when wifi_Status change
    //String webFolder = "/web";
    //String  _hostname;      //  SSID en mode AP et Serveur name en mode STATION

    //WiFiMode_t    _WiFiMode = (WiFiMode_t)99;  // mode unknow
    //WiFiMode_t    _newWiFiMode = WIFI_OFF;
    //TW_WiFiStatus_t  _WiFiStatus = tws_WIFI_TRANSITION;

};
