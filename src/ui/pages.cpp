#include "pages.h"

#include <math.h>
#include <time.h>
#include <WiFi.h>
#include "app/weather/wmo_code.h"
#include "version.h"

static const int16_t EYE_W = 80;
static const int16_t EYE_H = 80;
static const int16_t FACE_X = 20;
static const int16_t FACE_Y = 42;
static const int16_t FACE_W = 200;
static const int16_t FACE_H = 132;
static const int16_t LEFT_EYE_X = 10;
static const int16_t RIGHT_EYE_X = 110;
static const int16_t EYE_Y = 28;
static const int16_t PUPIL_SIZE = 36;
static const int16_t SCLERA_PAD = 6;
static const int16_t CONTENT_TOP = 24;
static const int16_t CONTENT_BOTTOM = 224;
static const int16_t CONTENT_LEFT = 8;
static const int16_t CONTENT_RIGHT = 232;

void Pages::begin() {
    _eyeSprite.setColorDepth(16);
    _eyeSprite.createSprite(FACE_W, FACE_H);
    _nextBlinkMs = millis() + 2500;
}

void Pages::drawStartupSplash(const AppConfig &cfg) {
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setTextSize(2);
    _tft.drawString("RockBase IoT", 120, 96);
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    _tft.drawString("DESK-BUDDY", 120, 132);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    _tft.drawString(String("v") + DESKBUDDY_VERSION, 120, 166);
    delay(1500);
}

void Pages::cycleAccentSplash(const AppConfig &cfg) {
    (void)cfg;
    static const uint16_t colors[] = {
        TFT_MAGENTA, TFT_CYAN, TFT_YELLOW, TFT_BLUE, TFT_GREEN,
        TFT_RED, TFT_ORANGE, TFT_PINK, TFT_WHITE
    };
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextSize(3);
    for (uint8_t i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i) {
        _tft.setTextColor(colors[i], TFT_BLACK);
        _tft.drawString("DESK-BUDDY", 120, 120);
        delay(90);
    }
}

uint16_t Pages::accent(const AppConfig &cfg) const {
    static const uint16_t colors[] = {
        TFT_MAGENTA, TFT_CYAN, TFT_YELLOW, TFT_BLUE, TFT_GREEN,
        TFT_RED, TFT_ORANGE, TFT_PINK, TFT_WHITE
    };
    if (!cfg.colorCycleMode) return cfg.customColor565;
    return colors[cfg.eyeColorIndex % (sizeof(colors) / sizeof(colors[0]))];
}

float Pages::celsiusToDisplay(const AppConfig &cfg, float celsius) const {
    if (cfg.unitsTemp == "fahrenheit") return celsius * 9.0f / 5.0f + 32.0f;
    return celsius;
}

const char *Pages::tempUnit(const AppConfig &cfg) const {
    return cfg.unitsTemp == "fahrenheit" ? "F" : "C";
}

String Pages::formatDate(const AppConfig &cfg, const tm &info) const {
    char yyyy[5];
    char mm[3];
    char dd[3];
    strftime(yyyy, sizeof(yyyy), "%Y", &info);
    strftime(mm, sizeof(mm), "%m", &info);
    strftime(dd, sizeof(dd), "%d", &info);

    if (cfg.dateFormat == "dd-mm-yyyy") return String(dd) + "-" + mm + "-" + yyyy;
    if (cfg.dateFormat == "mm-dd-yyyy") return String(mm) + "-" + dd + "-" + yyyy;
    if (cfg.dateFormat == "yyyy-dd-mm") return String(yyyy) + "-" + dd + "-" + mm;
    if (cfg.dateFormat == "dd/mm/yyyy") return String(dd) + "/" + mm + "/" + yyyy;
    if (cfg.dateFormat == "mm/dd/yyyy") return String(mm) + "/" + dd + "/" + yyyy;
    if (cfg.dateFormat == "yyyy/mm/dd") return String(yyyy) + "/" + mm + "/" + dd;
    if (cfg.dateFormat == "yyyy/dd/mm") return String(yyyy) + "/" + dd + "/" + mm;
    return String(yyyy) + "-" + mm + "-" + dd;
}

