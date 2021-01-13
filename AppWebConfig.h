/*************************************************
 *************************************************
    AppWebConfig.cpp  is part of of lib AppWebServer - Arduino lib for easy wifi interface with http and css
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

    Gestion du fichier de configuration
*/

#pragma once
#include <arduino.h>
// littleFS
#include <LittleFS.h>  //Include File System Headers 
#define TWFS  LittleFS


#ifdef DEBUG_ON 
#define D_print    Serial.print
#define D_println  Serial.println
#else
#define D_print(...)    while(0) {  }
#define D_println(...)  while(0) {  }
#endif
#define D1_print    Serial.print
#define D1_println  Serial.println

// Objet to deal with file config
class FileConfig {
  public:
    bool read();        //read from file  .conf or .ini
    bool save();        //save to file    .conf
    bool erase();       //erase file      .conf

    //  bool fromFile = false;  //mean was read from a file
    bool changed = false;   //need to be saved
    String webName;         //name of the web pages (to check concordance with sketch)
    String deviceName;      //saved device name
    String defaultWebFolder;       //default web folder path   
    String captiveWebFolder;      //webfolder path for CativeAP
    int bootForceAP = 0;    //if AP mode is off force temporary AP mode at boot  (mostly for debug) 
  private:
    String getParam(const String &astring);
};
