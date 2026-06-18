#include "weather.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "utils/logger.h"

static const char *TAG = "Weather";

bool WeatherClass::fetchWeather(double lat, double lon) {
    char url[640];
    snprintf(url, sizeof(url),
        "http://" WEATHER_API_HOST "/v1/forecast"
        "?latitude=%.4f&longitude=%.4f"
        "&current=temperature_2m,apparent_temperature,relative_humidity_2m,"
                  "wind_speed_10m,wind_direction_10m,surface_pressure,"
                  "weather_code,is_day,visibility"
        "&hourly=temperature_2m,weather_code,precipitation_probability,precipitation,relative_humidity_2m"
        "&daily=weather_code,temperature_2m_max,temperature_2m_min,"
               "precipitation_probability_max,precipitation_sum,sunrise,sunset,uv_index_max"
        "&forecast_days=5&forecast_hours=24&timezone=auto",
        lat, lon);

    HTTPClient http;
    http.begin(url);
    http.setTimeout(WEATHER_HTTP_TIMEOUT);
    int code = http.GET();
    if (code != 200) {
        log_e(TAG, "HTTP %d for weather request", code);
        http.end();
        return false;
    }

    bool ok = _parseWeatherResponse(http.getString());
    http.end();
    return ok;
}

bool WeatherClass::fetchAirQuality(double lat, double lon) {
    char url[512];
    snprintf(url, sizeof(url),
        "http://" AQI_API_HOST "/v1/air-quality"
        "?latitude=%.4f&longitude=%.4f"
        "&current=pm2_5,pm10,us_aqi,european_aqi,carbon_monoxide,nitrogen_dioxide,ozone"
        "&hourly=us_aqi,pm2_5,pm10"
        "&forecast_days=5&timezone=auto",
        lat, lon);

    HTTPClient http;
    http.begin(url);
    http.setTimeout(WEATHER_HTTP_TIMEOUT);
    int code = http.GET();
    if (code != 200) {
        log_e(TAG, "HTTP %d for AQI request", code);
        http.end();
        return false;
    }

    bool ok = _parseAqiResponse(http.getString());
    http.end();
    return ok;
}

bool WeatherClass::_parseWeatherResponse(const String &body) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        log_e(TAG, "JSON parse error: %s", err.c_str());
        return false;
    }

    JsonObject cur = doc["current"];
    _weather.current.temperature = cur["temperature_2m"] | NAN;
    _weather.current.apparent_temperature = cur["apparent_temperature"] | NAN;
    _weather.current.humidity = cur["relative_humidity_2m"] | NAN;
    _weather.current.wind_speed = cur["wind_speed_10m"] | NAN;
    _weather.current.wind_direction = cur["wind_direction_10m"] | NAN;
    _weather.current.pressure = cur["surface_pressure"] | NAN;
    _weather.current.visibility = cur["visibility"] | NAN;
    _weather.current.weather_code = cur["weather_code"] | -1;
    _weather.current.is_day = (cur["is_day"] | 1) != 0;
    _weather.timezone = doc["timezone"].as<String>();
    _weather.elevation = doc["elevation"] | NAN;

    _weather.daily.clear();
    JsonObject daily = doc["daily"];
    JsonArray dTime = daily["time"];
    JsonArray dCode = daily["weather_code"];
    JsonArray dTempMax = daily["temperature_2m_max"];
    JsonArray dTempMin = daily["temperature_2m_min"];
    JsonArray dPopMax = daily["precipitation_probability_max"];
    JsonArray dRainSum = daily["precipitation_sum"];
    JsonArray dSunrise = daily["sunrise"];
    JsonArray dSunset = daily["sunset"];
    JsonArray dUvIndex = daily["uv_index_max"];
    size_t numDays = dTime.size();
    if (numDays > 5) numDays = 5;
    _weather.daily.reserve(numDays);
    for (size_t i = 0; i < numDays; ++i) {
        WeatherDaily d;
        d.date = dTime[i].as<String>();
        d.weather_code = dCode[i] | -1;
        d.temp_max = dTempMax[i] | NAN;
        d.temp_min = dTempMin[i] | NAN;
        d.precipitation_probability_max = dPopMax[i] | 0;
        d.precipitation_sum = dRainSum[i] | 0.0f;
        d.uv_index_max = dUvIndex[i] | NAN;
        d.sunrise = dSunrise[i].as<String>();
        d.sunset = dSunset[i].as<String>();
        _weather.daily.push_back(d);
    }

    _weather.hourly.clear();
    JsonObject hourly = doc["hourly"];
    JsonArray hTime = hourly["time"];
    JsonArray hTemp = hourly["temperature_2m"];
    JsonArray hCode = hourly["weather_code"];
    JsonArray hPop = hourly["precipitation_probability"];
    JsonArray hRain = hourly["precipitation"];
    JsonArray hHumi = hourly["relative_humidity_2m"];
    size_t numHours = hTime.size();
    if (numHours > 24) numHours = 24;
    _weather.hourly.reserve(numHours);
    for (size_t i = 0; i < numHours; ++i) {
        WeatherHourly h;
        h.time = hTime[i].as<String>();
        h.temperature = hTemp[i] | NAN;
        h.weather_code = hCode[i] | -1;
        h.precipitation_probability = hPop[i] | 0;
        h.precipitation = hRain[i] | 0.0f;
        h.humidity = hHumi[i] | 0;
        _weather.hourly.push_back(h);
    }

    _weather.last_update_ms = millis();
    _weather.valid = true;
    return true;
}

bool WeatherClass::_parseAqiResponse(const String &body) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        log_e(TAG, "AQI JSON parse error: %s", err.c_str());
        return false;
    }

    JsonObject cur = doc["current"];
    _aqi.pm2_5 = cur["pm2_5"] | NAN;
    _aqi.pm10 = cur["pm10"] | NAN;
    _aqi.us_aqi = cur["us_aqi"] | 0;
    _aqi.european_aqi = cur["european_aqi"] | 0;
    _aqi.carbon_monoxide = cur["carbon_monoxide"] | NAN;
    _aqi.nitrogen_dioxide = cur["nitrogen_dioxide"] | NAN;
    _aqi.ozone = cur["ozone"] | NAN;

    _aqi.hourly.clear();
    JsonObject hourly = doc["hourly"];
    JsonArray hTime = hourly["time"];
    JsonArray hUsAqi = hourly["us_aqi"];
    JsonArray hPm25 = hourly["pm2_5"];
    JsonArray hPm10 = hourly["pm10"];
    size_t numHours = hTime.size();
    if (numHours > 24) numHours = 24;
    _aqi.hourly.reserve(numHours);
    for (size_t i = 0; i < numHours; ++i) {
        AirQualityHourly h;
        h.time = hTime[i].as<String>();
        h.us_aqi = hUsAqi[i] | 0;
        h.pm2_5 = hPm25[i] | NAN;
        h.pm10 = hPm10[i] | NAN;
        _aqi.hourly.push_back(h);
    }

    _aqi.last_update_ms = millis();
    _aqi.valid = true;
    return true;
}

