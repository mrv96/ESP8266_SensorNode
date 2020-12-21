#include "main.h"

//#define PETOK
#define MARIO

#ifdef PETOK

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include "WebServer.h"
#include "FirstConnection.h"
#include "NetConf.h"

//==============================================================

ESP8266WebServer server(80); //Server on port 80

//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);

  NetConfigSetting conf;

  if(setupSpiffs(&conf) == false) {
    routineWM(NODE_DEFAULT_ADDRESS);
  } else {
    WiFi.disconnect();

    WiFi.mode(WIFI_STA);

    WiFi.config(conf.ip, conf.gateway, conf.netmask, conf.dns1, conf.dns2);
    WiFi.begin(conf.ssid,conf.pwd);

    uint32_t cnt = 0;
    while (WiFi.status() != WL_CONNECTED/* && cnt < TIMEOUT_MS*/)
    {
      cnt += TIMEOUT_STEP_MS;
      delay(TIMEOUT_STEP_MS);
      Serial.print(".");
    }

    if(WiFi.status() != WL_CONNECTED) {
      routineWM(NODE_DEFAULT_ADDRESS);
    }
  }
  
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  if (!MDNS.begin(MDNS_NAME))
  { // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  } else {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started");
  }

  server.on("/", handleRoot);      //Which routine to handle at root location
  server.on("/action_page", handleForm); //form action is handled here

  server.begin();                  //Start server
  
  //Serial.println("HTTP server started");

  
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();          //Handle client requests
  MDNS.update();
}

#elif defined(MARIO)

//RF_DEFAULT = 0, // RF_CAL or not after deep-sleep wake up, depends on init data byte 108.
//RF_CAL = 1,      // RF_CAL after deep-sleep wake up, there will be large current.
//RF_NO_CAL = 2,   // no RF_CAL after deep-sleep wake up, there will only be small current.
//RF_DISABLED = 4 // disable RF after deep-sleep wake up, just like modem sleep, there will be the smallest current.
//#define WAKE_RF_DEFAULT  RF_DEFAULT
//#define WAKE_RFCAL       RF_CAL
//#define WAKE_NO_RFCAL    RF_NO_CAL
//#define WAKE_RF_DISABLED RF_DISABLED

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ThingSpeak.h>
#include "SensorUtils.h"

rtc rtcData;

uint16_t crcDataLength = sizeof(rtcData)-(((uint8_t*) &rtcData.crc32)+sizeof(rtcData.crc32)-((uint8_t*) &rtcData));

#ifdef BME680_SENSOR
#include <sys/time.h>
#include <bsec.h>
#include "BMEConfig.h"

Bsec sensor;

bsec_virtual_sensor_t sensor_list[] = {
  BSEC_OUTPUT_RAW_TEMPERATURE,
  BSEC_OUTPUT_RAW_PRESSURE,
  BSEC_OUTPUT_RAW_HUMIDITY,
  BSEC_OUTPUT_RAW_GAS,
  BSEC_OUTPUT_IAQ,
  BSEC_OUTPUT_STATIC_IAQ,
  BSEC_OUTPUT_CO2_EQUIVALENT,
  BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
  BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
  BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
};

#else

#include <Wire.h>
#include <HTU21D.h>
#include "HTUConfig.h"

/*
HTU21D(resolution)
resolution:
HTU21D_RES_RH12_TEMP14 - RH: 12Bit, Temperature: 14Bit, by default
HTU21D_RES_RH8_TEMP12  - RH: 8Bit,  Temperature: 12Bit
HTU21D_RES_RH10_TEMP13 - RH: 10Bit, Temperature: 13Bit
HTU21D_RES_RH11_TEMP11 - RH: 11Bit, Temperature: 11Bit
*/
HTU21D myHTU21D(HTU21D_RES_RH12_TEMP14);

#endif

