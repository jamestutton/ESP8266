#include "SSD1306.h"
#include "Fonts.h"

// 16x10
const char rssi_logo_frame[] PROGMEM = {
  0x00, 0x70,
  0x00, 0x50,
  0x00, 0x57,
  0x00, 0x55,
  0x70, 0x55,
  0x50, 0x55,
  0x57, 0x55,
  0x55, 0x55,
  0x77, 0x77,
};
const char rssi_logo_0[] PROGMEM = {
  0x11, 0x00,
  0x0A, 0x20,
  0x04, 0x00,
  0x0A, 0x22,
  0x11, 0x00,
  0x20, 0x22,
  0x00, 0x00,
  0x22, 0x22,
  0x00, 0x00,
  0x22, 0x22,
};
const char rssi_logo_1[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x20,
  0x00, 0x00,
  0x00, 0x22,
  0x00, 0x00,
  0x20, 0x22,
  0x00, 0x00,
  0x27, 0x22,
  0x05, 0x00,
  0x27, 0x22,
};
const char rssi_logo_2[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x20,
  0x00, 0x00,
  0x00, 0x22,
  0x00, 0x00,
  0x20, 0x22,
  0x00, 0x00,
  0x27, 0x22,
  0x07, 0x00,
  0x27, 0x22,
};
const char rssi_logo_3[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x20,
  0x00, 0x00,
  0x00, 0x22,
  0x00, 0x00,
  0x70, 0x22,
  0x50, 0x00,
  0x57, 0x22,
  0x57, 0x00,
  0x77, 0x22,
};
const char rssi_logo_4[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x20,
  0x00, 0x00,
  0x00, 0x22,
  0x00, 0x00,
  0x70, 0x22,
  0x70, 0x00,
  0x77, 0x22,
  0x77, 0x00,
  0x77, 0x22,
};
const char rssi_logo_5[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x20,
  0x00, 0x00,
  0x00, 0x27,
  0x00, 0x05,
  0x70, 0x25,
  0x70, 0x05,
  0x77, 0x25,
  0x77, 0x05,
  0x77, 0x27,
};
const char rssi_logo_6[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x20,
  0x00, 0x00,
  0x00, 0x27,
  0x00, 0x07,
  0x70, 0x27,
  0x70, 0x07,
  0x77, 0x27,
  0x77, 0x07,
  0x77, 0x27,
};
const char rssi_logo_7[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x70,
  0x00, 0x50,
  0x00, 0x57,
  0x00, 0x57,
  0x70, 0x57,
  0x70, 0x57,
  0x77, 0x57,
  0x77, 0x57,
  0x77, 0x77,
};
const char rssi_logo_8[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x70,
  0x00, 0x70,
  0x00, 0x77,
  0x00, 0x77,
  0x70, 0x77,
  0x70, 0x77,
  0x77, 0x77,
  0x77, 0x77,
  0x77, 0x77,
};

const char *rssi_logos[] = { rssi_logo_0, rssi_logo_1, rssi_logo_2, rssi_logo_3, rssi_logo_4, rssi_logo_5, rssi_logo_6, rssi_logo_7, rssi_logo_8, rssi_logo_frame, };

