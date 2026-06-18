#pragma once

#include <Arduino.h>

class TouchGesture {
public:
    static const uint32_t DOUBLE_TAP_MS = 280;
    static const uint32_t LONG_PRESS_MS = 800;
    static const uint32_t CONFIG_PRESS_MS = 5000;

    void update(bool touched, uint32_t nowMs);
    bool isPressed() const { return _down; }
    bool wasSingleTap();
    bool wasDoubleTap();
    bool wasLongPress();
    bool wasConfigPress();

private:
    bool _raw = false;
    bool _down = false;
    bool _pendingSingle = false;
    bool _singleTap = false;
    bool _doubleTap = false;
    bool _longPress = false;
    bool _configPress = false;
    bool _longPressEmitted = false;
    bool _configPressEmitted = false;
    uint32_t _downMs = 0;
    uint32_t _releaseMs = 0;
};
