## Main version is here :arrow_right: [Main Branch](https://github.com/net234/AppWebServer)
## Text en francais :arrow_right: [En Francais](LISEZMOI.md)

# AppWebServer beta version B02 (need BetaEvents.lib)

Simple Arduino lib to build an interative aplication upon local html pages
A file oriented Web Server build for dynamic user inteface with ESP8266 application.
Handle simple Server connected to local WiFi or Standalone WiFi server with no connection to internet</P>
This library uses classic HTML CSS javascipt files with dynamic (ajax like) update.

#### This works is build and tested with the ESP8266 Arduino platform with release(2.7.0) https://github.com/esp8266/Arduino

## Contents
 - [How it works](#how-it-works)
 - [Quick start](#quick-start)
 - [Releases](#releases)
 - [Contributors](#Contributors)


## How It Works
- Standard web pages recorded on the flash memory can be displayed on a browser.
- Specific keywords on html pages can be replace with dynamic data on the fly.

## Quick Start
- Open "FirstDemo" in your arduino IDE (File Sample AppWebServer).
- Select in tools menu -> Generic ESP8266 Module (should work on most ESP8266)
- Select in tools menu -> erase flash "Sketch and WiFi Settings" (only for this demo)
- Select in tools menu -> builtIn led: "2"
- Select in tools menu -> Flash Size 4MB (FS:3MB...
- Flash the web part : tool menu -> Esp8266 LittleFS Data Upload
- Compile and download Firstdemo.

- Connect to WiFi "APPWEBDEMO" with your computer or tablet.
- After few seconds a window will popup with a configuration page (if no popup windows call http://10.10.10.10)
- Configure your local WiFi name and password and save.
- Reconnect your computer to your local WiFi and call  http://appwebdemo.local
- You will got access to your ESP8266 and you can turn off or on the board LED.
- See more dynamics demo in Exemple


## Releases https://github.com/net234/AppWebServer
##### betaversion B01  27/12/2020 - pre release of the lib Copyright 2020 Pierre HENRY net23@frdev.com 


works with the staging release ver. 1.6.5-1044-g170995a, built on Aug 10, 2015 of the ESP8266 Arduino library.


### Contributions and thanks
The support and help I got from the community has been nothing short of phenomenal. I can't thank you guys enough. This is my first real attept in developing open source stuff and I must say, now I understand why people are so dedicated to it, it is because of all the wonderful people involved.

__THANK YOU__

[tzapu](https://github.com/tzapu/WiFiManager) for the great work on WiFimanager.  AppWebServer dont use WiFimanager but the same functionality are implemented.

lot more to add

Sorry if i have missed anyone.

#### Inspiration
- http://www.esp8266.com/viewtopic.php?f=29&t=2520
