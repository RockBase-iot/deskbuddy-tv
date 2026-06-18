#include "nm_tv_154_hal.h"

#include <driver/ledc.h>

static const uint8_t PIN_LCD_BL = 19;
static const uint8_t PIN_LCD_PWR = 21;
static const uint8_t PIN_TOUCH = T9;
static const uint16_t TOUCH_THRESHOLD_PRESS = 90;
static const uint8_t BL_LEDC_CHAN = 0;
static const uint16_t BL_LEDC_FREQ = 5000;
static const uint8_t BL_LEDC_RES = 10;

void NmTv154Hal::begin() {
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println("[boot] DeskBuddy boot");
    Serial.println("[boot] NM-TV-154 HAL begin");
    pinMode(PIN_LCD_PWR, OUTPUT);
    digitalWrite(PIN_LCD_PWR, LOW);
    Serial.println("[boot] LCD power rail enabled");
    delay(50);

    ledcSetup(BL_LEDC_CHAN, BL_LEDC_FREQ, BL_LEDC_RES);
    ledcAttachPin(PIN_LCD_BL, BL_LEDC_CHAN);
    ledcWrite(BL_LEDC_CHAN, 1023);
    Serial.println("[boot] Backlight PWM attached");

    _display.init();
    _display.setRotation(0);
    _display.invertDisplay(true);
    _display.fillScreen(TFT_BLACK);
    setBacklight(4);
    Serial.println("[boot] ST7789 initialized");
    if (_bootDiagnostics) {
        renderStartupPattern("HAL OK");
    }
}

void NmTv154Hal::update() {
    _touched = touchRead(PIN_TOUCH) < TOUCH_THRESHOLD_PRESS;
}

void NmTv154Hal::setBacklight(uint8_t level) {
    if (level > 4) level = 4;
    uint32_t duty = (uint32_t)((4u - level) * 200u);
    ledcWrite(BL_LEDC_CHAN, duty);
}

void NmTv154Hal::screenOn(bool on) {
    if (on) setBacklight(4);
    else ledcWrite(BL_LEDC_CHAN, 1023);
}

void NmTv154Hal::renderStartupPattern(const char *label) {
    _display.fillScreen(TFT_BLACK);
    _display.fillRect(0, 0, 80, 240, TFT_RED);
    _display.fillRect(80, 0, 80, 240, TFT_GREEN);
    _display.fillRect(160, 0, 80, 240, TFT_BLUE);
    _display.fillRect(0, 92, 240, 56, TFT_BLACK);
    _display.setTextDatum(MC_DATUM);
    _display.setTextSize(2);
    _display.setTextColor(TFT_WHITE, TFT_BLACK);
    _display.drawString(label ? label : "DeskBuddy", 120, 120);
    Serial.printf("[boot] renderStartupPattern: %s\n", label ? label : "DeskBuddy");
}
