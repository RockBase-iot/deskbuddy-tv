#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

class NmTv154Hal {
public:
    void begin();
    void update();
    TFT_eSPI &display() { return _display; }
    bool touched() const { return _touched; }
    void setBacklight(uint8_t level);
    void screenOn(bool on);
    void setBootDiagnostics(bool enabled) { _bootDiagnostics = enabled; }
    void renderStartupPattern(const char *label);

private:
    TFT_eSPI _display;
    bool _touched = false;
    bool _bootDiagnostics = false;
};
