#include <BME280I2C.h>
#include <Servo.h>
#include "Wifi.h"
#include "MQTT.h"
#include "OTA.h"
#include "Helper.h"
#include "Ticker.h"
#include "Log.h"
#include <NTPClient.h>
#include "Cust.h"

typedef enum {
  WAKE_UP_MEASUREMENT_ONLY,
  WAKE_UP_TRANSMIT_ONLY,
  WAKE_UP_MEASUREMENT_AND_TRANSMIT,
} wake_up_mode_t;

typedef struct {
  int16_t temp;
  uint16_t pressure;
  uint16_t humidity;
  uint16_t battery;
} stored_data_t;

// Structure which will be stored in RTC memory.
// First field is CRC32, which is calculated based on the
// rest of structure contents.
// Any fields can go after CRC32.
// We use byte array as an example.
struct {
  uint32_t Magic;
  uint32_t crc32;
  uint32_t IP;
  uint32_t GW;
  uint32_t NM;
  uint8_t BSSID[6];
  int32_t Channel;
  uint32_t auto_sleep_interval;
  uint32_t last_sleep_time;
  wake_up_mode_t next_wakeup_mode;
  uint32_t wakeup_counter;
  int32_t num_data;
  int32_t last_temp;
  stored_data_t data[DATA_SLOTS];
} rtcData;

BME280I2C bme(4, 4, 4, 1, 5, 0, false, 0x76);                   // Default : forced mode, standby time = 1000 ms
// Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
bool metric = true;
Servo myservo;  // create servo object to control a servo

//int temperature_threshold = 5;  // 0.05 Celsius degree, or 0.09 Fahrenheit
int temperature_threshold = 10;  // 0.10 Celsius degree, or 0.18 Fahrenheit
int auto_sleep_interval = 60000;
int power_up_wait = 30000; // ms
int debug_mode = 1;
int last_pos = 0;
int find_bme280 = 0;

bool waken = false;
int reset_config = 0;
unsigned long boot_time;
unsigned long wifi_connect_time;
unsigned long mqtt_connect_time;
unsigned long sleep_time;

extern char device_name[];

extern NTPClient timeClient;
extern Ticker led_blink;

extern void led_blink_callback(void);

int search_for_bme280(void) {
  if (find_bme280) {
    return 1;
  }

#if 1
  if (bme.begin(5, 4)) {
    find_bme280 = 1;
    gLog.println("Find BME280 at 5,4");
    return 1;
  }
  if (bme.begin(0, 2)) {
    find_bme280 = 1;
    gLog.println("Find BME280 at 0,2");
    return 1;
  }
  if (bme.begin(12, 14)) {
    find_bme280 = 1;
    gLog.println("Find BME280 at 12,14");
    return 1;
  }
#endif
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Could not find BME280 sensor!");
  find_bme280 = 0;
  return 0;
}

int do_measurement(stored_data_t *data) {
  if (!find_bme280) {
    if (!search_for_bme280()) {
      return 0;
    }
  }

  char output[32];
  float temp(NAN), hum(NAN), pres(NAN);
  uint8_t pressureUnit(0);                                           // unit: B000 = Pa, B001 = hPa, B010 = Hg, B011 = atm, B100 = bar, B101 = torr, B110 = N/m^2, B111 = psi

  bme.read(pres, temp, hum, metric, pressureUnit);

  if (!isnan(temp)) {
    data->temp = (int)(temp * 100.0 + 0.5);
    print_float(output, temp, 2);
    //client.publish(mqtt_topics[TEMP_SENSOR_TEMPERATURE], output, false);
    if (debug_mode >= 1) {
      //gLog.print(timeClient.getFormattedTime());
      gLog.printf(" Temp: %s", output);
    }
  }
  else {
    data->temp = 0xFFFF;
    find_bme280 = 0;
  }

  if (!isnan(pres)) {
    pres /= 1000.0;
    data->pressure = (int)(pres * 100.0 + 0.5);
    print_float(output, pres, 2);
    //client.publish(mqtt_topics[TEMP_SENSOR_PRESSURE], output, false);
    if (debug_mode >= 1) {
      gLog.printf("  Pressure: %s", output);
    }
  }
  else {
    data->pressure = 0xFFFF;
    find_bme280 = 0;
  }

#if 0
  if (!isnan(hum)) {
    print_float(output, hum, 2);
    data->humidity = (int)(hum * 100.0 + 0.5);
    //client.publish(mqtt_topics[TEMP_SENSOR_HUMIDITY], output, false);
    if (debug_mode >= 1) {
      gLog.printf("  Humidity: %s", output);
    }
  }
  else {
    data->humidity = 0xFFFF;
    find_bme280 = 0;
  }
#else
  data->humidity = 0xFFFF;
#endif

  data->battery = ESP.getVcc();
  print_float(output, data->battery / 1024.0 * 1.108, 2);
  //data->battery = analogRead(A0);
  if (debug_mode >= 1) {
    gLog.printf("  Battery: %d %s", data->battery, output);
    gLog.println();
  }

  return 1;
}

