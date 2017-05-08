#ifndef _WEB_SERVER_H_

#define _WEB_SERVER_H_

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern char dns_host[];

int setup_web_server(ESP8266WebServer *httpServer);

#endif

