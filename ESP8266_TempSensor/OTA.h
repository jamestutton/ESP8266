#ifndef _OTA_H_

#define _OTA_H_

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

extern ESP8266WebServer httpServer;
extern ESP8266HTTPUpdateServer httpUpdater;

extern void firmware_upload_start_callback(void);

int setup_ota_server(void);

#endif