boolean data_publish(stored_data_t *data) {
  boolean result = true;
  char temp[64], *p;
  int i;

  if (result && data && data->temp != 0xFFFF) {
    print_float(temp, (float)data->temp / 100.0, 2);
    if (client.publish(mqtt_topics[TEMP_SENSOR_TEMPERATURE], temp, true) == false) {
      result = false;
    }
  }

  if (result && data && data->pressure != 0xFFFF) {
    print_float(temp, (float)data->pressure / 100.0, 2);
    if (client.publish(mqtt_topics[TEMP_SENSOR_PRESSURE], temp, true) == false) {
      result = false;
    }
  }

  if (result && data && data->humidity != 0xFFFF) {
    print_float(temp, (float)data->humidity / 100.0, 2);
    if (client.publish(mqtt_topics[TEMP_SENSOR_HUMIDITY], temp, true) == false) {
      result = false;
    }
  }

  if (result && data && data->battery != 0xFFFF) {
    sprintf(temp, "%d", data->battery);
    if (client.publish(mqtt_topics[TEMP_SENSOR_BATTERY], temp, true) == false) {
      result = false;
    }
    client.loop();
  }

  if (debug_mode >= 1) {
    if (result && rtcData.num_data) {
      p = temp;
      p += sprintf(p, "[");
  #if 1 // Reverse order for Linear MQTT Dashboard
      for (i = rtcData.num_data - 1; i >= 0 ; i --) {
        if (i != 0) {
  #else
      for (i = 0; i < rtcData.num_data; i ++) {
        if (i != rtcData.num_data - 1) {
  #endif
          p += sprintf(p, "%d,", rtcData.data[i].battery);
        }
        else {
          p += sprintf(p, "%d]", rtcData.data[i].battery);
        }
      }
      if (debug_mode >= 2) {
        if (client.publish(mqtt_topics[TEMP_SENSOR_BATTERY_HISTORY], temp, true) == false) {
          result = false;
        }
        client.loop();
      }
    }
  }

  client.disconnect();

  return result;
}

void go_sleep(int sleeptime = -1) {
  RFMode sleepmode;

  rtcData.Magic = 0xdeadbeef;
  rtcData.auto_sleep_interval = auto_sleep_interval;
  rtcData.crc32 = calculateCRC32(((uint8_t*) &rtcData) + 8, sizeof(rtcData) - 8);
  // Write struct to RTC memory
  if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    gLog.println("Memory written");
  }

#if 0
  gLog.enable_serial();
  sleep_time = millis();
  gLog.printf("Times: b=%dms w=%dms m=%dms s=%dms vcc=%d ", boot_time, wifi_connect_time, mqtt_connect_time, sleep_time, analogRead(A0));
  gLog.println((float)analogRead(A0)/1024.0*3.2);
#endif

  switch (rtcData.next_wakeup_mode) {
    case WAKE_UP_MEASUREMENT_ONLY:
      sleepmode = WAKE_RF_DISABLED;
      break;
    case WAKE_UP_TRANSMIT_ONLY:
      sleepmode = WAKE_NO_RFCAL;
      break;
    case WAKE_UP_MEASUREMENT_AND_TRANSMIT:
      sleepmode = WAKE_RF_DEFAULT;
      break;
    default:
      break;
  }

  if (waken) {
    if (sleeptime == -1) {
      sleeptime = auto_sleep_interval;
    }
    if (sleeptime > 0) {
      sleeptime -= millis() + DEFAULT_DEEP_SLEEP_DELAY;
    }
    if (sleeptime < 0) {
      sleeptime = auto_sleep_interval;
    }
  }
  else {
    sleeptime = auto_sleep_interval;
  }

  gLog.printf("Go sleep %dms mode %d\n", sleeptime, sleepmode);
  //wifi_set_sleep_type(MODEM_SLEEP_T);
  ESP.deepSleep(sleeptime * 1000, sleepmode);
  delay(100);
}

void setup_powerup(void) {
  //initialize the light as an output and set to LOW (off)
#if (LED_LIGHT_PIN != 0)
  pinMode(LED_LIGHT_PIN, OUTPUT);
  digitalWrite(LED_LIGHT_PIN, LOW);
#endif

#if (SERVO_SIGNAL_PIN != 0)
  last_pos = 5;
  myservo.attach(SERVO_SIGNAL_PIN);  // attaches the servo on GIO2 to the servo object
  myservo.write(last_pos);
  delay(400);
  myservo.detach();
  delay(10);
  pinMode(SERVO_SIGNAL_PIN, OUTPUT); // Turn off the servo control signal output, let servo idle
  digitalWrite(SERVO_SIGNAL_PIN, HIGH);
#endif

  if (debug_mode && (debug_mode < 2)) {
    //start the serial line for debugging
    Serial.begin(115200);
    Serial.println("");
    delay(10);
  }

  //clean FS, for testing
#if 0
  gLog.println("Formatting FS");
  SPIFFS.format();
#endif

  if (reset_config) {
    gLog.println("SYSTEM RESET!!!");
    led_blink.attach(0.1, led_blink_callback);
    delay(1000);
  }

  //read configuration from FS json
  gLog.println("Loading config");
  load_config();

  config_host_name(device_name);

  gLog.println("Connecting Wifi");

#if 1
  if (connect_wifi("Lizzyjim", "aabbccddee", 8000, 1, search_for_bme280) != WL_CONNECTED) {
    if (connect_wifi("belkin.341c", "24bb6247", 8000, 1, search_for_bme280) != WL_CONNECTED) {
      connect_wifi();
    }
  }
#else
  connect_wifi();
#endif

  if (shouldSaveConfig) {
    config_host_name(device_name);
    save_config();
    delay(300);
    ESP.reset();
    delay(100);
  }

  //if you get here you have connected to the WiFi
  wifi_connect_time = millis();
  gLog.print("Wifi connected IP: ");
  gLog.println(WiFi.localIP());
  rtcData.IP = (uint32_t)WiFi.localIP();
  rtcData.GW = (uint32_t)WiFi.gatewayIP();
  rtcData.NM = (uint32_t)WiFi.subnetMask();
  memcpy(rtcData.BSSID, WiFi.BSSID(), 6);
  rtcData.Channel = wifi_get_channel();

#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, HIGH);
#endif

  setup_ota_server();

  timeClient.begin();
  timeClient.setTimeOffset(-4 * 3600);
  timeClient.setUpdateInterval(3600);
  if (timeClient.forceUpdate()) {
    gLog.print(timeClient.getFormattedTime());
    gLog.println(" NTP updated");
  }

  setup_mqtt();
  mqtt_connect_time = millis();

  rtcData.num_data = 0;
  if (do_measurement(&rtcData.data[rtcData.num_data])) {
    rtcData.num_data + 1;
    rtcData.last_temp = rtcData.data[rtcData.num_data - 1].temp;
    if (data_publish(&rtcData.data[0])) {
      rtcData.num_data = 0;
    }
  }
  else {
    rtcData.last_temp = 0;
  }

  rtcData.next_wakeup_mode = WAKE_UP_MEASUREMENT_ONLY;
  rtcData.wakeup_counter = 0;
}

int setup_network(void) {
  //initialize the light as an output and set to LOW (off)
#if (LED_LIGHT_PIN != 0)
  pinMode(LED_LIGHT_PIN, OUTPUT);
  digitalWrite(LED_LIGHT_PIN, LOW);
#endif

  //read configuration from FS json
  gLog.println("Loading config");
  load_config();

  config_host_name(device_name);

  gLog.println("Connecting Wifi");
  WiFi.mode(WIFI_STA);
  WiFi.config(rtcData.IP, rtcData.GW, rtcData.NM);
  //WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str(), rtcData.Channel, (const uint8_t*)rtcData.BSSID, true);

  unsigned long start = millis();
  uint8_t status;
  search_for_bme280();
  while (1) {
    status = WiFi.status();
    if (millis() > start + 3000) {
      break;
    }
    if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
      break;
    }
    delay(1);
  }

  if (status != WL_CONNECTED) {
    gLog.println("Connecting Wifi using manager");
    connect_wifi();//rtcData.IP, rtcData.GW, rtcData.NM);
  }

  //if you get here you have connected to the WiFi
  wifi_connect_time = millis();
  gLog.println("Wifi connected");

#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, HIGH);
#endif

  setup_mqtt();
  mqtt_connect_time = millis();
}

