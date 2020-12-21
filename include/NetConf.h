#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define MDNS_NAME "esp8266"
#define NODE_DEFAULT_ADDRESS ((uint32_t) 211)

typedef struct {
  IPAddress ip, gateway, netmask, dns1, dns2;
  String ssid, pwd;
} NetConfigSetting;