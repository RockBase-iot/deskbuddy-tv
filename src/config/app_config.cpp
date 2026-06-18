#include "app_config.h"

#include <Preferences.h>
#include "nvs_table.h"

static const char *PREF_NAMESPACE = "deskbuddy";

void loadAppConfig(AppConfig &cfg) {
    Preferences prefs;
    prefs.begin(PREF_NAMESPACE, true);
    cfg.wifiSsid = prefs.getString(NVS_KEY_WIFI_SSID, cfg.wifiSsid);
    cfg.wifiPassword = prefs.getString(NVS_KEY_WIFI_PASSWORD, cfg.wifiPassword);
    cfg.city = prefs.getString(NVS_KEY_CITY, cfg.city);
    cfg.latitude = prefs.getDouble(NVS_KEY_LAT, cfg.latitude);
    cfg.longitude = prefs.getDouble(NVS_KEY_LON, cfg.longitude);
    cfg.timezoneOffsetMinutes = prefs.getInt(NVS_KEY_TIMEZONE, cfg.timezoneOffsetMinutes);
    cfg.eyeColorIndex = prefs.getUChar(NVS_KEY_EYE_COLOR, cfg.eyeColorIndex);
    cfg.colorCycleMode = prefs.getBool(NVS_KEY_COLOR_CYCLE, cfg.colorCycleMode);
    cfg.customColor565 = prefs.getUShort(NVS_KEY_CUSTOM_COLOR, cfg.customColor565);
    cfg.roundEyeMode = prefs.getBool(NVS_KEY_ROUND_EYE, cfg.roundEyeMode);
    cfg.unitsTemp = prefs.getString(NVS_KEY_UNITS_TEMP, cfg.unitsTemp);
    cfg.unitsSpeed = prefs.getString(NVS_KEY_UNITS_SPEED, cfg.unitsSpeed);
    cfg.unitsPres = prefs.getString(NVS_KEY_UNITS_PRES, cfg.unitsPres);
    cfg.unitsDist = prefs.getString(NVS_KEY_UNITS_DIST, cfg.unitsDist);
    cfg.unitsPrecip = prefs.getString(NVS_KEY_UNITS_PRECIP, cfg.unitsPrecip);
    cfg.timeFormat = prefs.getString(NVS_KEY_TIME_FORMAT, cfg.timeFormat);
    cfg.dateFormat = prefs.getString(NVS_KEY_DATE_FORMAT, cfg.dateFormat);
    cfg.language = prefs.getString(NVS_KEY_LANGUAGE, cfg.language);
    prefs.end();
}

void saveAppConfig(const AppConfig &cfg) {
    Preferences prefs;
    prefs.begin(PREF_NAMESPACE, false);
    prefs.putString(NVS_KEY_WIFI_SSID, cfg.wifiSsid);
    prefs.putString(NVS_KEY_WIFI_PASSWORD, cfg.wifiPassword);
    prefs.putString(NVS_KEY_CITY, cfg.city);
    prefs.putDouble(NVS_KEY_LAT, cfg.latitude);
    prefs.putDouble(NVS_KEY_LON, cfg.longitude);
    prefs.putInt(NVS_KEY_TIMEZONE, cfg.timezoneOffsetMinutes);
    prefs.putUChar(NVS_KEY_EYE_COLOR, cfg.eyeColorIndex);
    prefs.putBool(NVS_KEY_COLOR_CYCLE, cfg.colorCycleMode);
    prefs.putUShort(NVS_KEY_CUSTOM_COLOR, cfg.customColor565);
    prefs.putBool(NVS_KEY_ROUND_EYE, cfg.roundEyeMode);
    prefs.putString(NVS_KEY_UNITS_TEMP, cfg.unitsTemp);
    prefs.putString(NVS_KEY_UNITS_SPEED, cfg.unitsSpeed);
    prefs.putString(NVS_KEY_UNITS_PRES, cfg.unitsPres);
    prefs.putString(NVS_KEY_UNITS_DIST, cfg.unitsDist);
    prefs.putString(NVS_KEY_UNITS_PRECIP, cfg.unitsPrecip);
    prefs.putString(NVS_KEY_TIME_FORMAT, cfg.timeFormat);
    prefs.putString(NVS_KEY_DATE_FORMAT, cfg.dateFormat);
    prefs.putString(NVS_KEY_LANGUAGE, cfg.language);
    prefs.end();
}