String Pages::formatWeekday(const tm &info) const {
    char weekday[4];
    strftime(weekday, sizeof(weekday), "%a", &info);
    return String(weekday);
}

void Pages::clearIfChanged(Page page) {
    if (_lastPage != page) {
        _tft.fillScreen(TFT_BLACK);
        _lastPage = page;
        _lastInfoDrawMs = 0;
    }
}

void Pages::clearContent() {
    _tft.fillRect(0, CONTENT_TOP, 240, CONTENT_BOTTOM - CONTENT_TOP, TFT_BLACK);
}

void Pages::drawHeader(const String &title, const AppConfig &cfg) {
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextSize(1);
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    _tft.drawString(title, CONTENT_LEFT, CONTENT_TOP);
    _tft.drawFastHLine(CONTENT_LEFT, CONTENT_TOP + 14, CONTENT_RIGHT - CONTENT_LEFT, TFT_DARKGREY);
}

void Pages::drawPageTitle(const String &title, const AppConfig &cfg) {
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextSize(title.length() > 8 ? 2 : 3);
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    _tft.drawString(title, 120, 44);
}

void Pages::draw(Page page, const AppConfig &cfg, const WeatherData &weather,
                 const AirQualityData &aqi,
                 float eyeAngle, bool needsRedraw) {
    clearIfChanged(page);
    switch (page) {
        case Page::Eyes: drawEyes(cfg, eyeAngle); break;
        case Page::Clock: if (shouldDrawInfo(page, needsRedraw, 1000)) drawClock(cfg); break;
        case Page::CurrentWeather: if (shouldDrawInfo(page, needsRedraw, 2000)) drawCurrentWeather(cfg, weather); break;
        case Page::Forecast: if (shouldDrawInfo(page, needsRedraw, 5000)) drawForecast(cfg, weather); break;
        case Page::HourlyGraph: if (shouldDrawInfo(page, needsRedraw, 5000)) drawHourlyGraph(cfg, weather); break;
        case Page::AirQuality: if (shouldDrawInfo(page, needsRedraw, 5000)) drawAirQuality(cfg, aqi); break;
        default: break;
    }
    drawStatusBar(page, cfg);
    drawPageIndicator(page, cfg);
}

bool Pages::shouldDrawInfo(Page page, bool needsRedraw, uint32_t intervalMs) {
    uint32_t now = millis();
    if (_lastInfoDrawMs == 0) {
        _lastInfoDrawMs = now;
        return true;
    }
    if (_lastPage == page && !needsRedraw && now - _lastInfoDrawMs < intervalMs) return false;
    _lastInfoDrawMs = now;
    return true;
}

void Pages::drawStatusBar(Page page, const AppConfig &cfg) {
    (void)page;
    struct tm info;
    char buf[8] = {0};
    bool haveTime = getLocalTime(&info, 1);
    if (haveTime) strftime(buf, sizeof(buf), "%H:%M", &info);
    String timeStr = haveTime ? String(buf) : String();
    uint16_t color = accent(cfg);
    if (_statusBarCity != cfg.city || _statusBarTime != timeStr || _statusBarColor != color) {
        _statusBarCity = cfg.city;
        _statusBarTime = timeStr;
        _statusBarColor = color;
        _statusBarDirty = true;
    }
    if (!_statusBarDirty) return;
    _statusBarDirty = false;
    _tft.fillRect(0, 0, 240, 20, TFT_BLACK);
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextSize(2);
    _tft.setTextColor(color, TFT_BLACK);
    _tft.drawString(cfg.city, 4, 2);
    if (haveTime) {
        _tft.setTextDatum(TR_DATUM);
        _tft.drawString(buf, 236, 2);
    }
}

