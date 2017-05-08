#include "OTA.h"
#include "Wifi.h"
#include "WebServer.h"
#include "Helper.h"
#include "Log.h"

const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater(true);

int setup_ota_server(void) {
  httpUpdater.setup_upload_start_callback(firmware_upload_start_callback);
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);

  setup_web_server(&httpServer);

#if 0
  gLog.printf("HTTPUpdateServer ready!\nOpen http://%s.local%s or http://%d.%d.%d.%d%s in your browser\n  and login with username '%s' and password '%s'\n",
                dns_host, update_path,
                WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], update_path,
                update_username, update_password);
#else
  gLog.printf("HTTPUpdateServer ready!\nOpen http://%s.local%s or http://%d.%d.%d.%d%s in your browser\n",
                dns_host, update_path,
                WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], update_path);
#endif
}

