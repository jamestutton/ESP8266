#include "MQTT.h"
#include "Helper.h"
#include "Log.h"
#include <NTPClient.h>

extern char mqtt_server[40];
extern char mqtt_port[8];
extern char mqtt_user[40];
extern char mqtt_password[64];
extern int debug_mode;

extern NTPClient timeClient;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  //convert topic to string to make it easier to work with
  char output[128];
  char *value = NULL;
  String valuestr;
  int i;
  mqtt_topic_type topic_index = MAX_MQTT_TOPICS;

  //Print out some debugging info
  if (debug_mode >= 2) {
    gLog.print(timeClient.getFormattedTime());
    gLog.print(" Callback update. Topic: ");
    gLog.println(topic);
  }

  for (i = 0; i < MAX_MQTT_TOPICS; i ++)
  {
    if (!strcmp(mqtt_topics[i], topic))
    {
      topic_index = (mqtt_topic_type)i;
      break;
    }
  }

  if (topic_index == MAX_MQTT_TOPICS) {
    return;
  }

  value = (char *)malloc(length + 1);
  memcpy(value, payload, length);
  value[length] = '\0';
  valuestr = value;
  free(value);

  mqtt_client_callback(topic_index, valuestr);
}

void setup_mqtt(void) {
  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if (WiFi.status() == WL_CONNECTED) {
    // Loop until we're reconnected to the MQTT server
    if (!client.connected()) {
      gLog.print(timeClient.getFormattedTime());
      gLog.println(" Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      client.setServer(mqtt_server, atoi(mqtt_port));
      client.setCallback(mqtt_callback);

      mqtt_setup_message();
      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str(), mqtt_user, mqtt_password, mqtt_will_message ? mqtt_notification_topics[NOTIFICATION_DEVICE_STATUS] : NULL, 1, true, mqtt_will_message)) {
        gLog.print(timeClient.getFormattedTime());
        gLog.println(" MQTT Connected");
        mqtt_connect_callback(true);
      }
      //otherwise print failed for debugging
      else {
        gLog.print(timeClient.getFormattedTime());
        gLog.println(" Failed.");
        mqtt_connect_callback(false);
      }
    }
  }
}

