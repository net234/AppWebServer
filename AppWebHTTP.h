/*************************************************
 *************************************************
    AppWebHTTP.cpp  is part of of lib AppWebServer - Arduino lib for easy wifi interface with http and css
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

    Gestion du http et des callback http
*/

#include "AppWebWifisetup.h"
//pointer du traducteur de Key
void (*onTranslateKeyPtr)(String &key) = NULL;

//TODO: skip all this for lowercase first letter
//TODO; change RANDOM with a PAGEID stuff
void translateKey(String &key) {
  if ( key.equals(F("HOSTNAME")) ) {
    key =  TWConfig.deviceName;
  } else if ( key.equals(F("ACTION_TITLE")) ) {
    key = AppWebPtr->ACTION_TITLE;
  } else if ( key.equals(F("ACTION_NAME")) ) {
    key = AppWebPtr->ACTION_NAME;
  } else if ( key.equals(F("ACTION_TEXT")) ) {
    key = AppWebPtr->ACTION_TEXT;
  } else if ( key.equals(F("PAGENAME")) ) {
    key = AppWebPtr->PAGENAME;
  } else if ( key.equals(F("CHIP_ID")) ) {
    key = ESP.getChipId();
  } else if ( key.equals(F("NEW_RANDOM")) ) {
    key = AppWebPtr->createRandom();
  } else if ( key.equals(F("RANDOM")) ) {
    key = AppWebPtr->_random;
  } else if ( key.equals(F("FLASH_CHIP_ID")) ) {
    key = ESP.getFlashChipId();
  } else if ( key.equals(F("IDE_FLASH_SIZE")) ) {
    key = ESP.getFlashChipSize();
  } else if ( key.equals(F("REAL_FLASH_SIZE")) ) {
    key = ESP.getFlashChipRealSize();
  } else if ( key.equals(F("SOFTAP_IP")) ) {   // TODO: show actual or programed IP ? may be add OFF or ON behind IP
    key = WiFi.softAPIP().toString();
  } else if ( key.equals(F("SOFTAP_SSID")) ) {
    key = WiFi.softAPSSID();
  } else if ( key.equals(F("SOFTAP_MAC")) ) {
    key = WiFi.softAPmacAddress();
  } else if ( key.equals(F("STATION_IP")) ) {   // TODO: show actual or programed IP ? may be add OFF or ON behind IP
    key = TWS::localIp;
  } else if ( key.equals(F("STATION_SSID")) ) {
    key = WiFi.SSID();
  } else if ( key.equals(F("TRY_SSID")) ) {
    if (trySetupPtr) key = trySetupPtr->SSID;
  } else if ( key.equals(F("TRY_STATUS")) ) {
    key = TWS::TryStatus;
  } else if ( key.equals(F("STATION_MAC")) ) {
    key = WiFi.macAddress();

    //specific wifisetuo
  } else if ( key.equals(F("WIFISETUP_SSID_NAME")) ) {
    key = WiFi.SSID(network[currentLine]);
  } else if ( key.equals(F("WIFISETUP_SSID_LEVEL")) ) {
    int level = RSSIdbToPercent(network[currentLine]);
    if (level > 100) level = 100;
    key = level;
  } else if ( key.equals(F("WIFISETUP_SSID_LOCKED")) ) {
    key = "&nbsp;";
    // todo add [#DATA dataname datavalue#] keywork to avoid HTML specific in code like "&#128274;"
    if (WiFi.encryptionType(network[currentLine]) != ENC_TYPE_NONE) key = "&#128274;";  //htmlcode icone cadena
  } else if (onTranslateKeyPtr) {
    (*onTranslateKeyPtr)(key);
  }
}




// onRequest callback
//pointer du gestionaire de request
void (*onStartRequestPtr)(const String &filename, const String &submitvalue) = NULL;

