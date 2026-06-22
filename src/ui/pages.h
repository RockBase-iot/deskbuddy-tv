#pragma once

#include <TFT_eSPI.h>
#include "app/weather/weather_data.h"
#include "config/app_config.h"

enum class Page : uint8_t {
    Eyes = 0,
    Clock,
    CurrentWeather,
    Forecast,
    HourlyGraph,
    AirQuality,
    Count
};

enum class EyeExpression : uint8_t {
    Normal = 0,
    Round,
    Heart,
    Star,
    Sleep,
    Angry,
};

class Pages {
public:
    explicit Pages(TFT_eSPI &display) : _tft(display), _eyeSprite(&display) {}
    void begin();
    void drawStartupSplash(const AppConfig &cfg);
    void cycleAccentSplash(const AppConfig &cfg);
    void markDirty() {
        _lastPage = Page::Count;
        _statusBarDirty = true;
        _indicatorDirty = true;
    }
    void draw(Page page, const AppConfig &cfg, const WeatherData &weather,
              const AirQualityData &aqi,
              float eyeAngle, bool needsRedraw = false);

private:
    TFT_eSPI &_tft;
    TFT_eSprite _eyeSprite;
    Page _lastPage = Page::Count;
    uint32_t _lastInfoDrawMs = 0;
    uint32_t _nextBlinkMs = 0;
    uint32_t _blinkUntilMs = 0;

    String _statusBarCity;
    String _statusBarTime;
    uint16_t _statusBarColor = TFT_BLACK;
    bool _statusBarDirty = true;
    Page _indicatorPage = Page::Count;
    uint16_t _indicatorColor = TFT_BLACK;
    bool _indicatorDirty = true;

    uint16_t accent(const AppConfig &cfg) const;
    float celsiusToDisplay(const AppConfig &cfg, float celsius) const;
    const char *tempUnit(const AppConfig &cfg) const;
    String formatDate(const AppConfig &cfg, const tm &info) const;
    String formatWeekday(const tm &info) const;
    void clearIfChanged(Page page);
    void clearContent();
    void drawHeader(const String &title, const AppConfig &cfg);
    void drawPageTitle(const String &title, const AppConfig &cfg);
    void drawStatusBar(Page page, const AppConfig &cfg);
    void drawPageIndicator(Page page, const AppConfig &cfg);
    bool shouldDrawInfo(Page page, bool needsRedraw, uint32_t intervalMs);
    void drawEyes(const AppConfig &cfg, float angle);
    void drawExpressionPupil(TFT_eSprite &sprite, EyeExpression expression,
                             int16_t cx, int16_t cy, uint16_t color);
    void drawHeartPupil(TFT_eSprite &sprite, int16_t cx, int16_t cy, uint16_t color);
    void drawStarPupil(TFT_eSprite &sprite, int16_t cx, int16_t cy, uint16_t color);
    void drawHeart(TFT_eSprite &sprite, int16_t cx, int16_t cy, int16_t size, uint16_t color);
    void drawStar(TFT_eSprite &sprite, int16_t cx, int16_t cy, int16_t radius, uint16_t color);
    void drawSleepEye(TFT_eSprite &sprite, int16_t x, int16_t y, uint16_t color, float sleepPhase);
    void drawSleepZ(TFT_eSprite &sprite, int16_t x, int16_t y, float sleepPhase, uint16_t color);
    void drawAngryBrow(TFT_eSprite &sprite, int16_t x, int16_t y, bool left, uint16_t color);
    void drawClock(const AppConfig &cfg);
    void drawCurrentWeather(const AppConfig &cfg, const WeatherData &weather);
    void drawForecastIcon(int code, int16_t x, int16_t y, uint16_t color);
    void drawForecast(const AppConfig &cfg, const WeatherData &weather);
    void drawHourlyGraph(const AppConfig &cfg, const WeatherData &weather);
    void drawAirQuality(const AppConfig &cfg, const AirQualityData &aqi);
};
