#ifndef _WIFI_H_

#define _WIFI_H_

#include "Config.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

extern void default_web_server_setup_callback(ESP8266WebServer *webserver);

void config_host_name(char *device_name);
uint8_t connect_wifi(const char *ssid, const char *password, int timeout, int delaytime, int (*loopcallback)(void));
void connect_wifi(boolean autoconnect = true, IPAddress IP = (uint32_t)0, IPAddress GW = (uint32_t)0, IPAddress NM = (uint32_t)0);

#endif

