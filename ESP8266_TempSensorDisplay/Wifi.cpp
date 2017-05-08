#include "Wifi.h"
#include "Helper.h"
#include "Log.h"
#include <NTPClient.h>

extern char device_name[];
extern int reset_config;
extern void switchAPCallback(void);
extern void switchSTACallback(void);
extern void connectedCallback(void);
extern void connectFailCallback(void);

extern NTPClient timeClient;

void config_host_name(char *device_name) {
  String hostname;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  hostname = "ESP_" + macToName(mac) + "_" + device_name;
  WiFi.hostname(hostname);
  gLog.print(timeClient.getFormattedTime());
  gLog.printf(" Hostname = %s\n", hostname.c_str());
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Should save config");
  shouldSaveConfig = true;
}

uint8_t connect_wifi(const char *ssid, const char *password, int timeout, int delaytime, int (*loopcallback)(void)) {
  unsigned long start = millis();
  uint8_t status;

  gLog.printf("Connecting to %s\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  switchSTACallback();

  while (1) {
    status = WiFi.status();
    if (millis() > start + timeout) {
      gLog.println("Connect timeout");
      break;
    }
    if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
      break;
    }
    if (loopcallback) {
      loopcallback();
    }
    delay(delaytime);
  }
  gLog.printf("Connect status %d\n", status);
  if (status == WL_CONNECTED) {
    connectedCallback();
  }
  else {
    connectFailCallback();
  }

  return status;
}

void connect_wifi(boolean autoconnect, IPAddress IP, IPAddress GW, IPAddress NM) {
  char temp[64];
  boolean ret;

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setDebugOutput(false);

  //set callbacks
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPModeCallback(switchAPCallback);
  wifiManager.setSTAModeCallback(switchSTACallback);
  wifiManager.setConnectedCallback(connectedCallback);
  wifiManager.setConnectionFailCallback(connectFailCallback);

  //set static ip
  if (((uint32_t)IP != 0) && ((uint32_t)GW != 0) && ((uint32_t)NM != 0)) {
    gLog.print("Static IP: "); gLog.println(IP);
    wifiManager.setSTAStaticIPConfig(IP, GW, NM);
  }
  else {
    gLog.println("Dynamic IP");
  }

  //add all your parameters here
  add_user_parameters(wifiManager);

  //reset settings - for testing
  if (reset_config) {
    wifiManager.resetSettings();
  }

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until STA connect time out
  wifiManager.setConnectTimeout(10);

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(60);

  // Setup the header
  sprintf(temp, "<p>Wifi Connection Setup for %s</p>", device_name);
  wifiManager.setCustomHeadElement(temp);

  // Setup the web server hooks
  wifiManager.setWebServerSetupCallback(default_web_server_setup_callback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  sprintf(temp, "AutoConnectAP_%s", device_name);
  if (autoconnect) {
    ret = wifiManager.autoConnect(temp, "password");
  }
  else {
    ret = wifiManager.startConfigPortal(temp, "password");
  }
  if (!ret) {
    gLog.print(timeClient.getFormattedTime());
    gLog.println(" Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //read updated parameters
  read_user_parameters(wifiManager);
  remove_user_parameters(wifiManager);
}