// 21x10
const char battery_logo_frame[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x76, 0x77, 0x17,
  0x55, 0x55, 0x15,
  0x55, 0x55, 0x15,
  0x55, 0x55, 0x15,
  0x55, 0x55, 0x15,
  0x76, 0x77, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_0[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x00, 0x10,
  0x05, 0x00, 0x10,
  0x05, 0x00, 0x10,
  0x05, 0x00, 0x10,
  0x05, 0x00, 0x10,
  0x06, 0x00, 0x10,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_1[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x00, 0x17,
  0x05, 0x00, 0x15,
  0x05, 0x00, 0x15,
  0x05, 0x00, 0x15,
  0x05, 0x00, 0x15,
  0x06, 0x00, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_2[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x00, 0x17,
  0x05, 0x00, 0x17,
  0x05, 0x00, 0x17,
  0x05, 0x00, 0x17,
  0x05, 0x00, 0x17,
  0x06, 0x00, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_3[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x70, 0x17,
  0x05, 0x50, 0x17,
  0x05, 0x50, 0x17,
  0x05, 0x50, 0x17,
  0x05, 0x50, 0x17,
  0x06, 0x70, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_4[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x70, 0x17,
  0x05, 0x70, 0x17,
  0x05, 0x70, 0x17,
  0x05, 0x70, 0x17,
  0x05, 0x70, 0x17,
  0x06, 0x70, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_5[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x77, 0x17,
  0x05, 0x75, 0x17,
  0x05, 0x75, 0x17,
  0x05, 0x75, 0x17,
  0x05, 0x75, 0x17,
  0x06, 0x77, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_6[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x77, 0x17,
  0x05, 0x77, 0x17,
  0x05, 0x77, 0x17,
  0x05, 0x77, 0x17,
  0x05, 0x77, 0x17,
  0x06, 0x77, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_7[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x76, 0x77, 0x17,
  0x55, 0x77, 0x17,
  0x55, 0x77, 0x17,
  0x55, 0x77, 0x17,
  0x55, 0x77, 0x17,
  0x76, 0x77, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_8[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x76, 0x77, 0x17,
  0x75, 0x77, 0x17,
  0x75, 0x77, 0x17,
  0x75, 0x77, 0x17,
  0x75, 0x77, 0x17,
  0x76, 0x77, 0x17,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};
const char battery_logo_charge[] PROGMEM = {
  0xF8, 0xFF, 0x0F,
  0x04, 0x00, 0x10,
  0x06, 0x08, 0x10,
  0x05, 0x0E, 0x10,
  0x85, 0xFF, 0x11,
  0xE5, 0x7F, 0x10,
  0x05, 0x1C, 0x10,
  0x06, 0x04, 0x10,
  0x04, 0x00, 0x10,
  0xF8, 0xFF, 0x0F,
};

const char *battery_logos[] = { battery_logo_0, battery_logo_1, battery_logo_2, battery_logo_3, battery_logo_4, battery_logo_5, battery_logo_6, battery_logo_7, battery_logo_8, battery_logo_charge, battery_logo_frame, };

// 17x10
const char cloud_logo[] PROGMEM = {
  0x80, 0x07, 0x00,
  0x40, 0x08, 0x00,
  0x20, 0x10, 0x00,
  0x14, 0x30, 0x00,
  0x12, 0x80, 0x00,
  0x11, 0x00, 0x01,
  0x21, 0x00, 0x01,
  0x01, 0x00, 0x01,
  0x02, 0x80, 0x00,
  0xFC, 0x7F, 0x00,
};
const char cloud_connected_logo[] PROGMEM = {
  0x80, 0x07, 0x00,
  0x40, 0x08, 0x00,
  0x20, 0x10, 0x00,
  0x14, 0x3E, 0x00,
  0x12, 0x8C, 0x00,
  0x11, 0x0A, 0x01,
  0x21, 0x01, 0x01,
  0x81, 0x00, 0x01,
  0x02, 0x80, 0x00,
  0xFC, 0x7F, 0x00,
};
const char cloud_disconnected_logo_1[] PROGMEM = {
  0x80, 0x07, 0x00,
  0x40, 0x08, 0x00,
  0x20, 0x10, 0x00,
  0x94, 0x38, 0x00,
  0x12, 0x85, 0x00,
  0x11, 0x02, 0x01,
  0x21, 0x05, 0x01,
  0x81, 0x08, 0x01,
  0x02, 0x80, 0x00,
  0xFC, 0x7F, 0x00,
};
const char cloud_disconnected_logo_2[] PROGMEM = {
};

// 16x12
const char wifi_logo[] PROGMEM = {
};

// 12x8
const char connected_logo[] PROGMEM = {
  0x70, 0x00,
  0x80, 0x00,
  0x30, 0x01,
  0x7F, 0x0F,
  0x7F, 0x0F,
  0x30, 0x01,
  0x80, 0x00,
  0x70, 0x00,
};
// 14x8
const char disconnected_logo_1[] PROGMEM = {
  0xC0, 0x01,
  0x00, 0x02,
  0x90, 0x04,
  0x7F, 0x3C,
  0x7F, 0x3C,
  0x90, 0x04,
  0x00, 0x02,
  0xC0, 0x01,
};
// 18x8
const char disconnected_logo_2[] PROGMEM = {
  0x00, 0x1C, 0x00,
  0x00, 0x20, 0x00,
  0x30, 0x49, 0x00,
  0x7F, 0xC6, 0x03,
  0x7F, 0xC6, 0x03,
  0x30, 0x49, 0x00,
  0x00, 0x20, 0x00,
  0x00, 0x1C, 0x00,
};

// 13x10
const char wifi_logo1[] PROGMEM = {
  0xF0, 0x01,
  0xFC, 0x07,
  0x0E, 0x0E,
  0xE3, 0x18,
  0xF9, 0x13,
  0x1C, 0x07,
  0x44, 0x04,
  0xF0, 0x01,
  0x10, 0x01,
  0x40, 0x00,
};
// 20x10
const char wifi_connected_logo1[] PROGMEM = {
  0xF0, 0x01, 0x00,
  0xFC, 0x07, 0x00,
  0x0E, 0x0E, 0x00,
  0xE3, 0x18, 0x00,
  0xF9, 0x13, 0x00,
  0x1C, 0x07, 0x00,
  0x44, 0x04, 0x00,
  0xF0, 0x01, 0x00,
  0x10, 0x01, 0x00,
  0x40, 0x00, 0x00,
};
// 19x12
const char wifi_connected_logo2[] PROGMEM = {
};
// 20x10
const char wifi_disconnected_logo[] PROGMEM = {
  0xF0, 0x01, 0x00,
  0xFC, 0x07, 0x00,
  0x0E, 0x0E, 0x00,
  0xE3, 0xF8, 0x0E,
  0xF9, 0xB3, 0x0B,
  0x1C, 0x67, 0x0D,
  0x44, 0xC4, 0x06,
  0xF0, 0x61, 0x0D,
  0x10, 0xA1, 0x0B,
  0x40, 0xE0, 0x0E,
};
// 23x10
const char wifi_connecting_logo[] PROGMEM = {
  0xF0, 0x01, 0x00,
  0xFC, 0x07, 0x00,
  0x0E, 0x0E, 0x00,
  0xE3, 0x18, 0x07,
  0xF9, 0x13, 0x08,
  0x1C, 0x47, 0x54,
  0x44, 0xE4, 0x38,
  0xF0, 0x51, 0x11,
  0x10, 0x81, 0x00,
  0x40, 0x00, 0x07,
};
// 23x10
const char wifi_AP_mode_logo[] PROGMEM = {
  0xF0, 0x01, 0x00,
  0xFC, 0x07, 0x00,
  0x0E, 0x0E, 0x00,
  0xE3, 0x98, 0x38,
  0xF9, 0x93, 0x48,
  0x1C, 0x47, 0x48,
  0x44, 0x44, 0x38,
  0xF0, 0xE1, 0x08,
  0x10, 0x21, 0x08,
  0x40, 0x20, 0x08,
};