void setup_waken(void) {
  gLog.printf("Wake up at mode %d counter %d data %d\n", rtcData.next_wakeup_mode, rtcData.wakeup_counter, rtcData.num_data);
  rtcData.wakeup_counter ++;

  switch (rtcData.next_wakeup_mode) {
    case WAKE_UP_MEASUREMENT_ONLY:
      if (rtcData.num_data >= DATA_SLOTS) {
        // Exceed data storage, drop one
        memmove(&rtcData.data[0], &rtcData.data[rtcData.num_data - DATA_SLOTS + 1], sizeof(stored_data_t) * (DATA_SLOTS - 1));
        rtcData.num_data = DATA_SLOTS - 1;
      }
      if (do_measurement(&rtcData.data[rtcData.num_data])) {
        rtcData.num_data ++;
        if (abs(rtcData.data[rtcData.num_data - 1].temp - rtcData.last_temp) >= temperature_threshold) {
          // New update, need to transmit out
          rtcData.last_temp = rtcData.data[rtcData.num_data - 1].temp;
          rtcData.next_wakeup_mode = WAKE_UP_TRANSMIT_ONLY;
          go_sleep(TRANSMIT_ONLY_DELAY);
        }
        if (rtcData.wakeup_counter == (DATA_STORE_BEFORE_TRANSMIT - 1)) {
          rtcData.next_wakeup_mode = WAKE_UP_MEASUREMENT_AND_TRANSMIT;
        }
      }
      break;
    case WAKE_UP_TRANSMIT_ONLY:
      setup_network();
      if (data_publish(&rtcData.data[rtcData.num_data - 1])) {
        // Sent out, reset history
        rtcData.next_wakeup_mode = WAKE_UP_MEASUREMENT_ONLY;
        rtcData.wakeup_counter = 0;
        rtcData.num_data = 0;
      }
      else {
        // Failed to send out, skip to next time slot
        rtcData.next_wakeup_mode = WAKE_UP_MEASUREMENT_AND_TRANSMIT;
      }
      go_sleep(auto_sleep_interval - TRANSMIT_ONLY_DELAY);
      break;
    case WAKE_UP_MEASUREMENT_AND_TRANSMIT:
      setup_network();
      if (rtcData.num_data >= DATA_SLOTS) {
        // Exceed data storage, drop one
        memmove(&rtcData.data[0], &rtcData.data[rtcData.num_data - DATA_SLOTS + 1], sizeof(stored_data_t) * (DATA_SLOTS - 1));
        rtcData.num_data = DATA_SLOTS - 1;
      }
      if (do_measurement(&rtcData.data[rtcData.num_data])) {
        rtcData.num_data ++;
        rtcData.last_temp = rtcData.data[rtcData.num_data - 1].temp;
        if (data_publish(&rtcData.data[rtcData.num_data - 1])) {
          // Sent out, reset history
          rtcData.next_wakeup_mode = WAKE_UP_MEASUREMENT_ONLY;
          rtcData.wakeup_counter = 0;
          rtcData.num_data = 0;
          go_sleep();
        }
        else {
          // Failed to send out, try retransmit once
          rtcData.next_wakeup_mode = WAKE_UP_TRANSMIT_ONLY;
          go_sleep(TRANSMIT_ONLY_DELAY);
        }
      }
      break;
    default:
      break;
  }

  go_sleep();
}