void onStartRequest(const String &filename, const String &submitvalue) {

  // track "appweb_xxxxx_[#PAGEID#]" submit
  if (&submitvalue && submitvalue.startsWith(F("appweb_")) && submitvalue.endsWith(AppWebPtr->_random) ) {
    if ( submitvalue.startsWith(F("appweb_wifisetup_")) ) {
      do_appweb_wifisetup();
    } else if ( submitvalue.startsWith(F("appweb_reset_")) ) {
      do_appweb_reset();
    }
  }
  // track "appweb_message"    request with no message
  if ( AppWebPtr->PAGENAME.startsWith(F("appweb_message")) && AppWebPtr->ACTION_redirect.length() > 0 ) {
    if (AppWebPtr->ACTION_TITLE.length() == 0 ) AppWebPtr->ACTION_redirect = F("/index.html");
    redirectUri = AppWebPtr->ACTION_redirect;
    AppWebPtr->ACTION_redirect = "";
  }

  if (onStartRequestPtr) (*onStartRequestPtr)(filename, submitvalue);
}

// onEndOfRequest callback
//pointer du gestionaire de request
void (*onEndOfRequestPtr)(const String &filename, const String &submitvalue) = NULL;

void onEndOfRequest(const String &filename, const String &submitvalue) {

  if (onEndOfRequestPtr) (*onEndOfRequestPtr)(filename, submitvalue);
}



//pointeur du gestionanire de refresh
bool  (*onRefreshItemPtr)(const String &keyname, String &key) = NULL;

bool onRefreshItem(const String &keyname, String &key) {
  if (onRefreshItemPtr) return (*onRefreshItemPtr)(keyname, key);
  return (false);
}

//pointeur du gestionaire de repeatLine
bool (*onRepeatLinePtr)(const String &repeatname, const int num) = NULL;
// appelé avant chaque ligne precédée d'un [#REPEAT_LINE xxxxxx#]  repeatname = xxxxxx
bool onRepeatLine(const String &repeatname, const int num) {
  D_print(F("WEB: got a repeat ")); D_print(repeatname);
  D_print(F(" num=")); D_println(num);
  if ( repeatname.equals(F("WIFISETUP_LIST")) ) return repeatLineScanNetwork(num);
  if (onRepeatLinePtr) return (*onRepeatLinePtr)(repeatname, num);
  D_println(F("WEB: repeat not catched "));
  return (false);
}




