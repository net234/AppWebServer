/*************************************************
 *************************************************
    AppWebCaptive.h   part of of lib AppWebServer - Arduino lib for easy wifi interface with http and css
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

    gestion d'un captive DNS server pour le mode AP
*/


// TODO: put back this in appWebServer.ccp
// ============   tools to handle captive portal  ============
// Captive DNS follow WiFiGetmode  in web handle event
bool captiveDNS = false;               //Captive portal DNS active



//// CaptiveDNS is active until a request to setupServer is done
void   captiveDNSStart() {
  D_println(F("DNS: Captive DNS ON"));
  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  if (!dnsServer.start(DNS_PORT, "*", WiFi.softAPIP()) ) {
    D1_println(F("DNS: Captive DNS erreur !!!!"));
  }
  //wifi_softap_dhcps_stop();
  //wifi_softap_dhcps_start();  TODO: validate if we need to restart dhcp
  captiveDNS = true;
}


void captiveDNSStop() {
  D_println(F("DNS: Captive DNS OFF"));
  dnsServer.stop();
  captiveDNS = false;
}

//void handleCaptivePortal() {
//  //#define DELAY_NOVALIDPAGE 1U * 60 * 1000
//  //#define CAPTIVEPORTAL_TIMEOUT  2U * 60 * 1000
//
//  static uint32_t prevMillis = millis();
//  if (millis() - prevMillis >= 1000) {
//    prevMillis += 1000;
//
//    if (timerCaptivePortal) {
//      //D_println(timerCaptivePortal);
//      if (--timerCaptivePortal == 0) {
//        captiveDNSStop();
//        WiFi.enableAP(false);  // desable AP
//      }
//    }
//  }
//  //  if (softAP && millis() - timerCaptivePortal > CAPTIVEPORTAL_TIMEOUT) {
//  //    if (miniServerPtr) miniServerPtr->configureWiFi(false);  //Stop captive
//  //    WiFi.begin();
//  //    return;
//  //  }
//  // if no request arrived we rearm captive DNS
//  //  if (!captiveDNS && millis() - timerCaptiveDNS > DELAY_NOVALIDPAGE) {
//  //    captiveDNSStart();
//  //  }
//  if (captiveDNS) dnsServer.processNextRequest();
//}

//bool    newConfigTry = false;
//bool    newConfigValide = false;
//String  newConfigSSID;
//String  newConfigPASS;


//
// ============   tools to handle captive portal  (END) ============
