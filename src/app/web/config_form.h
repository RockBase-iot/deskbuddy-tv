#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "config/app_config.h"

String renderConfigForm(const AppConfig &cfg, const char *message = "");
void applyFormToConfig(AsyncWebServerRequest *req, AppConfig &cfg);

