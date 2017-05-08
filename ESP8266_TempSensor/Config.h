#ifndef _CONFIG_H_

#define _CONFIG_H_

#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

//flag for saving data
extern bool shouldSaveConfig;

extern void load_user_config(JsonObject &json);
extern void save_user_config(JsonObject &json);
extern void add_user_parameters(WiFiManager &wifiManager);
extern void remove_user_parameters(WiFiManager &wifiManager);
extern void read_user_parameters(WiFiManager &wifiManager);

int load_config(void);
void save_config(void);

#endif

