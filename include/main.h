#pragma once

#include <Arduino.h>

//#define HTU21D_SENSOR //default
#define BME680_SENSOR

#if defined(HTU21D_SENSOR) && defined(BME680_SENSOR)
  #undef BME680_SENSOR
#endif

//==============================================================

#define TIMEOUT_STEP_MS 500
#define TIMEOUT_MS 30000

//==============================================================

//from BSEC library:
#define BSEC_MAX_STATE_BLOB_SIZE     (139)        /*!< Maximum size (in bytes) of the data blobs returned by bsec_get_state()*/

#define FW_VERSION "V0.2"

#define BUTTON_PIN D3

#define FACTORY_RESET_TIME_MS 5000

#define DEEPSLEEP_CORRECTION_TIME_US (14000000ULL) //to be found experimentally. Maybe it varies for each different ESP8266 chip

#define ACDC_TH_VOLTAGE 4.5
#define OFF_TH_VOLTAGE 3

#define SAMPLE_PERIOD_US (5*60*1000000ULL)
static_assert(SAMPLE_PERIOD_US%300000000ULL == 0, "SAMPLE_PERIOD_US must be multiple of 5min!");
//#define CONF_UPDATE_TIME_US (10*60*1000000ULL)
//static_assert(SAMPLE_PERIOD_US >= CONF_UPDATE_TIME_US, "Conf. time cannot be greater than the time between two samples!");

//NumCH & WriteAPI
//Mario46 + Cucina46: 998086, QZL25Q0X3LWQLYKH
//Sabina46 + Appartamento48: 998096, TZD96RGWP4QNZM41
//Nonno18: 1007621, 7DOVMP5V15NZPH9P

//Default: 1, Cucina46: 4, Appartamento48: 4
#define FIRST_FIELD 4

//----------------  Fill in your credentials   ---------------------
#define WLAN_SSID "Infostrada-852489"             // your network SSID (name) 
#define WLAN_PASSWD "Dizzymoon262!"               // your network password
//Thingiverse credentials
unsigned long myChannelNumber = 998086;           // Replace the 0 with your channel number
const char *myWriteAPIKey = "QZL25Q0X3LWQLYKH";   // Paste your ThingSpeak Write API Key between the quotes 
//------------------------------------------------------------------

//==============================================================

typedef struct {
  uint64_t sensor_state_time = 0; //due to program code it must be before crc32 field!
  uint32_t crc32 = 0;
  uint8_t ch;
  uint8_t ap_mac[6];
  //uint8_t sensor_state[4*((BSEC_MAX_STATE_BLOB_SIZE-1)/4+1)] = {0}; //used only for CRC32 in HTU21D
  uint8_t sensor_state[BSEC_MAX_STATE_BLOB_SIZE] = {0}; //used only for CRC32 in HTU21D
  uint64_t cntSleepTime_us = SAMPLE_PERIOD_US;
  uint32_t fwVersion = 0;
  //uint32_t enableConf = 1;
  //uint8_t padding; //find it increasing until the vector until the compiler error disappear
} rtc;

static_assert(sizeof(rtc)%4 == 0, "sizeof(rtc) must be multiple of 4 bytes!");
static_assert(sizeof(rtc)<=512, "RTC structure too big! RTC memory size is 512 bytes.");