void Pages::drawPageIndicator(Page page, const AppConfig &cfg) {
    uint16_t color = accent(cfg);
    if (_indicatorPage != page || _indicatorColor != color) {
        _indicatorPage = page;
        _indicatorColor = color;
        _indicatorDirty = true;
    }
    if (!_indicatorDirty) return;
    _indicatorDirty = false;
    const int total = (int)Page::Count;
    int active = (int)page;
    int startX = 120 - (total * 10) / 2;
    _tft.fillRect(0, 228, 240, 12, TFT_BLACK);
    for (int i = 0; i < total; ++i) {
        _tft.fillCircle(startX + i * 10, 234, i == active ? 3 : 2,
                        i == active ? color : TFT_DARKGREY);
    }
}

void Pages::drawEyes(const AppConfig &cfg, float angle) {
    uint16_t color = accent(cfg);
    uint32_t now = millis();
    if (_nextBlinkMs == 0) _nextBlinkMs = now + 2500;
    if (now >= _nextBlinkMs) {
        _blinkUntilMs = now + 140;
        _nextBlinkMs = now + (uint32_t)random(2500, 6000);
    }
    bool blinking = now < _blinkUntilMs;
    float sleepPhase = (sinf(angle * 1.2f) + 1.0f) * 0.5f;
    _eyeSprite.fillSprite(TFT_BLACK);

    float maxOffsetX = (EYE_W / 2.0f) - SCLERA_PAD - (PUPIL_SIZE / 2.0f);
    float maxOffsetY = (EYE_H / 2.0f) - SCLERA_PAD - (PUPIL_SIZE / 2.0f);
    int16_t offX = (int16_t)(cosf(angle) * maxOffsetX);
    int16_t offY = (int16_t)(sinf(angle * 0.5f) * maxOffsetY);

    auto drawEyelid = [&](int16_t x, int16_t y, int16_t h) {
        _eyeSprite.fillRect(x, y, EYE_W, h, TFT_BLACK);
        _eyeSprite.fillRect(x, y + EYE_H - h, EYE_W, h, TFT_BLACK);
    };

    EyeExpression expression = (EyeExpression)constrain((int)cfg.eyeExpression, 0, 6);

    if (expression == EyeExpression::Cute) {
        drawCuteFace(_eyeSprite, color, sleepPhase, blinking);
        _eyeSprite.pushSprite(FACE_X, FACE_Y);
        return;
    }
    if (expression == EyeExpression::Glasses) {
        drawGlassesFace(_eyeSprite, color, sleepPhase, blinking);
        _eyeSprite.pushSprite(FACE_X, FACE_Y);
        return;
    }

    auto drawOne = [&](int16_t x, int16_t y, bool left) {
        if (expression == EyeExpression::Round) {
            _eyeSprite.fillCircle(x + EYE_W / 2, y + EYE_H / 2, EYE_W / 2, color);
        } else {
            _eyeSprite.fillRoundRect(x, y, EYE_W, EYE_H, 10, color);
        }
        int16_t cx = x + constrain((EYE_W / 2) + offX, SCLERA_PAD + PUPIL_SIZE / 2,
                                   EYE_W - SCLERA_PAD - PUPIL_SIZE / 2);
        int16_t cy = y + constrain((EYE_H / 2) + offY, SCLERA_PAD + PUPIL_SIZE / 2,
                                   EYE_H - SCLERA_PAD - PUPIL_SIZE / 2);
        drawExpressionPupil(_eyeSprite, expression, cx, cy, TFT_BLACK);
        if (blinking) {
            drawEyelid(x, y, 34);
        }
        if (expression == EyeExpression::Angry) drawAngryBrow(_eyeSprite, x, y, left, color);
    };

    drawOne(LEFT_EYE_X, EYE_Y, true);
    drawOne(RIGHT_EYE_X, EYE_Y, false);
    _eyeSprite.pushSprite(FACE_X, FACE_Y);
}

