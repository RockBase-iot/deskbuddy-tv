#pragma once

#include <Arduino.h>
#include <vector>

#define WEATHER_API_HOST     "api.open-meteo.com"
#define AQI_API_HOST         "air-quality-api.open-meteo.com"
#define WEATHER_HTTP_TIMEOUT 4500

struct WeatherCurrent {
    float temperature = NAN;
    float apparent_temperature = NAN;
    float humidity = NAN;
    float wind_speed = NAN;
    float wind_direction = NAN;
    float pressure = NAN;
    float visibility = NAN;
    int weather_code = -1;
    bool is_day = true;
};

struct WeatherHourly {
    String time;
    float temperature = NAN;
    int weather_code = -1;
    int precipitation_probability = 0;
    float precipitation = 0.0f;
    int humidity = 0;
};

struct WeatherDaily {
    String date;
    int weather_code = -1;
    float temp_max = NAN;
    float temp_min = NAN;
    int precipitation_probability_max = 0;
    float precipitation_sum = 0.0f;
    float uv_index_max = NAN;
    String sunrise;
    String sunset;
};

struct WeatherData {
    WeatherCurrent current;
    std::vector<WeatherHourly> hourly;
    std::vector<WeatherDaily> daily;
    String timezone;
    float elevation = NAN;
    uint32_t last_update_ms = 0;
    bool valid = false;
};

struct AirQualityHourly {
    String time;
    int us_aqi = 0;
    float pm2_5 = NAN;
    float pm10 = NAN;
};

struct AirQualityData {
    float pm2_5 = NAN;
    float pm10 = NAN;
    int us_aqi = 0;
    int european_aqi = 0;
    float carbon_monoxide = NAN;
    float nitrogen_dioxide = NAN;
    float ozone = NAN;
    std::vector<AirQualityHourly> hourly;
    uint32_t last_update_ms = 0;
    bool valid = false;
};


