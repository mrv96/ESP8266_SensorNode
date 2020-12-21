#pragma once

typedef struct {
  float sampleTime;
  float param2;
} WebServerConfigs;

void handleRoot();
void handleForm();

// =============== private functions ====================
void loadConfiguration(const char *filename, WebServerConfigs &config);
void saveConfiguration(const char *filename, const WebServerConfigs &config);
void printConfigFile(const char *filename);