void Pages::drawExpressionPupil(TFT_eSprite &sprite, EyeExpression expression,
                                int16_t cx, int16_t cy, uint16_t color) {
    if (expression == EyeExpression::Heart) {
        drawHeartPupil(sprite, cx, cy, color);
        return;
    }
    if (expression == EyeExpression::Star) {
        drawStarPupil(sprite, cx, cy, color);
        return;
    }
    if (expression == EyeExpression::Round) {
        sprite.fillCircle(cx, cy, PUPIL_SIZE / 2, color);
        return;
    }
    sprite.fillRoundRect(cx - PUPIL_SIZE / 2, cy - PUPIL_SIZE / 2,
                         PUPIL_SIZE, PUPIL_SIZE, 6, color);
}

void Pages::drawHeartPupil(TFT_eSprite &sprite, int16_t cx, int16_t cy, uint16_t color) {
    drawHeart(sprite, cx, cy - 1, 15, color);
    sprite.fillCircle(cx - 4, cy - 6, 2, TFT_WHITE);
}

void Pages::drawStarPupil(TFT_eSprite &sprite, int16_t cx, int16_t cy, uint16_t color) {
    drawStar(sprite, cx, cy, 20, color);
    sprite.fillCircle(cx - 5, cy - 5, 2, TFT_WHITE);
}

void Pages::drawHeart(TFT_eSprite &sprite, int16_t cx, int16_t cy, int16_t size, uint16_t color) {
    for (int16_t y = -size; y <= size; ++y) {
        float yf = (float)y / (float)size;
        float half = size * sqrtf(max(0.0f, 1.0f - yf * yf));
        if (y < 0) {
            half += size * 0.32f * (1.0f - fabsf(yf));
        } else {
            half *= 1.0f - (float)y / ((float)size * 1.18f);
        }
        if (half < 0) half = 0;
        sprite.drawFastHLine(cx - (int16_t)half, cy + y, (int16_t)(half * 2.0f) + 1, color);
    }
    sprite.fillCircle(cx - size / 2, cy - size / 3, size / 2, color);
    sprite.fillCircle(cx + size / 2, cy - size / 3, size / 2, color);
}

void Pages::drawStar(TFT_eSprite &sprite, int16_t cx, int16_t cy, int16_t radius, uint16_t color) {
    const int points = 10;
    int16_t xs[points];
    int16_t ys[points];
    for (int i = 0; i < points; ++i) {
        float a = -PI / 2 + i * PI / 5.0f;
        int r = (i % 2 == 0) ? radius : radius * 0.42f;
        xs[i] = cx + (int16_t)(cosf(a) * r);
        ys[i] = cy + (int16_t)(sinf(a) * r);
    }
    for (int i = 1; i < points - 1; ++i) {
        sprite.fillTriangle(xs[0], ys[0], xs[i], ys[i], xs[i + 1], ys[i + 1], color);
    }
    sprite.fillTriangle(cx, cy, xs[8], ys[8], xs[9], ys[9], color);
}

void Pages::drawAngryBrow(TFT_eSprite &sprite, int16_t x, int16_t y, bool left, uint16_t color) {
    int16_t y0 = y - 14;
    if (left) sprite.drawWideLine(x + 12, y0 + 4, x + 68, y0 + 22, 5, color, TFT_BLACK);
    else sprite.drawWideLine(x + 12, y0 + 22, x + 68, y0 + 4, 5, color, TFT_BLACK);
}

void Pages::drawCuteFace(TFT_eSprite &sprite, uint16_t color, float phase, bool blinking) {
    int16_t bob = (int16_t)(phase * 3.0f);
    int16_t leftX = 66;
    int16_t rightX = 134;
    int16_t eyeY = 42 + bob;

    if (blinking) {
        sprite.drawWideLine(leftX - 12, eyeY + 8, leftX + 12, eyeY + 8, 5, color, TFT_BLACK);
        sprite.drawWideLine(rightX - 12, eyeY + 8, rightX + 12, eyeY + 8, 5, color, TFT_BLACK);
    } else {
        sprite.fillRoundRect(leftX - 13, eyeY - 24, 26, 54, 13, color);
        sprite.fillRoundRect(rightX - 13, eyeY - 24, 26, 54, 13, color);
        sprite.fillRoundRect(leftX - 5, eyeY - 14, 10, 34, 5, TFT_BLACK);
        sprite.fillRoundRect(rightX - 5, eyeY - 14, 10, 34, 5, TFT_BLACK);
    }

    sprite.fillRoundRect(34, 82 + bob, 24, 12, 6, color);
    sprite.fillRoundRect(142, 82 + bob, 24, 12, 6, color);
    sprite.fillEllipse(100, 106 + bob, 14, 8, color);
}

