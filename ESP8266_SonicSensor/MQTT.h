#ifndef _MQTT_H_

#define _MQTT_H_

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Config.h"
#include "MQTTtopics.h"

extern WiFiClient wifiClient;
extern PubSubClient client;

extern const char * mqtt_topics[];
extern const char * mqtt_notification_topics[];
extern char * mqtt_will_message;

extern void mqtt_client_callback(mqtt_topic_type topic_index, String &valuestr);
extern int mqtt_setup_message(void);
extern int mqtt_connect_callback(bool connected);

void mqtt_callback(char* topic, byte* payload, unsigned int length);
void setup_mqtt(void);

#endif

