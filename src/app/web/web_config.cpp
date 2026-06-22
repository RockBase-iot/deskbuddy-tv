#include "web_config.h"

#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <WiFi.h>
#include "app/web/config_form.h"
#include "config/app_config.h"
#include "utils/logger.h"

static const char *TAG = "WebConfig";
static const byte DNS_PORT = 53;
static const uint32_t RESTART_DELAY_MS = 1200;

static AsyncWebServer _server(80);
static DNSServer _dns;
static volatile bool _saved = false;
static bool _running = false;
static uint32_t _savedAtMs = 0;
static volatile bool _otaInProgress = false;
static volatile uint8_t _otaProgress = 0;
static volatile bool _otaOk = false;
static uint32_t _otaDoneAtMs = 0;

void WebConfig::start(const char *ap_ssid) {
    if (_running) return;

    _saved = false;
    _savedAtMs = 0;
    _otaInProgress = false;
    _otaProgress = 0;
    _otaOk = false;
    _otaDoneAtMs = 0;

    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid);
    IPAddress apIp = WiFi.softAPIP();
    _dns.start(DNS_PORT, "*", apIp);
    log_i(TAG, "AP started: %s IP=%s", ap_ssid, apIp.toString().c_str());

    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
        AppConfig cfg;
        loadAppConfig(cfg);
        req->send(200, "text/html", renderConfigForm(cfg));
    });

    _server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *req) {
        req->redirect("/");
    });
    _server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *req) {
        req->redirect("/");
    });
    _server.on("/fwlink", HTTP_GET, [](AsyncWebServerRequest *req) {
        req->redirect("/");
    });

    _server.on("/save", HTTP_POST, [](AsyncWebServerRequest *req) {
        AppConfig cfg;
        loadAppConfig(cfg);
        applyFormToConfig(req, cfg);
        saveAppConfig(cfg);
        req->send(200, "text/html", renderConfigForm(cfg, "Saved. Device will restart."));
        _saved = true;
        _savedAtMs = millis();
    });

    _server.on("/ota", HTTP_GET, [](AsyncWebServerRequest *req) {
        AppConfig cfg;
        loadAppConfig(cfg);
        req->send(200, "text/html", renderConfigForm(cfg));
    });

    _server.on("/api/ota", HTTP_POST,
        [](AsyncWebServerRequest *req) {
            bool ok = !_otaInProgress && !_otaOk && !Update.hasError();
            req->send(ok ? 200 : 500, "text/html",
                      ok ? "OTA complete. Device will restart." : "OTA failed.");
            if (ok) {
                _otaOk = true;
                _otaDoneAtMs = millis();
            }
        },
        [](AsyncWebServerRequest *req, const String &filename, size_t index,
           uint8_t *data, size_t len, bool final) {
            (void)req;
            if (index == 0) {
                log_i(TAG, "OTA upload start: %s", filename.c_str());
                _otaInProgress = true;
                _otaProgress = 0;
                _otaOk = false;
                _otaDoneAtMs = 0;
                if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
                    log_e(TAG, "OTA begin failed");
                    Update.printError(Serial);
                }
            }
            if (!Update.hasError()) {
                size_t written = Update.write(data, len);
                if (written != len) {
                    log_e(TAG, "OTA write failed");
                    Update.printError(Serial);
                }
                size_t total = Update.size();
                if (total > 0) {
                    _otaProgress = (uint8_t)min((size_t)99, (Update.progress() * 100) / total);
                }
            }
            if (final) {
                if (!Update.hasError() && Update.end(true)) {
                    _otaProgress = 100;
                    log_i(TAG, "OTA upload complete");
                } else {
                    log_e(TAG, "OTA end failed");
                    Update.printError(Serial);
                }
                _otaInProgress = false;
            }
        });

    _server.onNotFound([](AsyncWebServerRequest *req) {
        req->redirect("/");
    });

    _server.begin();
    _running = true;
}

void WebConfig::process() {
    if (!_running) return;

    _dns.processNextRequest();
    if (_saved && _savedAtMs != 0 && millis() - _savedAtMs >= RESTART_DELAY_MS) {
        log_i(TAG, "Config saved, restarting");
        ESP.restart();
    }
    if (_otaOk && _otaDoneAtMs != 0 && millis() - _otaDoneAtMs >= RESTART_DELAY_MS) {
        log_i(TAG, "OTA complete, restarting");
        ESP.restart();
    }
}

void WebConfig::stop() {
    if (!_running) return;

    _dns.stop();
    _server.end();
    WiFi.softAPdisconnect(true);
    _running = false;
    _saved = false;
    _savedAtMs = 0;
    _otaInProgress = false;
    _otaProgress = 0;
    _otaOk = false;
    _otaDoneAtMs = 0;
}

bool WebConfig::isRunning() const {
    return _running;
}

bool WebConfig::otaInProgress() const {
    return _otaInProgress;
}

uint8_t WebConfig::otaProgress() const {
    return _otaProgress;
}