void Pages::drawGlassesFace(TFT_eSprite &sprite, uint16_t color, float phase, bool blinking) {
    int16_t bob = (int16_t)(phase * 2.0f);
    int16_t y = 34 + bob;

    sprite.fillRoundRect(38, y, 52, 50, 8, color);
    sprite.fillRoundRect(110, y, 52, 50, 8, color);
    sprite.fillRoundRect(44, y + 6, 40, 38, 6, TFT_BLACK);
    sprite.fillRoundRect(116, y + 6, 40, 38, 6, TFT_BLACK);
    sprite.drawWideLine(89, y + 20, 111, y + 20, 5, color, TFT_BLACK);
    sprite.drawWideLine(37, y + 8, 28, y + 3, 4, color, TFT_BLACK);
    sprite.drawWideLine(163, y + 8, 172, y + 3, 4, color, TFT_BLACK);

    if (blinking) {
        sprite.drawWideLine(56, y + 25, 72, y + 25, 4, color, TFT_BLACK);
        sprite.drawWideLine(128, y + 25, 144, y + 25, 4, color, TFT_BLACK);
    } else {
        sprite.fillEllipse(64, y + 25, 10, 17, color);
        sprite.fillEllipse(136, y + 25, 10, 17, color);
        sprite.fillEllipse(64, y + 25, 4, 10, TFT_BLACK);
        sprite.fillEllipse(136, y + 25, 4, 10, TFT_BLACK);
    }

    sprite.drawWideLine(86, 95 + bob, 94, 103 + bob, 4, color, TFT_BLACK);
    sprite.drawWideLine(94, 103 + bob, 106, 105 + bob, 4, color, TFT_BLACK);
    sprite.drawWideLine(106, 105 + bob, 116, 97 + bob, 4, color, TFT_BLACK);
    sprite.drawArc(100, 24 + bob, 12, 9, 205, 335, color, TFT_BLACK);
}

void Pages::drawClock(const AppConfig &cfg) {
    clearContent();
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    struct tm info;
    if (!getLocalTime(&info, 20)) {
        _tft.setTextSize(2);
        _tft.drawString("Syncing time", 120, 116);
        return;
    }
    char timeBuf[8];
    char meridiem[3] = "";
    if (cfg.timeFormat == "12h") {
        strftime(timeBuf, sizeof(timeBuf), "%I:%M", &info);
        strftime(meridiem, sizeof(meridiem), "%p", &info);
    } else {
        strftime(timeBuf, sizeof(timeBuf), "%H:%M", &info);
    }
    String dateLine = formatWeekday(info) + " " + formatDate(cfg, info);
    _tft.setTextSize(4);
    _tft.drawString(timeBuf, 120, 96);
    if (cfg.timeFormat == "12h") {
        _tft.setTextSize(2);
        _tft.drawString(String(meridiem), 120, 128);
    }
    _tft.setTextSize(2);
    _tft.drawString(dateLine, 120, 154);
    _tft.setTextSize(1);
    String ip;
    if (WiFi.status() == WL_CONNECTED) ip = WiFi.localIP().toString();
    else ip = WiFi.softAPIP().toString();
    if (ip != "0.0.0.0") _tft.drawString(ip, 120, 180);
    else _tft.drawString("No IP", 120, 180);
}

