#include "MQTT.h"
#include "Helper.h"
#include <ESP8266httpUpdate.h>
#include <Servo.h>
#include "Log.h"

extern int auto_sleep_interval;
extern int debug_mode;
extern int last_pos;

extern Servo myservo;


#define SERVO_SIGNAL_PIN  2

#define ROOM_NAME "room5"
extern char device_name[];
static int topcis_inited = 0;

const char *mqtt_topics_src[] = {
  "/home/%s/tempsensor/trigger",
  "/home/%s/tempsensor/interval",
  "/home/%s/tempsensor/temperature",
  "/home/%s/tempsensor/pressure",
  "/home/%s/tempsensor/humidity",
  "/home/%s/tempsensor/battery",
  "/home/%s/tempsensor/batteryhistory",
  "/home/%s/tempsensor/debugmode",
  "/home/%s/tempsensor/debug",
  "/home/%s/tempsensor/servopos",
  "/home/devicestatus/%s/fwotalink",
};
const char *mqtt_topics[MAX_MQTT_TOPICS] = { NULL, };
const char *mqtt_notification_topics_src[] = {
  "/home/devicestatus/%s/status",
  "/home/notification/%s/doorstatus",
};
const char *mqtt_notification_topics[MAX_MQTT_NOTIFICATION_TOPICS];
char *mqtt_will_message = NULL;
bool updated = false;


void mqtt_client_callback(mqtt_topic_type topic_index, String &valuestr) {
  int servopos;
  char output[64];
  switch (topic_index)
  {
    case TEMP_SENSOR_TRIGER:
      break;
    case TEMP_SENSOR_INTERVAL:
      GET_VALUE(auto_sleep_interval, "Auto sleep interval = %dms\n", 500, 3600000);
      break;
    case TEMP_SENSOR_DEBUG_MODE:
      GET_VALUE(debug_mode, "Debug mode = %d\n", 0, 2);
      if (debug_mode) {
        gLog.enable_serial();
      }
      else {
        gLog.disable_serial();
      }
      break;
    case TEMP_SENSOR_SERVO_POSITION:
      GET_VALUE(servopos, "Servo position = %d\n", 5, 175);
      if (servopos != -1)
      {
#define SERVO_STEP  3
#define SERVO_DELAY 20
        int pos;
        myservo.attach(SERVO_SIGNAL_PIN);  // attaches the servo on GIO2 to the servo object
        pos = last_pos;
        if (servopos < last_pos)
        {
          while (pos > servopos)
          {
            pos -= SERVO_STEP;
            if (pos < servopos)
            {
              pos = servopos;
            }
            myservo.write(pos);
            delay(SERVO_DELAY);
          }
        }
        else
        {
          while (pos < servopos)
          {
            pos += SERVO_STEP;
            if (pos > servopos)
            {
              pos = servopos;
            }
            myservo.write(pos);
            delay(SERVO_DELAY);
          }
        }
        last_pos = servopos;
        myservo.write(servopos);              // tell servo to go to position in variable 'pos'
        delay(400);                           // waits 400ms make sure the servo to reach the position
        myservo.detach();
        delay(10);
        pinMode(SERVO_SIGNAL_PIN, OUTPUT);                   // Turn off the servo control signal output, let servo idle
        digitalWrite(SERVO_SIGNAL_PIN, HIGH);
      }
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
    //client.subscribe(mqtt_topics[TEMP_SENSOR_TRIGER]);
    client.subscribe(mqtt_topics[TEMP_SENSOR_INTERVAL], 1);
    client.subscribe(mqtt_topics[TEMP_SENSOR_DEBUG_MODE], 1);
#if (SERVO_SIGNAL_PIN != 0)
    client.subscribe(mqtt_topics[TEMP_SENSOR_SERVO_POSITION], 1);
#endif

    char temp[64];
    //sprintf(temp, "Device [%s] online", device_name);
    sprintf(temp, "Online");
    client.publish(mqtt_notification_topics[NOTIFICATION_DEVICE_STATUS], temp, true);
  }
  else {
    delay(1000);
  }
}

