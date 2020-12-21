#include <Arduino.h>
#include <LittleFS.h>    // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include "NetConf.h"
#include "FirstConnection.h"

bool shouldSaveConfig = false;

bool setupSpiffs(NetConfigSetting *conf) {
  //clean FS, for testing
  // SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (LittleFS.begin()) {
    Serial.println("mounted file system");
    if (LittleFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument jsonDoc(1024);
        auto error = deserializeJson(jsonDoc, buf.get());
        serializeJson(jsonDoc, Serial);
        if (!error) {
          Serial.println("\nparsed json");

           if(jsonDoc["ip"]) {            
             Serial.println("setting custom ip from config");

             if(!(conf->ip.fromString((const char*) jsonDoc["ip"]))) {
               return false;
             }
             if(!(conf->gateway.fromString((const char*) jsonDoc["gateway"]))) {
               return false;
             }
             if(!(conf->netmask.fromString((const char*) jsonDoc["subnet"]))) {
               return false;
             }
             if(!(conf->dns1.fromString((const char*) jsonDoc["dns1"]))) {
               return false;
             }
             if(!(conf->dns2.fromString((const char*) jsonDoc["dns2"]))) {
               if(strcmp_P((const char*) jsonDoc["dns2"], PSTR("(IP unset)")) == 0) conf->dns2 = conf->dns1;
               else return false;
             }

             conf->ssid = String((const char*) jsonDoc["ssid"]);
             conf->pwd = String((const char*) jsonDoc["pwd"]);

             Serial.println(conf->ip);

             return true;
           } else {
             Serial.println("no custom ip in config");
           }

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  return false;
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void routineWM(uint8 staticNodeNetPosition) {
  //wifi connection setup: load the sketch and then connect to Node_Setup_WiFi network
    WiFiManager wifiManager;
    
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.resetSettings();
    wifiManager.autoConnect("Node_Setup_WiFi");

    uint32_t defaultIP = uint32_t(WiFi.gatewayIP())&uint32_t(WiFi.subnetMask());
    defaultIP = defaultIP + (staticNodeNetPosition<<__builtin_popcount(uint32_t(WiFi.subnetMask())));
  
    WiFi.config(IPAddress(defaultIP), WiFi.gatewayIP(), WiFi.subnetMask(),WiFi.dnsIP(0),WiFi.dnsIP(1));
    WiFi.reconnect();

    //save the custom parameters to FS
    if (shouldSaveConfig) {
      Serial.println("saving config");
      DynamicJsonDocument jsonDoc(1024);
  
      jsonDoc["ip"] = WiFi.localIP().toString();
      jsonDoc["gateway"] = WiFi.gatewayIP().toString();
      jsonDoc["subnet"] = WiFi.subnetMask().toString();
      jsonDoc["dns1"] = WiFi.dnsIP(0).toString();
      jsonDoc["dns2"] = WiFi.dnsIP(1).toString();
      jsonDoc["ssid"] = WiFi.SSID();
      jsonDoc["pwd"] = WiFi.psk();
  
      File configFile = LittleFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }

      serializeJsonPretty(jsonDoc, Serial);
      serializeJson(jsonDoc, configFile);
      configFile.close();
      //end save
      shouldSaveConfig = false;
    }
}