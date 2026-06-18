#include "touch_gesture.h"

void TouchGesture::update(bool touched, uint32_t nowMs) {
    _singleTap = false;
    _doubleTap = false;
    _longPress = false;
    _configPress = false;

    if (_pendingSingle && (uint32_t)(nowMs - _releaseMs) >= DOUBLE_TAP_MS) {
        _singleTap = true;
        _pendingSingle = false;
    }

    if (touched && !_raw) {
        _raw = true;
        _down = true;
        _downMs = nowMs;
        _longPressEmitted = false;
        _configPressEmitted = false;
        if (_pendingSingle && (uint32_t)(nowMs - _releaseMs) < DOUBLE_TAP_MS) {
            _pendingSingle = false;
            _doubleTap = true;
        }
        return;
    }

    if (touched && _down && !_longPressEmitted &&
        (uint32_t)(nowMs - _downMs) >= LONG_PRESS_MS) {
        _longPress = true;
        _longPressEmitted = true;
        _pendingSingle = false;
        return;
    }

    if (touched && _down && !_configPressEmitted &&
        (uint32_t)(nowMs - _downMs) >= CONFIG_PRESS_MS) {
        _configPress = true;
        _configPressEmitted = true;
        _pendingSingle = false;
        return;
    }

    if (!touched && _raw) {
        _raw = false;
        _down = false;
        if (!_longPressEmitted && !_doubleTap) {
            _pendingSingle = true;
            _releaseMs = nowMs;
        }
    }
}

bool TouchGesture::wasSingleTap() {
    bool value = _singleTap;
    _singleTap = false;
    return value;
}

bool TouchGesture::wasDoubleTap() {
    bool value = _doubleTap;
    _doubleTap = false;
    return value;
}

bool TouchGesture::wasLongPress() {
    bool value = _longPress;
    _longPress = false;
    return value;
}

bool TouchGesture::wasConfigPress() {
    bool value = _configPress;
    _configPress = false;
    return value;
}