//// Dealing with WEB HTTP request
//#define LOCHex2Char(X) (X + (X <= 9 ? '0' : ('A' - 10)))
char LOCHex2Char( byte aByte) {
  return aByte + (aByte <= 9 ? '0' : 'A' -  10);
}
//// looking for requested file into local web pages
void HTTP_HandleRequests() {

  D_print(F("WEB receved a "));
  D_print( String((Server.method() == HTTP_GET) ? "GET '" : "POST '" ));
  D_print(Server.uri());
  D_print(F("' from "));
  D_print(Server.client().remoteIP());
  D_print(':');
  D_print(Server.client().remotePort());
  D_print(F(" <= "));
  D_print(Server.client().localIP());
  D_print(':');
  D_print(Server.client().localPort());
  D_println();

  // find if client call STATION or AP
  String filePath = AppWebPtr->_defaultWebFolder; //default /web;
  if ( Server.client().localIP() == WiFi.localIP() ) {
    // specific for station nothing special to do
    D_println(F("WEB: answer as STATION"));

  } else if ( Server.client().localIP() != WiFi.softAPIP() ) {
    // specific for unknow client -> abort request
    D1_println(F("WEB: unknown client IP !!!!"));
    return;
  } else {
    // specific for AP client -> check specific config for filename and handle Captive mode
    D_println(F("WEB: answer as AP"));
    if (AppWebPtr->_captiveAP ) {
      filePath = AppWebPtr->_captiveWebFolder; //default /web/wifisetup

      // in captive mode all requests to html or txt are re routed to "http://localip()" with a 302 reply
      if ( !( Server.hostHeader().startsWith( WiFi.softAPIP().toString() ) )  && Server.uri().endsWith(".html") ||  Server.uri().endsWith(".txt") || Server.uri().endsWith("redirect") ) {
        D_println(F("WEB: Request redirected to captive portal"));
        String aStr = F("http://");
        aStr += Server.client().localIP().toString();
        aStr += F("/index.html");
        Server.sendHeader("Location", aStr, true);
        Server.send ( 302, "text/plain", "");
        Server.client().stop();
        D_println(F("WEB: --- GET closed with a 302"));
        return;
      } // if captive page
      //
      // Gestion des Health Check
      if (Server.uri().endsWith("generate_204") ) {
        D_println(F("Generate204"));
        Server.setContentLength(0);
        Server.send ( 204 );
        Server.client().stop();
        D_println(F("WEB: --- GET closed with a 204"));
        return;
      }
      //
      //  // rearm timeout for captive portal
      //  // to hide captive mode stop DNS captive if a request is good (hostheader=localip)
      //  if (softAP) {
      //    timerCaptivePortal = millis();
      //    if (captiveDNS) captiveDNSStop();
      //  }
      //
      //
    }
  }

  if ( filePath.length() == 0 ) filePath = '/';
  filePath += Server.uri();
  // todo   protection against ../

  if (filePath.endsWith(F("/")) ) filePath += "index.html";   //default page ;

  String fileMIME;

  // find MIMETYPE for standard files
  enum fileType_t { NONE, CSS, ICO, PNG, JPG, GIF, JS, HTML } fileType = NONE;
  if ( filePath.endsWith(F(".css")) ) {
    fileType = CSS;
    fileMIME = F("text/css");
  } else if ( filePath.endsWith(F(".ico")) ) {
    fileType = ICO;
    fileMIME = F("image/x-icon");
  } else if ( filePath.endsWith(F(".png")) ) {
    fileType = PNG;
    fileMIME = F("image/png");
  } else if ( filePath.endsWith(F(".jpg")) ) {
    fileType = JPG;
    fileMIME = F("image/jpg");
  } else if ( filePath.endsWith(F(".gif")) ) {
    fileType = GIF;
    fileMIME = F("image/gif");
  } else if ( filePath.endsWith(F(".js")) ) {
    fileType = JS;
    fileMIME = F("application/javascript");
  } else if ( filePath.endsWith(F(".html")) ) {
    fileType = HTML;
    fileMIME = F("text/html");
  }

  // On each http request a callback to onRequest is call to inform sketch of any http request with the submit name if submit arg exist
  // if redirectTo(aUri) is set then an error 302 will be sent to redirect request



  TWS::redirectUri = "";
  String submitValue;
  if ( Server.hasArg(F("submit")) ) {
    submitValue = Server.arg(F("submit"));
    D_print(F("WEB: Submit action '"));
    D_print(submitValue);
    D_println("'");
  }

  // this buffer is user for file transismission and is locked in static ram to avoid heap fragmentation
  static    char staticBufferLine[1025];               // static are global so dont overload stack

  // detection of [#PAGENAME appweb_message#] on first line of html pages
  AppWebPtr->PAGENAME = "";
  // default page name is the filename itself
  int aPos = filePath.lastIndexOf("/");
  if (aPos >= 0) AppWebPtr->PAGENAME = filePath.substring(aPos + 1);
  if (fileType == HTML) {
    // try to grab keyword in first line of the file
    File aFile = LittleFS.open(filePath, "r");
    if (aFile) {
      aFile.setTimeout(0);   // to avoid delay at EOF
      // read first line  TODO: cache the keyword to avoid rereading file on each refresh ????
      int size = aFile.readBytesUntil( '\n', staticBufferLine, 1000 );
      staticBufferLine[size] = 0x00;
      aFile.close();
      if (char* startPtr = strstr(staticBufferLine, "[#PAGENAME ") ) {  // start detected
        startPtr += 11;
        char* stopPtr = strstr( startPtr + 1, "#]" ); // at least 1 letter keyword [#  #]
        int len = stopPtr - startPtr;
        if (  stopPtr && len > 0 && len <= 50 ) { // abort if no stop or lenth of keyword over 50
          stopPtr[0] = 0x00;  // end of keyword
          AppWebPtr->PAGENAME = startPtr;
          AppWebPtr->PAGENAME.trim();
        }
      }
    }
    if (AppWebPtr->PAGENAME.length() > 0) {
      D_print(F("WEB: pagename is "));
      D_println(AppWebPtr->PAGENAME);
    }
  }

  onStartRequest(filePath, submitValue);

  if (TWS::redirectUri.length() > 0) {
    D_print(F("WEB redirect "));
    D_println(TWS::redirectUri);
    Server.sendHeader("Location", TWS::redirectUri, true);
    Server.send ( 302, "text/plain", "");
    Server.client().stop();
    return;
  }




  //  // ================ SPECIFIC FOR "QUERY REFRESH"  START =======================
  //
  // query 'refresh' is a POST sended by 'appwebserver.js' to update class 'refresh' web items in real time
  // if first arg is named 'refresh'  all itmes are translated with  onRefreshItem  call back
  // then they are sended back to client in a urlencoded string
  if (Server.method() == HTTP_POST && Server.args() > 0 && Server.argName(0) == "refresh") {
    // debug track
    D_print(F("WEB: Query refresh ("));
    D_print(Server.arg(Server.args() - 1).length());
    D_print(F(") "));
    D_println(Server.arg(Server.args() - 1)); // try to get 'plain'
    // traitement des chaines par le Sketch
    String answer;
    answer.reserve(1000);   // should be max answer (all answers)

    for (uint8_t N = 0; N < Server.args(); N++) {
      //     Serial.print(F("WEB: refresh "));
      //     Serial.print(Serveur.argName(N));
      //     Serial.print(F("="));
      //      Serial.println(Serveur.arg(N));
      String aKeyName = Server.argName(N);
      String aKey = Server.arg(N);
      if ( !aKeyName.equals(F("plain")) && onRefreshItem(aKeyName, aKey) ) {
        if (answer.length() > 0) answer += '&';
        answer +=  aKeyName;
        answer +=  '=';
        if ( aKey.length() > 500) {
          aKey.remove(490);
          aKey += "...";
        }
        // uri encode maison :)
        for (int N = 0; N < aKey.length(); N++) {
          char aChar = aKey[N];
          //TODO:  should I keep " " to "+" conversion ????  save 2 char but oldy
          if (aChar == ' ') {
            answer += '+';
          } else if ( isAlphaNumeric(aChar) ) {
            answer +=  aChar;
          } else if (String(".-~_").indexOf(aChar) >= 0) {
            answer +=  aChar;
          } else {
            answer +=  '%';
            answer += LOCHex2Char( aChar >> 4 );
            answer += LOCHex2Char( aChar & 0xF);
          } // if alpha
        } // for
      } // valide keyname
    } // for each arg

    D_print(F("WEB: refresh answer ("));
    D_print(answer.length());
    D_print(F(") "));
    D_println(answer);
    //
    Server.sendHeader("Cache-Control", "no-cache");
    Server.setContentLength(answer.length());
    Server.send(200, fileMIME.c_str(), answer);
    //    Serveur.client().stop();



    return;
  }
  //  // ================ QUERY REFRESH  END =======================
  //
  //
  //  ================= Deal with local web pages ============
  File aFile;
  if (fileType != NONE) aFile = LittleFS.open(filePath, "r");
  bool doChunk = false;
  if (aFile) {
    D_print(F("WEB: Answer with file "));
    D_println(filePath);
    if (fileType == HTML) {
      Server.sendHeader("Cache-Control", "no-cache");
      Server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      doChunk = true;
    } else {
      Server.sendHeader("Cache-Control", "max-age=10800, public");
      Server.setContentLength(aFile.size());
    }
    Server.send(200, fileMIME.c_str());
    aFile.setTimeout(0);   // to avoid delay at EOF

    String repeatString;
    int  repeatNumber;
    bool repeatActive = false;
    String aStrKey;  // la clef du repeat
    // repeat until end of file
    while (aFile.available()) {
      int size;
      if (!doChunk) {

        // standard file (not HTML) are with 1024 byte buffer
        size = aFile.readBytes( staticBufferLine, 1024 );
      } else {

        // chunked file are read line by line with spefic keyword detection
        // first keyword is [# REPEAT_LINE xxxxxx#]
        if (!repeatActive) {  // if repeat we will send the repeat line
          // if not in repeat line mode   just read one line
          size = aFile.readBytesUntil( '\n', staticBufferLine, 1000 );
          if (size < 1000) staticBufferLine[size++] = '\n'; //!!! on exactly 1000 bytes lines the '\n' will be lost :)
          staticBufferLine[size] = 0x00; // make staticBufferLine a Cstring
          // Gestion du [# REPEAT_LINE xxxxxx#]
          // if a line start with  [#REPEAT_LINE *#] it will be sended until OnRepat Call bask returne false
          // this help to display records of database ot any table (see wifisetup.html)

          if (strncmp( "[#REPEAT_LINE", staticBufferLine, 13) == 0) {
            char* startPtr = staticBufferLine + 14;
            char* stopPtr = strstr( startPtr + 1, "#]" ); // at least 1 letter keyword [#  #]
            int len = stopPtr - startPtr;
            if (  stopPtr && len >= 3 && len <= 40 ) { // grab keyword if stop ok and lenth of keyword under 40
              // grab keyword
              stopPtr[0] = 0x00;  // end of keyword
              stopPtr += 2;       // skip "#]"
              aStrKey = startPtr;
              aStrKey.trim();
              // Save the line in a string
              repeatString = stopPtr;
              repeatActive = true;
              repeatNumber = 0;
            } // end if  repeat KEY ok
          }  // end if REPEAT found
        }  // endif  !repeat active  (detection REPEAT)
        if (repeatActive) {
          staticBufferLine[0] = 0x00;
          // ask the sketch if we should repeat
          repeatActive = onRepeatLine(aStrKey, repeatNumber++);
          if ( repeatActive ) strcpy(staticBufferLine, repeatString.c_str());
          size = strlen(staticBufferLine);
        }

        char* currentPtr = staticBufferLine;
        // cut line in part to deal with kerwords "[# xxxxx #]"
        while ( currentPtr = strstr(currentPtr, "[#") ) {  // start detected
          char* startPtr = currentPtr + 2;
          char* stopPtr = strstr( startPtr + 1, "#]" ); // at least 1 letter keyword [#  #]
          int len = stopPtr - startPtr;
          if (  !stopPtr || len <= 0 || len >= 50 ) { // abort if no stop or lenth of keyword over 50
            break;
          }
          // grab keyword
          stopPtr[0] = 0x00;   // aKey is Cstring
          stopPtr += 2;        // skip "#]"
          String aStr = startPtr;
          aStr.trim();
          // callback to deal with keywords
          translateKey(aStr);

          // Copie de la suite de la chaine dans une chaine
          String bBuffer = stopPtr;

          // Ajout de la chaine de remplacement
          strncpy(currentPtr, aStr.c_str(), 200);
          currentPtr += min(aStr.length(), 200U);
          // Ajoute la suite
          strncpy(currentPtr, bBuffer.c_str(), 500);
          size = strlen(staticBufferLine);

          //
        }// while
      } // else do chunk
      //
      //      D_print('.');
      if (size) Server.sendContent_P(staticBufferLine, size);
    }  // if avail
    if (doChunk) Server.chunkedResponseFinalize();
    //    D_println("<");
    //Server.client().stop();
    D_println(F("WEB: GET answered with no stop "));
    aFile.close();
    onEndOfRequest(filePath, submitValue);
    return;
  }
  //  deal with file not found
  D_println("error 404");
  String message = F("File Not Found\n");
  message += "URI: ";
  message += Server.uri();
  message += F("\nMethod: ");
  message += (Server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += Server.args(); // last is plain = all arg
  D_println(message);
  message += F("\n<br>");
  for (uint8_t i = 0; i < Server.args(); i++) {
    message += " " + Server.argName(i) + ": " + Server.arg(i) + "\n<br>";
  }
  message += "<H2><a href=\"/\">go home</a></H2><br>";
  Server.send(404, "text/html", message);
  Server.client().stop();

}
