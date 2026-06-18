#include "web_config.h"

#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
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

void WebConfig::start(const char *ap_ssid) {
    if (_running) return;

    _saved = false;
    _savedAtMs = 0;

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
}

void WebConfig::stop() {
    if (!_running) return;

    _dns.stop();
    _server.end();
    WiFi.softAPdisconnect(true);
    _running = false;
    _saved = false;
    _savedAtMs = 0;
}

bool WebConfig::isRunning() const {
    return _running;
}