void connectWiFi(rtc *rtcData, bool rtcValid) {
  //Switch Radio back On
  WiFi.forceSleepWake();
  delay(1);

  // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings unnecessarily in the flash memory.
  WiFi.persistent(false);

  // Bring up the WiFi connection
  WiFi.mode( WIFI_STA );
  //WiFi.config( ip,dns, gateway, subnet ); //STATIC CONFIG
  //-----------Now we replace the normally used "WiFi.begin();" with a precedure using connection data stored by us
  if(rtcValid) {
    // The RTC data was good, make a quick connection
    WiFi.begin(WLAN_SSID, WLAN_PASSWD, rtcData->ch, rtcData->ap_mac, true);
  } else {
    // The RTC data was not valid, so make a regular connection
    WiFi.begin(WLAN_SSID, WLAN_PASSWD);
  }

  //------now wait for connection
  int retries = 0;
  int wifiStatus = WiFi.status();
  while( wifiStatus != WL_CONNECTED ) { //look also WiFi.waitForConnectResult()
    retries++;
    if( retries == 100 ) {
      // Quick connect is not working, reset WiFi and try regular connection
      rtcValid = false;
      WiFi.disconnect();
      delay(10);
      WiFi.forceSleepBegin();
      delay(10);
      WiFi.forceSleepWake();
      delay(10);
      WiFi.begin(WLAN_SSID, WLAN_PASSWD);
    }
    if(retries == 600) {
      // Giving up after n seconds and going back to sleep
      WiFi.disconnect( true );
      delay(1);
      WiFi.mode( WIFI_OFF );
      ESP.deepSleep(5e6);
      return; // Not expecting this to be called, the previous call will never return.
    }
    delay(50);
    wifiStatus = WiFi.status();
  }

  if(!rtcValid) {
    // Write current connection info back to RTC
    rtcData->ch = WiFi.channel();
    memcpy( rtcData->ap_mac, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
    rtcData->crc32 = calculateCRC32(((uint8_t*) &rtcData->crc32)+sizeof(rtcData->crc32), crcDataLength);
    ESP.rtcUserMemoryWrite(0, (uint32_t*) rtcData, sizeof(rtc));
  }
}

// Save boot up time by not configuring them if they haven't changed
//if(WiFi.SSID() != ssid) {
//  Serial.println(F("Initialising Wifi..."));
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, pass);
//  WiFi.persistent(true);
//  WiFi.setAutoConnect(true);
//  WiFi.setAutoReconnect(true);
//}
//while(WiFi.waitForConnectResult() != WL_CONNECTED) {
//  Serial.println("Connection Failed! Rebooting...");
//  ESP.deepSleep(5e6); //restart after 5 sec
//}

