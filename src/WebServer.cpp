#include <ESP8266WebServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "web_pages.h"

const char *filename = "/web_server_config.txt";  // <- SD library uses 8.3 filenames

// This cannot be passed as parameters inside functions because they are callback functions
extern ESP8266WebServer server;

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
  WebServerConfigs config;
  
  char html[sizeof(MAIN_page)+10000]; //NOTE: sizeof(html) > sizeof(MAIN_page)
  
  //Serial.println(sizeof(MAIN_page));

  loadConfiguration(filename, config);
  
  snprintf_P(html, sizeof(html), MAIN_page,config.param2,config.param2, config.sampleTime, config.param2);

   server.send_P(200, "text/html", html);
}

//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
  WebServerConfigs config;

  String sampleTime = server.arg("sampleTime");
  String par2_in = server.arg("param2");

  Serial.print("Sample time: ");
  Serial.println(config.sampleTime = sampleTime.toFloat());

  Serial.print("parameter 2: ");
  Serial.println(config.param2 = par2_in.toFloat());

  saveConfiguration(filename, config);
  printConfigFile(filename);

  // WebServerConfigs config2;
  // loadConfiguration(filename, config2);
  // if(config != config2) ERRORE!;

  server.send_P(200, "text/html", SEND_page_OK);

  // TODO: inserire pagina di fallimento se qualcosa va storto
  //if (wrong)
  // server.send_P(200, "text/html", SEND_page_FAILURE);
}

//=======================================================================

// Loads the configuration from a file
void loadConfiguration(const char *filename, WebServerConfigs &config) {

  // Initialize LittleFS filesystem library
  while (!LittleFS.begin())
  {
    Serial.println(F("Failed to initialize SD library"));
    delay(1000);
  }

  // Open file for reading
  File file = LittleFS.open(filename, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  config.sampleTime = doc["sample_time"] | 123;
  config.param2 = doc["param2"] | 456;
  // strlcpy(config.hostname,                  // <- destination
  //         doc["hostname"] | "example.com",  // <- source
  //         sizeof(config.hostname));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
  LittleFS.end();
}

// Saves the configuration to a file
void saveConfiguration(const char *filename, const WebServerConfigs &config) {
  // Initialize LittleFS filesystem library
  while (!LittleFS.begin())
  {
    Serial.println(F("Failed to initialize SD library"));
    delay(1000);
  }

  // Open file for writing
  File file = LittleFS.open(filename, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Set the values in the document
  doc["sample_time"] = config.sampleTime;
  doc["param2"] = config.param2;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
  LittleFS.end();
}

// Prints the content of a file to the Serial
void printConfigFile(const char *filename) {
  // Initialize LittleFS filesystem library
  while (!LittleFS.begin())
  {
    Serial.println(F("Failed to initialize SD library"));
    delay(1000);
  }

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
  LittleFS.end();
}