void cust_init(void) {
  boot_time = millis();
}

void cust_setup(void) {
  // Read struct from RTC memory
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData.Magic, sizeof(rtcData.Magic))) {
    gLog.printf("Magic = %X\n", rtcData.Magic);
    if (rtcData.Magic == 0xdeadbeef) {
      if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
        if (debug_mode >= 3) {
          printMemory((uint8_t*) &rtcData, sizeof(rtcData));
        }
        uint32_t crcOfData = calculateCRC32(((uint8_t*) &rtcData) + 8, sizeof(rtcData) - 8);
        if (crcOfData != rtcData.crc32) {
          gLog.println("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!");
        }
        else {
          gLog.println("CRC32 check ok, data is probably valid.");
          auto_sleep_interval = rtcData.auto_sleep_interval;
          waken = true;
        }
      }
      else {
        gLog.println("Read RTC mem failed\n");
      }
    }
  }
  else {
    gLog.println("Read magic failed\n");
  }

  rst_info* ri = system_get_rst_info();
  if (ri != NULL) {
    if (ri->reason == REASON_EXT_SYS_RST) {
      gLog.println("Reset botton pressed!\n");
      waken = false;
    }
  }

  if (waken) {
    setup_waken();
  }
  else {
    if (debug_mode) {
      gLog.enable_serial();
    }
    setup_powerup();
  }
}

void cust_loop(void) {
  if (millis() - mqtt_connect_time > power_up_wait) {
    go_sleep();
  }
}

