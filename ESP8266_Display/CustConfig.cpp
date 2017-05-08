#include "Config.h"

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40] = "";
char mqtt_port[8] = "1883";
char mqtt_user[40] = "";
char mqtt_password[64] = "";
char device_name[64] = "display";

char wifi_ssid1[16] = "";
char wifi_password1[16] = "";
char wifi_ssid2[16] = "";
char wifi_password2[16] = "";

WiFiManagerParameter *custom_mqtt_server = NULL;
WiFiManagerParameter *custom_mqtt_port = NULL;
WiFiManagerParameter *custom_mqtt_user = NULL;
WiFiManagerParameter *custom_mqtt_password = NULL;
WiFiManagerParameter *custom_device_name = NULL;

void load_user_config(JsonObject &json) {
  const char *temp;

  if (temp = json["mqtt_server"]) {
    strcpy(mqtt_server, temp);
  }
  if (temp = json["mqtt_port"]) {
    strcpy(mqtt_port, temp);
  }
  if (temp = json["mqtt_user"]) {
    strcpy(mqtt_user, temp);
  }
  if (temp = json["mqtt_password"]) {
    strcpy(mqtt_password, temp);
  }
  if (temp = json["device_name"]) {
    strcpy(device_name, temp);
  }
  if (temp = json["wifi_ssid1"]) {
    strcpy(wifi_ssid1, temp);
  }
  if (temp = json["wifi_password1"]) {
    strcpy(wifi_password1, temp);
  }
  if (temp = json["wifi_ssid2"]) {
    strcpy(wifi_ssid2, temp);
  }
  if (temp = json["wifi_password2"]) {
    strcpy(wifi_password2, temp);
  }
}

void save_user_config(JsonObject &json) {
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;
  json["mqtt_user"] = mqtt_user;
  json["mqtt_password"] = mqtt_password;
  json["device_name"] = device_name;
  json["wifi_ssid1"] = wifi_ssid1;
  json["wifi_password1"] = wifi_password1;
  json["wifi_ssid2"] = wifi_ssid2;
  json["wifi_password2"] = wifi_password2;
}

void add_user_parameters(WiFiManager &wifiManager) {
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  custom_mqtt_server = new WiFiManagerParameter("server", "mqtt server", mqtt_server, 40);
  custom_mqtt_port = new WiFiManagerParameter("port", "mqtt port", mqtt_port, 6);
  custom_mqtt_user = new WiFiManagerParameter("user", "mqtt user", mqtt_user, 40);
  custom_mqtt_password = new WiFiManagerParameter("password", "mqtt password", mqtt_password, 64, "type='password'");
  custom_device_name = new WiFiManagerParameter("devicename", "device name", device_name, 64);

  wifiManager.addParameter(custom_mqtt_server);
  wifiManager.addParameter(custom_mqtt_port);
  wifiManager.addParameter(custom_mqtt_user);
  wifiManager.addParameter(custom_mqtt_password);
  wifiManager.addParameter(custom_device_name);
}

void remove_user_parameters(WiFiManager &wifiManager) {
  if (custom_mqtt_server) {
    delete custom_mqtt_server;
    custom_mqtt_server = NULL;
  }
  if (custom_mqtt_port) {
    delete custom_mqtt_server;
    custom_mqtt_server = NULL;
  }
  if (custom_mqtt_user) {
    delete custom_mqtt_server;
    custom_mqtt_server = NULL;
  }
  if (custom_mqtt_password) {
    delete custom_mqtt_server;
    custom_mqtt_server = NULL;
  }
  if (custom_device_name) {
    delete custom_device_name;
    custom_device_name = NULL;
  }
}

void read_user_parameters(WiFiManager &wifiManager) {
  if (custom_mqtt_server) {
    strcpy(mqtt_server, custom_mqtt_server->getValue());
  }
  if (custom_mqtt_port) {
    strcpy(mqtt_port, custom_mqtt_port->getValue());
  }
  if (custom_mqtt_user) {
    strcpy(mqtt_user, custom_mqtt_user->getValue());
  }
  if (custom_mqtt_password) {
    strcpy(mqtt_password, custom_mqtt_password->getValue());
  }
  if (custom_device_name) {
    strcpy(device_name, custom_device_name->getValue());
  }
}

