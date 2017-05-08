#include "MQTT.h"
#include "Helper.h"
#include <ESP8266httpUpdate.h>
#include "Log.h"

extern int audo_trigger_interval;
extern int measurement_timeout;
extern int measurement_denoise;
extern int measurement_denoise_delay;
extern int measurement_error_threshold;
extern int door_open_threshold;
extern int door_close_threshold;
extern int door_velocity_threshold;
extern int door_opener_trigger_time;
extern int door_opener_trigger_interval;
extern int debug_mode;

extern int do_measurement(void);
extern int trigger_opener(int ms);


extern char device_name[];
static int topcis_inited = 0;

const char *mqtt_topics_src[] = {
  "/home/%s/sonicsensor/trigger",
  "/home/%s/sonicsensor/interval",
  "/home/%s/sonicsensor/timeout",
  "/home/%s/sonicsensor/denoise",
  "/home/%s/sonicsensor/denoisedelay",
  "/home/%s/sonicsensor/errorthreshold",
  "/home/%s/sonicsensor/doorstatus",
  "/home/%s/sonicsensor/dooropenthreshold",
  "/home/%s/sonicsensor/doorclosethreshold",
  "/home/%s/sonicsensor/velocitythreshold",
  "/home/%s/sonicsensor/debugmode",
  "/home/%s/sonicsensor/time",
  "/home/%s/sonicsensor/distance",
  "/home/%s/sonicsensor/distancehistory",
  "/home/%s/sonicsensor/averagedistance",
  "/home/%s/sonicsensor/dooropener",
  "/home/%s/sonicsensor/dooropenertriggertime",
  "/home/%s/sonicsensor/dooropenertriggerinterval",
  "/home/%s/sonicsensor/debug",
  "/home/devicestatus/%s/fwotalink",
};
const char *mqtt_topics[MAX_MQTT_TOPICS] = { NULL, };
const char *mqtt_notification_topics_src[] = {
  "/home/devicestatus/%s/status",
  "/home/notifications/%s/doorstatus",
};
const char *mqtt_notification_topics[MAX_MQTT_NOTIFICATION_TOPICS] = { NULL, };
char *mqtt_will_message = NULL;
bool updated = false;


