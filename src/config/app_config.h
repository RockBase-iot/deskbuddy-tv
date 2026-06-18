#pragma once

#include <Arduino.h>

struct AppConfig {
    String wifiSsid;
    String wifiPassword;
    String city = "Shanghai";
    double latitude = 31.2304;
    double longitude = 121.4737;
    int timezoneOffsetMinutes = 480;
    uint8_t eyeColorIndex = 0;
    bool colorCycleMode = true;
    uint16_t customColor565 = 0x07E0;
    bool roundEyeMode = false;
    String unitsTemp = "celsius";
    String unitsSpeed = "kmh";
    String unitsPres = "hpa";
    String unitsDist = "km";
    String unitsPrecip = "mm";
    String timeFormat = "24h";
    String dateFormat = "yyyy-mm-dd";
    String language = "en";
};

void loadAppConfig(AppConfig &cfg);
void saveAppConfig(const AppConfig &cfg);