void Pages::drawCurrentWeather(const AppConfig &cfg, const WeatherData &weather) {
    clearContent();
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    drawPageTitle("CURRENT", cfg);
    if (!weather.valid) {
        _tft.setTextSize(2);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString("Weather loading", 120, 116);
        return;
    }
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextSize(5);
    _tft.drawString(String((int)roundf(celsiusToDisplay(cfg, weather.current.temperature))) + tempUnit(cfg), 120, 96);
    _tft.setTextSize(2);
    _tft.drawString(wmo_weather_text(weather.current.weather_code), 120, 140);
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextSize(1);
    _tft.drawString("Feels " + String(celsiusToDisplay(cfg, weather.current.apparent_temperature), 1) + tempUnit(cfg), CONTENT_LEFT, 166);
    _tft.drawString("Humidity " + String((int)weather.current.humidity) + "%", 126, 166);
    _tft.drawString("Wind " + String(weather.current.wind_speed, 1) + " km/h", CONTENT_LEFT, 188);
    _tft.drawString("Pressure " + String(weather.current.pressure, 0) + " hPa", CONTENT_LEFT, 206);
}

void Pages::drawForecastIcon(int code, int16_t x, int16_t y, uint16_t color) {
    const uint16_t sunColor = TFT_YELLOW;
    const uint16_t rainColor = _tft.color565(70, 180, 255);
    const uint16_t snowColor = TFT_WHITE;
    const uint16_t fogColor = TFT_DARKGREY;
    const uint16_t lightningColor = TFT_YELLOW;

    auto drawSun = [&](int16_t cx, int16_t cy, int16_t r) {
        _tft.fillCircle(cx, cy, r, sunColor);
        for (int i = 0; i < 8; ++i) {
            float a = i * PI / 4.0f;
            _tft.drawLine(cx + (int16_t)(cosf(a) * (r + 3)), cy + (int16_t)(sinf(a) * (r + 3)),
                          cx + (int16_t)(cosf(a) * (r + 6)), cy + (int16_t)(sinf(a) * (r + 6)),
                          sunColor);
        }
    };

    auto drawCloud = [&](int16_t cx, int16_t cy, uint16_t cloudColor) {
        _tft.fillCircle(cx - 7, cy + 2, 5, cloudColor);
        _tft.fillCircle(cx, cy - 2, 7, cloudColor);
        _tft.fillCircle(cx + 8, cy + 2, 5, cloudColor);
        _tft.fillRoundRect(cx - 12, cy, 24, 8, 2, cloudColor);
    };

    auto drawRain = [&](int16_t cx, int16_t cy, int drops) {
        const int16_t xs[] = {-7, 0, 7};
        for (int i = 0; i < drops && i < 3; ++i) {
            _tft.drawLine(cx + xs[i] + 2, cy + 7, cx + xs[i] - 1, cy + 13, rainColor);
        }
    };

    auto drawSnowflake = [&](int16_t cx, int16_t cy, uint16_t flakeColor) {
        _tft.drawLine(cx - 3, cy, cx + 3, cy, flakeColor);
        _tft.drawLine(cx, cy - 3, cx, cy + 3, flakeColor);
        _tft.drawLine(cx - 2, cy - 2, cx + 2, cy + 2, flakeColor);
        _tft.drawLine(cx - 2, cy + 2, cx + 2, cy - 2, flakeColor);
    };

    auto drawLightning = [&](int16_t cx, int16_t cy) {
        _tft.fillTriangle(cx + 1, cy + 4, cx - 4, cy + 12, cx + 1, cy + 11, lightningColor);
        _tft.fillTriangle(cx + 1, cy + 10, cx - 1, cy + 17, cx + 6, cy + 8, lightningColor);
    };

    if (code == 0 || code == 1) {
        drawSun(x, y, 5);
        return;
    }
    if (code == 2) {
        drawSun(x - 7, y - 5, 4);
        drawCloud(x + 3, y + 1, color);
        return;
    }
    if (code == 3) {
        drawCloud(x, y, color);
        _tft.drawFastHLine(x - 9, y + 10, 18, fogColor);
        return;
    }
    if (code == 45 || code == 48) {
        drawCloud(x, y - 2, color);
        _tft.drawFastHLine(x - 12, y + 8, 24, fogColor);
        _tft.drawFastHLine(x - 9, y + 12, 18, fogColor);
        _tft.drawFastHLine(x - 6, y + 16, 12, fogColor);
        return;
    }
    if (code >= 51 && code <= 57) {
        drawCloud(x, y - 3, color);
        drawRain(x, y, code >= 53 ? 3 : 2);
        if (code >= 56) drawSnowflake(x + 11, y + 10, snowColor);
        return;
    }
    if (code >= 61 && code <= 67) {
        drawCloud(x, y - 3, color);
        drawRain(x, y, code >= 63 ? 3 : 2);
        if (code >= 66) drawSnowflake(x + 11, y + 10, snowColor);
        return;
    }
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) {
        drawCloud(x, y - 4, color);
        drawSnowflake(x - 6, y + 9, snowColor);
        drawSnowflake(x + 6, y + 11, snowColor);
        return;
    }
    if (code >= 80 && code <= 82) {
        drawSun(x - 7, y - 6, 3);
        drawCloud(x + 2, y - 3, color);
        drawRain(x, y, code == 80 ? 2 : 3);
        return;
    }
    if (code >= 95 && code <= 99) {
        drawCloud(x, y - 4, color);
        drawLightning(x, y - 1);
        if (code >= 96) {
            _tft.fillCircle(x + 9, y + 10, 2, snowColor);
            _tft.fillCircle(x - 9, y + 12, 2, snowColor);
        }
        return;
    }
    _tft.drawCircle(x, y, 8, color);
    _tft.drawPixel(x, y, color);
}