void mqtt_client_callback(mqtt_topic_type topic_index, String &valuestr) {
  char output[64];
  switch (topic_index)
  {
    case SONIC_SENSOR_TRIGER:
      do_measurement();
      break;
    case SONIC_SENSOR_INTERVAL:
      GET_VALUE(audo_trigger_interval, "Auto trigger interval = %dms\n", 500, 3600000);
      break;
    case SONIC_SENSOR_TIMEOUT:
      GET_VALUE(measurement_timeout, "Measurement timeout = %dms\n", 10, 300);
      break;
    case SONIC_SENSOR_DENOISE:
      GET_VALUE(measurement_denoise, "Measurement denoise time = %d\n", 1, 10);
      break;
    case SONIC_SENSOR_DENOISE_DELAY:
      GET_VALUE(measurement_denoise_delay, "Measurement denoise delay = %dms\n", 10, 500);
      break;
    case SONIC_SENSOR_ERROR_THRESHOLD:
      GET_VALUE(measurement_error_threshold, "Measurement error threshold = %dmm\n", 0, 500);
      break;
    case SONIC_SENSOR_DOOR_STATUS:
      break;
    case SONIC_SENSOR_DOOR_OPEN_THRESHOLD:
      GET_VALUE(door_open_threshold, "Door open threshold = %dmm\n", -10000, 10000);
      break;
    case SONIC_SENSOR_DOOR_CLOSE_THRESHOLD:
      GET_VALUE(door_close_threshold, "Door close threshold = %dmm\n", -10000, 10000);
      break;
    case SONIC_SENSOR_VELOCITY_THRESHOLD:
      GET_VALUE(door_velocity_threshold, "Door velocity threshold = %dmm/s\n", 100, 500);
      break;
    case SONIC_SENSOR_DEBUG_MODE:
      GET_VALUE(debug_mode, "Debug mode = %d\n", 0, 2);
      break;
    case SONIC_SENSOR_MEASURED_TIME:
      break;
    case SONIC_SENSOR_MEASURED_DISTANCE:
      break;
    case SONIC_SENSOR_DOOR_OPENER_TRIGGER:
      if (valuestr == "TRIGGER") {
        trigger_opener(door_opener_trigger_time);
      }
      break;
    case SONIC_SENSOR_DOOR_OPENER_TRIGGER_TIME:
      GET_VALUE(door_opener_trigger_time, "Door opener trigger time = %dms\n", 100, 500);
      break;
    case SONIC_SENSOR_DOOR_OPENER_TRIGGER_INTERVAL:
      GET_VALUE(door_opener_trigger_interval, "Door opener trigger interval = %dms\n", 1000, 60000);
      break;
    case SONIC_SENSOR_DEBUG:
      break;
    case DEVICE_FIRMWARE_OTA_LINK:
      if (valuestr.length() > 10) {
        gLog.printf("OTA update from %s\n", valuestr.c_str());
        ESPhttpUpdate.rebootOnUpdate(false);
        t_httpUpdate_return ret = ESPhttpUpdate.update(valuestr);
        switch(ret) {
          case HTTP_UPDATE_FAILED:
            gLog.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
          case HTTP_UPDATE_NO_UPDATES:
            gLog.println("HTTP_UPDATE_NO_UPDATES");
            break;
          case HTTP_UPDATE_OK:
            gLog.println("HTTP_UPDATE_OK");
            updated = true;
            setup_mqtt();
            client.publish(mqtt_topics[DEVICE_FIRMWARE_OTA_LINK], "", true);
            client.loop();
            ESP.restart();
            break;
        }
      }
      break;
    default:
      break;
  }
}

int mqtt_setup_message(void) {
  int i;
  char temp[128];

  if (!topcis_inited) {
    topcis_inited = 1;
    for (i = 0; i < MAX_MQTT_TOPICS; i ++) {
      if (mqtt_topics[i]) {
        free((void *)mqtt_topics[i]);
      }
      sprintf(temp, mqtt_topics_src[i], device_name);
      mqtt_topics[i] = strdup(temp);
    }
    for (i = 0; i < MAX_MQTT_NOTIFICATION_TOPICS; i ++) {
      if (mqtt_notification_topics[i]) {
        free((void *)mqtt_notification_topics[i]);
      }
      sprintf(temp, mqtt_notification_topics_src[i], device_name);
      mqtt_notification_topics[i] = strdup(temp);
    }
    if (mqtt_will_message) {
      free(mqtt_will_message);
    }
    //sprintf(temp, "Device [%s] offline", device_name);
    sprintf(temp, "Offline");
    mqtt_will_message = strdup(temp);
  }
}

int mqtt_connect_callback(bool connected) {
  if (connected) {
    if (!updated) {
      client.subscribe(mqtt_topics[DEVICE_FIRMWARE_OTA_LINK], 1);
    }
    //client.subscribe(mqtt_topics[SONIC_SENSOR_TRIGER]);
    client.subscribe(mqtt_topics[SONIC_SENSOR_INTERVAL], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_TIMEOUT], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DENOISE], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DENOISE_DELAY], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_ERROR_THRESHOLD], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DOOR_OPEN_THRESHOLD], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DOOR_CLOSE_THRESHOLD], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_VELOCITY_THRESHOLD], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DOOR_OPENER_TRIGGER], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DOOR_OPENER_TRIGGER_TIME], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DOOR_OPENER_TRIGGER_INTERVAL], 1);
    client.subscribe(mqtt_topics[SONIC_SENSOR_DEBUG_MODE], 1);

    char temp[64];
    //sprintf(temp, "Device [%s] online", device_name);
    sprintf(temp, "Online");
    client.publish(mqtt_notification_topics[NOTIFICATION_DEVICE_STATUS], temp, true);
  }
  else {
    delay(1000);
  }
}

