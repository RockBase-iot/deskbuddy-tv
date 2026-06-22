#include <Arduino.h>
#include <WiFi.h>

#include "app/weather/weather.h"
#include "app/web/web_config.h"
#include "app/web/web_server.h"
#include "app/wifi/wifi_manager.h"
#include "config/app_config.h"
#include "hal/nm_tv_154_hal.h"
#include "input/touch_gesture.h"
#include "ui/pages.h"
#include "version.h"

static NmTv154Hal hal;
static TouchGesture touch;
static AppConfig config;
static WifiManager wifi;
static WebConfig configPortal;
static WebServer webServer;
static WeatherClass weatherClient;
static Pages *pages = nullptr;

static const Page PAGE_ORDER[] = {
    Page::Eyes,
    Page::Clock,
    Page::CurrentWeather,
    Page::Forecast,
    Page::HourlyGraph,
    Page::AirQuality,
};

static Page currentPage = Page::Eyes;
static uint32_t lastWeatherFetch = 0;
static uint32_t lastWifiAttempt = 0;
static float eyeAngle = 0.0f;
static bool needsRedraw = true;
static bool wifiConnected = false;
static bool timeSynced = false;
static bool stationWebStarted = false;
static bool portalStarted = false;
static bool configModeActive = false;

static const uint32_t WIFI_RETRY_MS = 30000;
static const char *CONFIG_AP_SSID = "DeskBuddy";
static const uint32_t CONFIG_PORTAL_MS = 180000;
static const uint32_t CONFIG_STATUS_REFRESH_MS = 1000;
static const uint8_t EXPRESSION_COUNT = 6;
static uint32_t configModeStartedMs = 0;
static uint32_t lastConfigStatusDrawMs = 0;
static uint32_t lastConfigStaticDrawMs = 0;
static uint32_t lastOtaStatusDrawMs = 0;
static uint16_t lastConfigRemainingSec = 0xffff;
static uint8_t lastOtaProgress = 0xff;
static bool configPortalTimedOut = false;

