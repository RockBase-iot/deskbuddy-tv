#pragma once

#include <Arduino.h>

#ifndef log_i
#define log_i(tag, fmt, ...) Serial.printf("[I][%s] " fmt "\n", tag, ##__VA_ARGS__)
#endif

#ifndef log_w
#define log_w(tag, fmt, ...) Serial.printf("[W][%s] " fmt "\n", tag, ##__VA_ARGS__)
#endif

#ifndef log_e
#define log_e(tag, fmt, ...) Serial.printf("[E][%s] " fmt "\n", tag, ##__VA_ARGS__)
#endif