void Pages::drawForecast(const AppConfig &cfg, const WeatherData &weather) {
    clearContent();
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    drawPageTitle("FORECAST", cfg);
    if (!weather.valid) {
        _tft.setTextSize(2);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString("No data", 120, 116);
        return;
    }
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.drawString("DATE", 8, 68);
    _tft.drawString("TEMP", 112, 68);
    _tft.drawString("RAIN", 184, 68);
    _tft.drawFastHLine(8, 82, 224, TFT_DARKGREY);

    int y = 90;
    for (size_t i = 0; i < weather.daily.size() && i < 5; ++i) {
        const WeatherDaily &d = weather.daily[i];
        String day = d.date.length() >= 10 ? d.date.substring(5) : d.date;
        _tft.setTextSize(1);
        _tft.setTextColor(TFT_WHITE, TFT_BLACK);
        _tft.drawString(day, 8, y + 4);
        drawForecastIcon(d.weather_code, 70, y + 10, accent(cfg));
        _tft.setTextSize(2);
        _tft.setTextColor(accent(cfg), TFT_BLACK);
        _tft.drawString(String((int)roundf(celsiusToDisplay(cfg, d.temp_min))) + "/" +
                        String((int)roundf(celsiusToDisplay(cfg, d.temp_max))) + tempUnit(cfg), 106, y);
        _tft.drawString(String(d.precipitation_probability_max) + "%", 186, y);
        y += 26;
    }
}

