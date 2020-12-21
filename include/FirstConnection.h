#pragma once

#include "NetConf.h"

bool setupSpiffs(NetConfigSetting *conf);
void saveConfigCallback ();
void routineWM(uint8 staticNodeNetPosition);