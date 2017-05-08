#include "MQTT.h"
#include <ESP8266httpUpdate.h>
#include <TFT_22_ILI9225.h>
#include "Log.h"

/* MQTT OTA over HTTP server is supported by publich the message using mosquitto_pub, for example
 * mosquitto_pub -t "/home/devicestatus/display2/fwotalink" -m "http://192.168.1.129:8000/ESP8266_Display2.ino.d1.bin"
 */

extern TFT_22_ILI9225 display;

extern char device_name[];
static int topcis_inited = 0;

const char *mqtt_topics[] = {
  "/home/garage/sonicsensor/doorstatus",
  "/home/room1/tempsensor/temperature",
  "/home/room2/tempsensor/temperature",
  "/home/room3/tempsensor/temperature",
  "/home/room4/tempsensor/temperature",
  "/home/room5/tempsensor/temperature",
  "/home/devicestatus/display2/fwotalink",
};
const char *mqtt_notification_topics_src[] = {
  "/home/devicestatus/%s/status",
};
const char *mqtt_notification_topics[MAX_MQTT_NOTIFICATION_TOPICS] = { NULL, };
char *mqtt_will_message = NULL;
bool updated = false;

mqtt_topic_type last_focus = MAX_MQTT_TOPICS;

void set_focus(mqtt_topic_type focus) {
  switch (last_focus) {
    case SONIC_SENSOR1_DOOR_STATUS:
      display.fillRectanglewh(0, 20, 4, 16, COLOR_BLACK);
      break;
    case TEMP_SENSOR1_TEMPERATURE:
      display.fillRectanglewh(0, 50, 4, 16, COLOR_BLACK);
      break;
    case TEMP_SENSOR2_TEMPERATURE:
      display.fillRectanglewh(0, 75, 4, 16, COLOR_BLACK);
      break;
    case TEMP_SENSOR3_TEMPERATURE:
      display.fillRectanglewh(0, 100, 4, 16, COLOR_BLACK);
      break;
    case TEMP_SENSOR4_TEMPERATURE:
      display.fillRectanglewh(0, 125, 4, 16, COLOR_BLACK);
      break;
    case TEMP_SENSOR5_TEMPERATURE:
      display.fillRectanglewh(0, 150, 4, 16, COLOR_BLACK);
      break;
    default:
      break;
  }

  last_focus = focus;
  switch (last_focus) {
    case SONIC_SENSOR1_DOOR_STATUS:
      display.fillRectanglewh(0, 20 + 4, 4, 16 - 10, COLOR_BLUEVIOLET);
      break;
    case TEMP_SENSOR1_TEMPERATURE:
      display.fillRectanglewh(0, 50 + 4, 4, 16 - 10, COLOR_BLUEVIOLET);
      break;
    case TEMP_SENSOR2_TEMPERATURE:
      display.fillRectanglewh(0, 75 + 4, 4, 16 - 10, COLOR_BLUEVIOLET);
      break;
    case TEMP_SENSOR3_TEMPERATURE:
      display.fillRectanglewh(0, 100 + 4, 4, 16 - 10, COLOR_BLUEVIOLET);
      break;
    case TEMP_SENSOR4_TEMPERATURE:
      display.fillRectanglewh(0, 125 + 4, 4, 16 - 10, COLOR_BLUEVIOLET);
      break;
    case TEMP_SENSOR5_TEMPERATURE:
      display.fillRectanglewh(0, 150 + 4, 4, 16 - 10, COLOR_BLUEVIOLET);
      break;
    default:
      break;
  }
}

void mqtt_client_callback(mqtt_topic_type topic_index, String &valuestr) {
  display.setFont(Terminal11x16);
  switch (topic_index) {
    case SONIC_SENSOR1_DOOR_STATUS:
      //display.setTextAlignment(TEXT_ALIGN_CENTER);
      if (valuestr == "OPEN") {
        display.fillRectanglewh(10, 20, 70, 23, COLOR_RED);
        display.drawRectanglewh(10, 20, 70, 23, COLOR_WHITE);
        display.setBackgroundColor(COLOR_RED);
        display.drawText(22, 24, valuestr, COLOR_YELLOW);
        display.setBackgroundColor(COLOR_BLACK);
      }
      else {
        display.fillRectanglewh(10, 20, 70, 23, COLOR_BLACK);
        display.drawRectanglewh(10, 20, 70, 23, COLOR_WHITE);
        display.drawText(18, 24, valuestr, COLOR_YELLOW);
      }
      //display.setTextAlignment(TEXT_ALIGN_LEFT);
      break;
    case TEMP_SENSOR1_TEMPERATURE:
      display.fillRectanglewh(10, 50, 40, 16, COLOR_BLACK);
      display.drawText(10, 50, valuestr, COLOR_YELLOW);
      break;
    case TEMP_SENSOR2_TEMPERATURE:
      display.fillRectanglewh(10, 75, 40, 16, COLOR_BLACK);
      display.drawText(10, 75, valuestr, COLOR_YELLOW);
      break;
    case TEMP_SENSOR3_TEMPERATURE:
      display.fillRectanglewh(10, 100, 40, 16, COLOR_BLACK);
      display.drawText(10, 100, valuestr, COLOR_YELLOW);
      break;
    case TEMP_SENSOR4_TEMPERATURE:
      display.fillRectanglewh(10, 125, 40, 16, COLOR_BLACK);
      display.drawText(10, 125, valuestr, COLOR_YELLOW);
      break;
    case TEMP_SENSOR5_TEMPERATURE:
      display.fillRectanglewh(10, 150, 40, 16, COLOR_BLACK);
      display.drawText(10, 150, valuestr, COLOR_YELLOW);
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
  set_focus(topic_index);
  display.setFont(Terminal6x8);
}

int mqtt_setup_message(void) {
  int i;
  char temp[128];

  if (!topcis_inited) {
    topcis_inited = 1;
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
  int i;

  if (connected) {
    display.fillRectanglewh(0, 0, display.maxX(), display.maxY(), COLOR_BLACK);
    display.drawText(0, 0, "Connected to MQTT server", COLOR_BLUE);
    if (!updated) {
      client.subscribe(mqtt_topics[DEVICE_FIRMWARE_OTA_LINK], 1);
    }
    for (i = 0; i < MAX_MQTT_TOPICS; i ++) {
      if (i != DEVICE_FIRMWARE_OTA_LINK) {
        client.subscribe(mqtt_topics[i]);
      }
    }

    char temp[64];
    //sprintf(temp, "Device [%s] online", device_name);
    sprintf(temp, "Online");
    client.publish(mqtt_notification_topics[NOTIFICATION_DEVICE_STATUS], temp, true);
  }
  else {
    display.fillRectanglewh(0, 0, display.maxX(), display.maxY(), COLOR_BLACK);
    display.drawText(0, 0, "Connect MQTT server failed", COLOR_RED);
    delay(1000);
  }
}

