#pragma once

#include <Arduino.h>
#include <bsec.h>

/* Configure the BSEC library with information about the sensor
    18v/33v = Voltage at Vdd. 1.8V or 3.3V
    3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
    4d/28d = Operating age of the sensor in days
    generic_18v_3s_4d
    generic_18v_3s_28d
    generic_18v_300s_4d
    generic_18v_300s_28d
    generic_33v_3s_4d
    generic_33v_3s_28d
    generic_33v_300s_4d
    generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_300s_4d/bsec_iaq.txt"
};

#define CS_PIN D8

#define LOG(fmt, ...) (Serial.printf("%lu: " fmt "\n", millis(), ##__VA_ARGS__))

#define N_FIELDS 5

inline bool CheckSensor(Bsec sensor) {
  if (sensor.status < BSEC_OK) {
    LOG("BSEC error, status %d!", sensor.status);
    return false;
  } else if (sensor.status > BSEC_OK) {
    LOG("BSEC warning, status %d!", sensor.status);
  }

  if (sensor.bme680Status < BME680_OK) {
    LOG("Sensor error, bme680_status %d!", sensor.bme680Status);
    return false;
  } else if (sensor.bme680Status > BME680_OK) {
    LOG("Sensor warning, status %d!", sensor.bme680Status);
  }

  return true;
}