void handleInterrupt() {
  uint64_t tmpTime, initTime = millis();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  while((tmpTime=millis())-initTime<FACTORY_RESET_TIME_MS && (digitalRead(BUTTON_PIN)==LOW))
    yield();

  if(tmpTime-initTime >= FACTORY_RESET_TIME_MS) {
    //TODO: factoryReset();
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void setup() {
  // we disable WiFi, coming from DeepSleep, as we do not need it right away
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(1);
  
  Serial.begin(115200);
  Serial.println("\nBooting");

  //float battery = ((R1+R2)/R2)*(analogRead(A0)/1023);
  float battery = (585.0/115.0)*(analogRead(A0)/1023.0);

  if(battery >= ACDC_TH_VOLTAGE) {
    pinMode(BUTTON_PIN, INPUT); //Pullup not needed because external resistor
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);
  }

  // Read struct from RTC memory
  uint32_t crcOfData;
  bool rtcValid = false;

  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    crcOfData = calculateCRC32(((uint8_t*) &rtcData.crc32)+sizeof(rtcData.crc32), crcDataLength);
    Serial.print("CRC32 of data: ");
    Serial.println(crcOfData, HEX);
    Serial.print("CRC32 read from RTC: ");
    Serial.println(rtcData.crc32, HEX);
    if (crcOfData != rtcData.crc32) {
      Serial.println("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!");
    } else {
      Serial.println("CRC32 check ok, data is probably valid.");
      rtcValid = true;
    }
  }//else ERRORE!!!

  uint32_t currentFw = fwVersionToUINT32((char*) FW_VERSION);
  if(rtcData.fwVersion != currentFw) {
    rtcValid = false;
    rtcData.fwVersion = currentFw;
  }

  //If crc32 not valid reset the struct to default values
  if(!rtcValid) {
    rtcData.sensor_state_time = 0;
    rtcData.cntSleepTime_us = SAMPLE_PERIOD_US;
//    rtcData.enableConf = 1;
  }

//  bool enableConf = rtcData.enableConf;
//  if(battery < ACDC_TH_VOLTAGE && !enableConf)
//    rtcData.enableConf = 1;
//  else if(battery >= ACDC_TH_VOLTAGE && enableConf)
//    rtcData.enableConf = 0;

  uint64_t intermediateTime = 0;

#ifdef BME680_SENSOR
  
  SPI.begin();
  sensor.begin(CS_PIN, SPI);
  if (!CheckSensor(sensor)) {
    //LOG("Failed to init BME680, check wiring!");
    ESP.deepSleep(5e6);
    return;
  }

  //LOG("BSEC version %d.%d.%d.%d", sensor.version.major, sensor.version.minor, sensor.version.major_bugfix, sensor.version.minor_bugfix);

  sensor.setConfig(bsec_config_iaq);
  if (!CheckSensor(sensor)) {
    //LOG("Failed to set config!");
    ESP.deepSleep(5e6);
    return;
  }

  if(rtcValid) {
    sensor.setState(rtcData.sensor_state);
    if (!CheckSensor(sensor)) {
      //LOG("Failed to set state!");
      ESP.deepSleep(5e6);
      return;
    } else {
      LOG("Successfully set state, CRC32: %u", rtcData.crc32);
    }
  } else {
    LOG("Saved state missing");
    memset(rtcData.sensor_state,0,sizeof(rtcData.sensor_state)); //actually needed?
  }

  sensor.updateSubscription(sensor_list, sizeof(sensor_list) / sizeof(sensor_list[0]), BSEC_SAMPLE_RATE_ULP);
  if (!CheckSensor(sensor)) {
    //LOG("Failed to update subscription!");
    ESP.deepSleep(5e6);
    return;
  }

  float data[N_FIELDS];
  
  intermediateTime = micros();

  /******** GET DATA *******/
  if(sensor.run((rtcData.sensor_state_time += intermediateTime)/1000)) {
    LOG("Temperature raw %.2f compensated %.2f", sensor.rawTemperature, sensor.temperature);
    LOG("Humidity raw %.2f compensated %.2f", sensor.rawHumidity, sensor.humidity);
    LOG("Pressure %.2f kPa", sensor.pressure / 1000);
    LOG("IAQ %.0f accuracy %d", sensor.iaq, sensor.iaqAccuracy);
    LOG("Static IAQ %.0f accuracy %d", sensor.staticIaq, sensor.staticIaqAccuracy);
    LOG("Gas resistance %.2f kOhm", sensor.gasResistance / 1000);

    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.
//    int setField(field, value);
//    int setStatus(status);
//    int setLatitude(latitude);
//    int setLongitude(longitude);
//    int setElevation(elevation);
//    int setCreatedAt(createdAt);

    data[0] = sensor.temperature;
    data[1] = sensor.humidity;
    data[2] = sensor.pressure / 100;
    data[3] = ((int)(sensor.iaq+0.5))+((sensor.iaqAccuracy+1)%4)/10.0; //in questo modo se accuracy = 3 ho solo valori interi, i decimali indicano iaqAccuracy+1 (N.B. se accuracy = 3 IAQ avrebbe dei decimali che arrotondo all'intero più vicino)
    data[4] = battery;
    
    sensor.getState(rtcData.sensor_state);
    CheckSensor(sensor); //Only useful to notify error/warnings through serial
  } else {
    LOG("Failed to get data!");
    ESP.deepSleep(5e6);
    return;
  }

  uint64_t sleepTime_us = sensor.nextCall*1000 - rtcData.sensor_state_time + intermediateTime; //updated only inside run() method
  LOG("%llu: %llu", rtcData.sensor_state_time, sensor.nextCall);

  if(rtcData.cntSleepTime_us >= SAMPLE_PERIOD_US) rtcData.cntSleepTime_us = sleepTime_us;
  else rtcData.cntSleepTime_us += sleepTime_us;

  rtcData.crc32 = calculateCRC32(((uint8_t*) &rtcData.crc32)+sizeof(rtcData.crc32), crcDataLength);
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));
  //Serial.println(F("Saved state to RTC memory at %lld", rtcData.crc32));

#else

  if(!rtcValid) {
    rtcData.crc32 = calculateCRC32(((uint8_t*) &rtcData.crc32)+sizeof(rtcData.crc32), crcDataLength);
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));
  }

  if(myHTU21D.begin(SDA_PIN, SCL_PIN) != true) {
    //Serial.println(F("HTU21D, SHT21 sensor is faild or not connected"));
    ESP.deepSleep(5e6);
    return;
  }
  //Serial.println(F("HTU21D, SHT21 sensor is active"));

  float data[] = {myHTU21D.readTemperature(), myHTU21D.readCompensatedHumidity(), battery};

  if(data[0] == HTU21D_ERROR || data[1] == HTU21D_ERROR) {
    //Serial.println(F("I2C communication error happened."));
    ESP.deepSleep(5e6);
    return;
  }

  uint64_t sleepTime_us = SAMPLE_PERIOD_US;