void Pages::drawHourlyGraph(const AppConfig &cfg, const WeatherData &weather) {
    clearContent();
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    drawPageTitle("HOURLY 24H", cfg);
    if (!weather.valid || weather.hourly.empty()) {
        _tft.setTextSize(2);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString("No data", 120, 116);
        return;
    }
    _tft.setTextDatum(TL_DATUM);

    float minT = celsiusToDisplay(cfg, weather.hourly[0].temperature);
    float maxT = minT;
    for (const auto &h : weather.hourly) {
        float temp = celsiusToDisplay(cfg, h.temperature);
        minT = min(minT, temp);
        maxT = max(maxT, temp);
    }
    if (fabsf(maxT - minT) < 0.1f) maxT = minT + 1.0f;

    int prevX = -1;
    int prevY = -1;
    size_t count = min((size_t)24, weather.hourly.size());
    const int16_t CHART_X = 10;
    const int16_t CHART_Y = 72;
    const int16_t CHART_W = 196;
    const int16_t TEMP_GRAPH_H = 68;
    const int16_t RAIN_BASE_Y = 214;
    const int16_t RAIN_BAR_MAX_H = 52;
    _tft.drawRect(CHART_X, CHART_Y, CHART_W, 142, TFT_DARKGREY);
    _tft.drawFastHLine(CHART_X, RAIN_BASE_Y - RAIN_BAR_MAX_H, CHART_W, TFT_DARKGREY);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setCursor(16, 78);
    _tft.print("TEMP");
    _tft.setCursor(166, 166);
    _tft.print("RAIN");
    for (size_t i = 0; i < count; ++i) {
        const auto &h = weather.hourly[i];
        int x = 14 + (count > 1 ? (int)((i * 188) / (count - 1)) : 0);
        float temp = celsiusToDisplay(cfg, h.temperature);
        int y = (RAIN_BASE_Y - RAIN_BAR_MAX_H - 8) - (int)((temp - minT) * TEMP_GRAPH_H / (maxT - minT));
        int barH = map(constrain(h.precipitation_probability, 0, 100), 0, 100, 0, RAIN_BAR_MAX_H);
        _tft.drawFastVLine(x, RAIN_BASE_Y - barH, barH, TFT_BLUE);
        if (prevX >= 0) _tft.drawLine(prevX, prevY, x, y, accent(cfg));
        prevX = x;
        prevY = y;
    }
    _tft.setTextSize(1);
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    _tft.drawString(String(maxT, 0) + tempUnit(cfg), 210, 78);
    _tft.drawString(String(minT, 0) + tempUnit(cfg), 210, RAIN_BASE_Y - RAIN_BAR_MAX_H - 8);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setCursor(12, 204);
    _tft.print("NOW");
    _tft.setCursor(96, 204);
    _tft.print("+12H");
    _tft.setCursor(174, 204);
    _tft.print("+24H");
}

void Pages::drawAirQuality(const AppConfig &cfg, const AirQualityData &aqi) {
    clearContent();
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    drawPageTitle("AIR QUALITY", cfg);
    if (!aqi.valid) {
        _tft.setTextDatum(TC_DATUM);
        _tft.drawString("AQI loading", 120, 130);
        return;
    }

    auto drawMetricBox = [&](int16_t x, int16_t y, int16_t w, int16_t h,
                             const char *label, const String &value) {
        _tft.drawRoundRect(x, y, w, h, 4, TFT_DARKGREY);
        _tft.setTextDatum(TC_DATUM);
        _tft.setTextSize(1);
        _tft.setTextColor(TFT_WHITE, TFT_BLACK);
        _tft.drawString(label, x + w / 2, y + 5);
        _tft.setTextSize(2);
        _tft.setTextColor(accent(cfg), TFT_BLACK);
        _tft.drawString(value, x + w / 2, y + 22);
    };

    drawMetricBox(10, 70, 104, 48, "AQI", String(aqi.us_aqi) + " US");
    drawMetricBox(126, 70, 104, 48, "EU AQI", String(aqi.european_aqi));

    _tft.setTextDatum(TL_DATUM);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.drawString("PARTICLES", 12, 126);
    drawMetricBox(10, 140, 104, 38, "PM2.5", String(aqi.pm2_5, 1));
    drawMetricBox(126, 140, 104, 38, "PM10", String(aqi.pm10, 1));
    
    
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextSize(1);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.drawString("GASES", 12, 188);
    _tft.setTextSize(2);
    _tft.setTextColor(accent(cfg), TFT_BLACK);
    _tft.drawString("O3 " + String(aqi.ozone, 0), 5, 210);
    _tft.drawString("NO2 " + String(aqi.nitrogen_dioxide, 0), 85, 210);
    _tft.drawString("CO " + String(aqi.carbon_monoxide, 0), 165, 210);
}