static void showBootStage(const char *stage) {
    Serial.printf("[boot] %s\n", stage);
    TFT_eSPI &tft = hal.display();
    tft.fillRect(0, 200, 240, 40, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString(stage, 120, 220);
}

static void drawConfigPortalStatic() {
    TFT_eSPI &tft = hal.display();
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("WiFi Config", 120, 24);

    tft.drawFastHLine(18, 44, 204, TFT_DARKGREY);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("SSID: DeskBuddy", 18, 58);
    tft.drawString("AP:", 18, 88);
    tft.drawString(WiFi.softAPIP().toString(), 66, 88);

    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Open 192.168.4.1", 18, 122);
    tft.drawString("Long press exit", 18, 140);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("TIME LEFT", 120, 170);
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(String("v") + DESKBUDDY_VERSION, 120, 232);
}

static void drawConfigCountdown(uint16_t remainingSec) {
    TFT_eSPI &tft = hal.display();
    tft.fillRect(70, 186, 100, 40, TFT_BLACK);
    char timeText[8];
    snprintf(timeText, sizeof(timeText), "%us", remainingSec);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(3);
    tft.drawString(timeText, 120, 204);
}

static void drawConfigPortalStatus(uint32_t nowMs) {
    uint32_t elapsedMs = nowMs - configModeStartedMs;
    uint32_t remainingMs = elapsedMs >= CONFIG_PORTAL_MS ? 0 : CONFIG_PORTAL_MS - elapsedMs;
    uint16_t remainingSec = (remainingMs + 999) / 1000;
    if (needsRedraw || lastConfigStaticDrawMs == 0) {
        drawConfigPortalStatic();
        lastConfigStaticDrawMs = nowMs;
        lastConfigRemainingSec = 0xffff;
        needsRedraw = false;
    }
    if (!needsRedraw && lastConfigRemainingSec == remainingSec &&
        nowMs - lastConfigStatusDrawMs < CONFIG_STATUS_REFRESH_MS) {
        return;
    }

    lastConfigStatusDrawMs = nowMs;
    lastConfigRemainingSec = remainingSec;
    drawConfigCountdown(remainingSec);
}

static void drawOtaStatus(uint32_t nowMs) {
    uint8_t progress = configPortal.otaProgress();
    if (lastOtaProgress == progress && nowMs - lastOtaStatusDrawMs < 500) {
        return;
    }
    lastOtaStatusDrawMs = nowMs;
    lastOtaProgress = progress;

    TFT_eSPI &tft = hal.display();
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("OTA Updating", 120, 58);
    tft.setTextSize(4);
    tft.drawString(String(progress) + "%", 120, 112);
    tft.drawRect(34, 154, 172, 16, TFT_DARKGREY);
    tft.fillRect(36, 156, map(progress, 0, 100, 0, 168), 12, TFT_BLUE);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Do not power off", 120, 190);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(String("v") + DESKBUDDY_VERSION, 120, 226);
}

static void refreshWeatherNow() {
    Serial.println("[net] Fetch Open-Meteo weather");
    bool weatherOk = weatherClient.fetchWeather(config.latitude, config.longitude);
    Serial.printf("[net] Weather result=%s\n", weatherOk ? "ok" : "fail");
    Serial.println("[net] Fetch Open-Meteo air quality");
    bool aqiOk = weatherClient.fetchAirQuality(config.latitude, config.longitude);
    Serial.printf("[net] AQI result=%s\n", aqiOk ? "ok" : "fail");
    lastWeatherFetch = millis();
    needsRedraw = true;
}

static void tryConnectWifi(bool force = false);
static void syncNetworkServicesIfReady();

static void fetchWeatherIfDue(bool force = false) {
    if (!wifiConnected) return;
    uint32_t now = millis();
    if (!force && now - lastWeatherFetch < 10UL * 60UL * 1000UL) return;
    refreshWeatherNow();
}

static void clearScreenBeforeNormalUi() {
    hal.display().fillScreen(TFT_BLACK);
    if (pages) {
        pages->markDirty();
    }
    needsRedraw = true;
}

static void exitConfigPortal(const char *reason) {
    Serial.printf("[net] Config portal exit: %s\n", reason ? reason : "manual");
    if (configPortal.isRunning()) {
        configPortal.stop();
    }

    portalStarted = false;
    configModeActive = false;
    configPortalTimedOut = true;
    wifiConnected = false;
    timeSynced = false;
    lastWifiAttempt = 0;
    lastWeatherFetch = 0;

    loadAppConfig(config);
    clearScreenBeforeNormalUi();
    WiFi.mode(WIFI_STA);

    if (config.wifiSsid.length() > 0) {
        showBootStage("WiFi reconnect");
        tryConnectWifi(true);
        syncNetworkServicesIfReady();
        fetchWeatherIfDue(true);
    }
}

static void startConfigPortalIfNeeded() {
    uint32_t now = millis();
    hal.display().fillScreen(TFT_BLACK);
    if (portalStarted && configPortal.isRunning()) {
        configModeActive = true;
        configModeStartedMs = now;
        lastConfigStatusDrawMs = 0;
        lastConfigStaticDrawMs = 0;
        lastConfigRemainingSec = 0xffff;
        lastOtaStatusDrawMs = 0;
        lastOtaProgress = 0xff;
        needsRedraw = true;
        drawConfigPortalStatus(now);
        return;
    }

    Serial.println("[boot] Config portal");
    if (stationWebStarted) {
        webServer.stop();
        stationWebStarted = false;
    }
    wifi.stopTimeSync();
    wifiConnected = false;
    timeSynced = false;
    configModeActive = true;
    configModeStartedMs = now;
    lastConfigStatusDrawMs = 0;
    lastConfigStaticDrawMs = 0;
    lastConfigRemainingSec = 0xffff;
    lastOtaStatusDrawMs = 0;
    lastOtaProgress = 0xff;
    configPortalTimedOut = false;
    configPortal.start(CONFIG_AP_SSID);
    portalStarted = true;
    needsRedraw = true;
    drawConfigPortalStatus(now);
}

static void tryConnectWifi(bool force) {
    if (wifiConnected || config.wifiSsid.length() == 0) return;

    uint32_t now = millis();
    if (!force && now - lastWifiAttempt < WIFI_RETRY_MS) return;
    lastWifiAttempt = now;

    Serial.printf("[net] WiFi connect SSID=%s\n", config.wifiSsid.c_str());
    wifiConnected = wifi.connect(config.wifiSsid, config.wifiPassword);
    Serial.printf("[net] WiFi result=%s\n", wifiConnected ? "ok" : "fail");
    needsRedraw = true;
}

static void syncNetworkServicesIfReady() {
    if (!wifiConnected) return;

    if (!configModeActive && portalStarted && configPortal.isRunning()) {
        configPortal.stop();
        portalStarted = false;
        WiFi.mode(WIFI_STA);
    }

    if (!timeSynced) {
        Serial.println("[net] NTP sync");
        wifi.syncTime(config.timezoneOffsetMinutes);
        timeSynced = true;
        needsRedraw = true;
    }

    if (!stationWebStarted) {
        webServer.start();
        stationWebStarted = true;
    }
}

void setup() {
    hal.setBootDiagnostics(false);
    hal.begin();
    showBootStage("Init app");
    Serial.println("[boot] Loading config");
    loadAppConfig(config);

    pages = new Pages(hal.display());
    pages->begin();
    pages->drawStartupSplash(config);
    pages->cycleAccentSplash(config);
    showBootStage("UI ready");

    bool wifiOk = false;
    if (config.wifiSsid.length() > 0) {
        showBootStage("WiFi connecting");
        tryConnectWifi(true);
        wifiOk = wifiConnected;
    }

    if (!wifiOk) {
        startConfigPortalIfNeeded();
    }

    syncNetworkServicesIfReady();
    if (configModeActive) {
        Serial.println("[boot] DeskBuddy ready");
        drawConfigPortalStatus(millis());
    } else {
        showBootStage("DeskBuddy ready");
        pages->draw(currentPage, config, weatherClient.weather(),
                    weatherClient.airQuality(), eyeAngle, true);
        fetchWeatherIfDue(true);
    }
}

void loop() {
    uint32_t now = millis();
    hal.update();
    configPortal.process();
    touch.update(hal.touched(), now);

    if (configModeActive) {
        if (configPortal.otaInProgress()) {
            drawOtaStatus(now);
            delay(50);
            return;
        }
        if (now - configModeStartedMs >= CONFIG_PORTAL_MS) {
            exitConfigPortal("timeout");
        } else {
            drawConfigPortalStatus(now);
            if (touch.wasConfigPress()) {
                exitConfigPortal("touch");
                delay(50);
                return;
            }
            touch.wasSingleTap();
            touch.wasDoubleTap();
            touch.wasLongPress();
            delay(50);
            return;
        }
    }

    if (touch.wasSingleTap()) {
        uint8_t currentIndex = 0;
        for (uint8_t i = 0; i < sizeof(PAGE_ORDER) / sizeof(PAGE_ORDER[0]); ++i) {
            if (PAGE_ORDER[i] == currentPage) currentIndex = i;
        }
        currentPage = PAGE_ORDER[(currentIndex + 1) % (sizeof(PAGE_ORDER) / sizeof(PAGE_ORDER[0]))];
        pages->markDirty();
        needsRedraw = true;
    }
    if (touch.wasDoubleTap()) {
        if (config.colorCycleMode) {
            config.eyeColorIndex = (config.eyeColorIndex + 1) % 9;
            saveAppConfig(config);
            needsRedraw = true;
        }
    }
    if (touch.wasLongPress() && currentPage == Page::Eyes) {
        config.eyeExpression = (config.eyeExpression + 1) % EXPRESSION_COUNT;
        config.roundEyeMode = config.eyeExpression == 1;
        saveAppConfig(config);
        needsRedraw = true;
    }
    if (touch.wasConfigPress()) {
        startConfigPortalIfNeeded();
        delay(50);
        return;
    }

    if (!wifiConnected && config.wifiSsid.length() > 0) {
        tryConnectWifi();
    }
    if (!wifiConnected && !portalStarted && !configPortalTimedOut) {
        startConfigPortalIfNeeded();
        delay(50);
        return;
    }
    syncNetworkServicesIfReady();
    fetchWeatherIfDue();
    eyeAngle += 0.02f;
    if (eyeAngle > TWO_PI) eyeAngle -= TWO_PI;

    if (pages) {
        pages->draw(currentPage, config, weatherClient.weather(),
                    weatherClient.airQuality(), eyeAngle, needsRedraw);
        needsRedraw = false;
    }
    delay(50);
}
