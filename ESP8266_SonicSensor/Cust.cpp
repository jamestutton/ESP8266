#include "MQTT.h"
#include "Log.h"
#include <NTPClient.h>
#include "Cust.h"

//Sonic Sensor trigger/echo pins
#define SENSOR_TRIGER_PIN 15
#define SENSOR_ECHO_PIN   13
#define OPENER_TRIGGER_PIN  5

int audo_trigger_interval = 2000; // ms
int measurement_timeout = 100;    // ms
int measurement_denoise = 20;      // times
int measurement_denoise_delay = 10;    // ms
int measurement_error_threshold = 100;  // mm
int door_status = -1;
int door_open_threshold = -1000;  // mm
int door_close_threshold = +3000; // mm
int door_velocity_threshold = 60;  // mm/s
int door_opener_trigger_time = 200; // ms
int door_opener_trigger_interval = 2000;  // ms
int debug_mode = 2;
#define DENOISE_FILTER_THRESHOLD  500 // mm
#define DENOISE_FILTER_THRESHOLD_RATIO  5 // 80% valid possibility
#define NUMBER_DISTANCE_HISTORY 4
int last_distances[NUMBER_DISTANCE_HISTORY];
unsigned long lasttriggertime;
unsigned long lastopenertriggertime;
int reset_config = 0;

extern NTPClient timeClient;

