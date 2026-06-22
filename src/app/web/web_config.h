#pragma once

#include <Arduino.h>

// WebConfig — captive-portal / AP-mode configuration web server.
//
// Start the server when no WiFi credentials are available or the user
// holds the boot button. Serves an HTML form; on submit, validates input
// and persists values to NVS via saveAppConfig().
class WebConfig {
public:
    // Start the AP and HTTP server. Non-blocking; call process() from loop().
    // ap_ssid: the SSID of the configuration hotspot.
    void start(const char *ap_ssid = "WeatherStation-Setup");

    // Pump DNS captive portal requests and handle deferred restart after save.
    void process();

    // Stop the server and AP.
    void stop();

    bool isRunning() const;
    bool otaInProgress() const;
    uint8_t otaProgress() const;
};