#endif

  if(battery >= ACDC_TH_VOLTAGE) {
    connectWiFi(&rtcData, rtcValid);
  }

  if(rtcData.cntSleepTime_us-sleepTime_us == 0) {
    if(battery < ACDC_TH_VOLTAGE) {
      connectWiFi(&rtcData, rtcValid);
    }

    WiFiClient  client;
    ThingSpeak.begin(client);
    int x;

    if((x=set_fields(FIRST_FIELD, sizeof(data)/sizeof(data[0]), data)) == 200) {
      if((x=ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey)) != 200) {
        Serial.println("QUI2");
        Serial.println("Problem updating channel. HTTP error code " + String(x));
        ESP.deepSleep(5e6);
        return;
      }
    } else Serial.println("ThingSpeak error code " + String(x));
  }

  //Check for remote updates
//  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.0.2", 80, "/esp/update/arduino.php", "optional current version string here");
//  switch(ret) {
//      case HTTP_UPDATE_FAILED:
//          //Serial.println("[update] Update failed.");
//          break;
//      case HTTP_UPDATE_NO_UPDATES:
//          //Serial.println("[update] Update no Update.");
//          break;
//      case HTTP_UPDATE_OK:
//          //Serial.println("[update] Update ok."); // may not be called since we reboot the ESP
//          break;
//  }

//PROBLEMA: autoriscaldamento della scheda
//SOLUZIONE2: web server per update. No config o possibile gestione di account per configurazione
//SOLUZIONE3: stare attivo solo tot tempo per config e update, poi tornare a fare normalmente
  

  if(battery >= ACDC_TH_VOLTAGE) {
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);
  
    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");
  
    // No authentication by default
    // ArduinoOTA.setPassword("admin");
  
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }
  
      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      //Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      //Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      //Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        //Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        //Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        //Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        //Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        //Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
  
    uint32_t crc_bak = rtcData.crc32;
    uint64_t initTime = micros();//variable to take in account the time used for the second rtcWrite
    
    rtcData.crc32 = 0;
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData.crc32, sizeof(rtcData.crc32));

    sleepTime_us = sleepTime_us - (micros() - initTime);
    
    while(micros() < sleepTime_us) {
      ArduinoOTA.handle();
      yield();//give the system a chance to execute
    }

    //Servono?
    WiFi.disconnect(true);
    delay(1);

    rtcData.crc32 = crc_bak;
    rtcData.sensor_state_time += micros() - intermediateTime;
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData.sensor_state_time, sizeof(rtcData.sensor_state_time) + sizeof(rtcData.crc32));
    
    ESP.restart();
  } else if(battery <= OFF_TH_VOLTAGE) {
    
    rtcData.crc32 = 0;
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData.crc32, sizeof(rtcData.crc32));

    //Servono?
    WiFi.disconnect(true);
    delay(1);
    
    ESP.deepSleep(0); //sleep until a reset occurs (in theory cause power supply plug)
    
  } else if(micros()+100 < sleepTime_us) { //avoid a possible "overflow" of time and a deepSleep shorter than 100uS (extimated execution time of the instructions inside the if)
    
    //LOG("Deep sleep for %llu ms. BSEC next call at %llu ms.", (sleepTime_us-micros())/1000, sensor.nextCall);

    rtcData.sensor_state_time += sleepTime_us - intermediateTime; //sleepTime_us - intermediateTime = micros() - intermediateTime + sleepTime_us - micros()
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData.sensor_state_time, sizeof(rtcData.sensor_state_time));

    //Servono?
    WiFi.disconnect(true);
    delay(1);

    ESP.deepSleep(sleepTime_us - micros() + DEEPSLEEP_CORRECTION_TIME_US);
    
  } else {
    rtcData.sensor_state_time += micros() - intermediateTime;
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData.sensor_state_time, sizeof(rtcData.sensor_state_time));

    //Servono?
    WiFi.disconnect(true);
    delay(1);
    
    ESP.restart();
  }
}

void loop() {}

//TODO: rename sensor_state_time

#endif
