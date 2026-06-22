#pragma once

#include <Arduino.h>

// WifiManager — connect to an AP, sync time via SNTP, and disconnect.
class WifiManager {
public:
    // Connect to the given SSID. Returns true when an IP is obtained.
    // Blocks until connected or a timeout (~15 s) is reached.
    bool connect(const String &ssid, const String &password);

    // Synchronize the system clock via SNTP.
    // utcOffsetMinutes is the UTC offset in minutes, e.g. 480 for UTC+8.
    bool syncTime(int utcOffsetMinutes);

    // Stop SNTP before WiFi mode changes that tear down lwIP interfaces.
    void stopTimeSync();

    // Disconnect WiFi and power down the radio.
    void disconnect();

    // Switch to SoftAP mode with the given SSID (open network, no password).
    // Default IP: 192.168.4.1. Call webServer.start() after this.
    void startAP(const String &ssid);
};