int do_measurement_once(int &distance) {
  char output[32];
  int valid = 0;
  unsigned long now, start, end, t;
  int new_status = -1;

  digitalWrite(LED_LIGHT_PIN, LOW);
  digitalWrite(SENSOR_TRIGER_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(SENSOR_TRIGER_PIN, HIGH);
  delayMicroseconds(50);
  digitalWrite(SENSOR_TRIGER_PIN, LOW);

#if 0
  start = micros();
  t = start;
  while (1) {
    if (digitalRead(SENSOR_ECHO_PIN) == HIGH) {
      gLog.printf("HIGH = %4d  ", micros() - t);
      start = micros();
      valid = 1;
      break;
    }
    if (micros() - start > measurement_timeout * 1000) {
      if (debug_mode >= 2) {
        client.publish(mqtt_topics[SONIC_SENSOR_DEBUG], "Wait high timedout", true);
      }
      valid = 0;
      break;
    }
  }
  while (valid) {
    if (digitalRead(SENSOR_ECHO_PIN) == LOW) {
      end = micros();
      gLog.printf("LOW = %6d  dis = %5d\n", end - start, (end - start) * 340 / 2000);
      valid = 1;
      break;
    }
    if (micros() - start > measurement_timeout * 1000) {
      if (debug_mode >= 2) {
        client.publish(mqtt_topics[SONIC_SENSOR_DEBUG], "Wait low timedout", true);
      }
      valid = 0;
      break;
    }
  }
  
  digitalWrite(LED_LIGHT_PIN, HIGH);

  if (valid) {
    distance = (end - start) * 340 / 2000;
    if (debug_mode >= 2) {
      sprintf(output, "%d", end - start);
      client.publish(mqtt_topics[SONIC_SENSOR_MEASURED_TIME], output, true);
      sprintf(output, "%d", distance);
      client.publish(mqtt_topics[SONIC_SENSOR_MEASURED_DISTANCE], output, true);
    }
  }
#else
  distance = pulseIn(SENSOR_ECHO_PIN, HIGH, measurement_timeout * 1000) * 340 / 2000;
  digitalWrite(LED_LIGHT_PIN, HIGH);
  if (distance != 0) {
    if (debug_mode >= 1) {
      gLog.printf("%4d ", distance);
    }
    valid = 1;
    if (debug_mode >= 2) {
      sprintf(output, "%d", distance);
      client.publish(mqtt_topics[SONIC_SENSOR_MEASURED_DISTANCE], output, true);
    }
  }
  else {
    if (debug_mode >= 1) {
      gLog.print(" --  ");
    }
    valid = 0;
    if (debug_mode >= 2) {
      client.publish(mqtt_topics[SONIC_SENSOR_DEBUG], "Wait signal timedout", true);
    }
  }
#endif

  return valid;
}

int do_measurement(void) {
  char output[128], *p;
  int valid = 0;
  int distance, new_status = -1;
  int i, n = 0, t;
  int distances[measurement_denoise], total_distance = 0;

  p = output;
  p += sprintf(p, "[");
  for (i = 0; i < measurement_denoise; i ++) {
    valid = do_measurement_once(distance);
    if (valid) {
      distances[n ++] = distance;
      total_distance += distance;
      p += sprintf(p, "%d,", distance);
    }
    if (i != measurement_denoise - 1) {
      delay(measurement_denoise_delay);
    }
  }
  p += sprintf(p, "]");
  if (debug_mode >= 1) {
    client.publish(mqtt_topics[SONIC_SENSOR_MEASURED_DISTANCE_HISTORY], output, false);
    if (n) {
      distance = total_distance / n;
      gLog.printf(">> %4d ", distance);
    }
    else {
      gLog.println();
    }
  }

  if (n > 1) {
    distance = total_distance / n;

    t = n;
    for (i = 0; i < n; i ++) {
#if 0
      if (abs(distances[i] - distance) >= DENOISE_FILTER_THRESHOLD) {
#else
      if (abs(distances[i] - distance) >= (distance / DENOISE_FILTER_THRESHOLD_RATIO)) {
#endif
        total_distance -= distances[i];
        t --;
        if (debug_mode >= 1) {
          gLog.printf("-%d ", i);
        }
      }
    }

    if (t < (measurement_denoise / 3)) {
      if (debug_mode >= 1) {
        gLog.println("|| Drop all.");
      }
      return 0;
    }
    distance = total_distance / t;

    if (debug_mode >= 1) {
      sprintf(output, "%d", distance);
      client.publish(mqtt_topics[SONIC_SENSOR_MEASURED_AVERAGE_DISTANCE], output, true);
      gLog.printf("|| %4d\n", distance);
    }

    if (door_open_threshold < 0) {
      if (distance <= -door_open_threshold) {
        new_status = 1;
      }
    } else {
      if (distance >= door_open_threshold) {
        new_status = 1;
      }
    }

    if (door_close_threshold < 0) {
      if (distance <= -door_close_threshold) {
        new_status = 0;
      }
    } else {
      if (distance >= door_close_threshold) {
        new_status = 0;
      }
    }

    if (debug_mode >= 2) {
      sprintf(output, "%d: %d/%d %d", distance, door_open_threshold, door_close_threshold, new_status);
      gLog.println(output);
    }

    if ((new_status != -1) && (new_status != door_status)) {
      door_status = new_status;
      if (door_status) {
        gLog.print(timeClient.getFormattedTime());
        gLog.println(" Door opened");
        client.publish(mqtt_topics[SONIC_SENSOR_DOOR_STATUS], "OPEN", true);
#if ENABLE_NOFITICATION
        client.publish(mqtt_notification_topics[NOTIFICATION_SONIC_SENSOR_DOOR_STATUS], "Garage Door OPENED!");
#endif
      } else {
        gLog.print(timeClient.getFormattedTime());
        gLog.println(" Door closed");
        client.publish(mqtt_topics[SONIC_SENSOR_DOOR_STATUS], "CLOSE", true);
#if ENABLE_NOFITICATION
        client.publish(mqtt_notification_topics[NOTIFICATION_SONIC_SENSOR_DOOR_STATUS], "Garage Door CLOSED!");
#endif
      }
    }

    if (n >= measurement_denoise - 1) {
      for (i = 0; i < NUMBER_DISTANCE_HISTORY - 1; i ++) {
        last_distances[i] = last_distances[i + 1];
      }
      last_distances[i] = distance;
      if (debug_mode >= 2) {
        gLog.printf("His: %d %d %d %d\n", last_distances[0], last_distances[1], last_distances[2], last_distances[3]);
      }

      if (new_status == -1) {
        n = 0;
        for (i = 0; i < NUMBER_DISTANCE_HISTORY - 1; i ++) {
          if (last_distances[i] != -10000) {
            if (last_distances[i] - last_distances[i + 1] > (door_velocity_threshold * audo_trigger_interval / 1000)) {
              n --;
            }
            else if (last_distances[i] - last_distances[i + 1] < -(door_velocity_threshold * audo_trigger_interval / 1000)) {
              n ++;
            }
          }
        }
  
        if (debug_mode >= 2) {
          gLog.printf("door %d  n %d\n", door_status, n);
        }
        if ((door_status == 0) && (n <= -(NUMBER_DISTANCE_HISTORY - 1))) {
          // Door closed, distance declining, door opening?
          gLog.print(timeClient.getFormattedTime());
          gLog.println(" Door opening");
          client.publish(mqtt_topics[SONIC_SENSOR_DOOR_STATUS], "OPENING", true);
#if ENABLE_NOFITICATION
          client.publish(mqtt_notification_topics[NOTIFICATION_SONIC_SENSOR_DOOR_STATUS], "Garage Door OPENING!");
#endif
          // Reset door status, make sure it will be updated later
          door_status = -1;
        }
        else if ((door_status == 1) && (n >= (NUMBER_DISTANCE_HISTORY - 1))) {
          // Door opened, distance ascending, door closing?
          gLog.print(timeClient.getFormattedTime());
          gLog.println(" Door closing");
          client.publish(mqtt_topics[SONIC_SENSOR_DOOR_STATUS], "CLOSING", true);
#if ENABLE_NOFITICATION
          client.publish(mqtt_notification_topics[NOTIFICATION_SONIC_SENSOR_DOOR_STATUS], "Garage Door CLOSING!");
#endif
          // Reset door status, make sure it will be updated later
          door_status = -1;
        }
      }
    }
    return 1;
  }
  else {
    gLog.println(">> No enough sample");
  }

  return 0;
}

int trigger_opener(int ms) {
  unsigned int now = millis();

  if ((now - lastopenertriggertime) >= door_opener_trigger_interval) {
    lastopenertriggertime = now;
    gLog.print(timeClient.getFormattedTime());
    gLog.println(" Trigger door opener!");
    digitalWrite(OPENER_TRIGGER_PIN, LOW);
    delay(ms);
    digitalWrite(OPENER_TRIGGER_PIN, HIGH);
    gLog.print(timeClient.getFormattedTime());
    gLog.println(" Door opener trigger released!");
  }
  else {
    gLog.print(timeClient.getFormattedTime());
    gLog.printf(" Ignore too frequent door opener trigger request %dms < %dms!\n", now - lastopenertriggertime, door_opener_trigger_interval);
  }
}

void cust_init(void) {
#if (SYSTEM_RESET_PIN != 0)
  pinMode(SYSTEM_RESET_PIN, INPUT);
  if (digitalRead(SYSTEM_RESET_PIN) == LOW) {
    reset_config = 1;
  }
#endif

  gLog.enable_log();
  if (debug_mode) {
    gLog.enable_serial();
  }

  pinMode(OPENER_TRIGGER_PIN, OUTPUT);
  digitalWrite(OPENER_TRIGGER_PIN, HIGH);

  pinMode(SENSOR_TRIGER_PIN, OUTPUT);
  digitalWrite(SENSOR_TRIGER_PIN, LOW);
  pinMode(SENSOR_ECHO_PIN, INPUT);

  //initialize the light as an output and set to LOW (off)
#if (LED_LIGHT_PIN != 0)
  pinMode(LED_LIGHT_PIN, OUTPUT);
  digitalWrite(LED_LIGHT_PIN, LOW);
#endif
}

void cust_setup(void) {
#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, HIGH);
#endif

  for (int i = 0; i < NUMBER_DISTANCE_HISTORY; i ++) {
    last_distances[i] = -10000;
  }
  lasttriggertime = millis();
  lastopenertriggertime = millis();
}

void cust_loop(void) {
  if (millis() - lasttriggertime > audo_trigger_interval) {
    lasttriggertime = millis();
    do_measurement();
  }
}

