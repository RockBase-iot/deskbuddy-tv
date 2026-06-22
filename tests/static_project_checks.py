from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def test_platformio_nm_tv_154_environment_exists():
    ini = read("platformio.ini")
    assert "[env:nm-tv-154]" in ini
    assert "board = esp32dev" in ini
    assert "-DNMTV154_BOARD" in ini
    assert "-DUSE_HSPI_PORT" in ini
    assert "board_build.partitions = ota.csv" in ini
    for flag in [
        "-DTFT_WIDTH=240",
        "-DTFT_HEIGHT=240",
        "-DTFT_MOSI=13",
        "-DTFT_SCLK=14",
        "-DTFT_CS=15",
        "-DTFT_DC=2",
        "-DTFT_RST=-1",
        "-DLOAD_GLCD=1",
    ]:
        assert flag in ini


def test_ota_partition_table_and_config_portal_ota():
    partition = read("ota.csv")
    web_config_h = read("src/app/web/web_config.h")
    web_config_cpp = read("src/app/web/web_config.cpp")
    form = read("src/app/web/config_form.cpp")
    main = read("src/main.cpp")
    version = read("src/version.h")

    for token in [
        "ota_0",
        "ota_1",
        "0x180000",
        "littlefs",
    ]:
        assert token in partition
    assert "DESKBUDDY_VERSION" in version
    assert "DESKBUDDY_BUILD" in version
    assert "Update.h" in web_config_cpp
    assert "Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)" in web_config_cpp
    assert "Update.write(data, len)" in web_config_cpp
    assert "Update.end(true)" in web_config_cpp
    assert '"/ota"' in web_config_cpp
    assert '"/api/ota"' in web_config_cpp
    assert "otaProgress" in web_config_h
    assert "otaInProgress" in web_config_h
    assert "OTA Update" in form
    assert "Firmware .bin" in form
    assert "DESKBUDDY_VERSION" in form
    assert "DESKBUDDY_VERSION" in main
    assert "OTA Updating" in main


def test_extracted_modules_have_local_dependencies():
    files = [
        "src/app/weather/weather.cpp",
        "src/app/wifi/wifi_manager.cpp",
        "src/app/web/web_server.cpp",
        "src/app/web/web_config.cpp",
    ]
    for file in files:
        text = read(file)
        assert "app/config/" not in text


def test_open_meteo_model_supports_requested_pages():
    data = read("src/app/weather/weather_data.h")
    weather = read("src/app/weather/weather.cpp")
    assert "precipitation" in data
    assert "precipitation_sum" in data
    assert "precipitation_probability_max" in data
    assert "AirQualityHourly" in data
    assert "hourly" in data
    assert "carbon_monoxide" in weather
    assert "nitrogen_dioxide" in weather
    assert "ozone" in weather
    assert "daily_aqi" not in data


def test_config_model_matches_deskbuddy_requirements():
    config = read("src/config/app_config.h")
    for field in [
        "wifiSsid",
        "wifiPassword",
        "city",
        "latitude",
        "longitude",
        "timezoneOffsetMinutes",
        "eyeColorIndex",
        "roundEyeMode",
    ]:
        assert field in config


def test_nm_tv_154_runtime_modules_exist():
    for path in [
        "src/main.cpp",
        "src/hal/nm_tv_154_hal.h",
        "src/hal/nm_tv_154_hal.cpp",
        "src/input/touch_gesture.h",
        "src/ui/pages.h",
        "src/ui/pages.cpp",
    ]:
        assert (ROOT / path).exists(), path


def test_main_wires_six_pages_and_gestures():
    main = read("src/main.cpp")
    for token in [
        "Page::Eyes",
        "Page::Clock",
        "Page::CurrentWeather",
        "Page::Forecast",
        "Page::HourlyGraph",
        "Page::AirQuality",
        "wasSingleTap",
        "wasDoubleTap",
        "wasLongPress",
        "fetchWeather",
        "fetchAirQuality",
    ]:
        assert token in main


def test_boot_path_is_observable_before_network():
    main = read("src/main.cpp")
    hal = read("src/hal/nm_tv_154_hal.cpp")
    for token in [
        "Serial.println",
        "showBootStage",
        "renderStartupPattern",
        "DeskBuddy boot",
        "WiFi connecting",
        "Config portal",
    ]:
        assert token in main or token in hal
    assert "setBootDiagnostics" in main


def test_config_portal_does_not_block_main_ui_loop():
    main = read("src/main.cpp")
    web_config_h = read("src/app/web/web_config.h")
    web_config_cpp = read("src/app/web/web_config.cpp")

    assert "void process()" in web_config_h
    assert "bool isRunning() const" in web_config_h
    assert "configPortal.process()" in main
    assert "configPortal.start(\"DeskBuddy-Setup\");\n        return;" not in main
    assert "while (!_saved" not in web_config_cpp


def test_config_portal_status_screen_persists_with_countdown():
    main = read("src/main.cpp")
    web_config_cpp = read("src/app/web/web_config.cpp")
    wifi_h = read("src/app/wifi/wifi_manager.h")
    wifi_cpp = read("src/app/wifi/wifi_manager.cpp")

    for token in [
        'CONFIG_AP_SSID = "DeskBuddy"',
        "CONFIG_PORTAL_MS = 180000",
        "configModeActive",
        "configModeStartedMs",
        "drawConfigPortalStatus",
        "exitConfigPortal",
        '"WiFi Config"',
        '"SSID: DeskBuddy"',
        '"AP:"',
        '"%us"',
        "drawConfigCountdown",
        '"Long press exit"',
        "WiFi.softAPIP()",
        "wifiConnected = false",
        "timeSynced = false",
        "loadAppConfig(config)",
        "wifi.stopTimeSync()",
        "tryConnectWifi(true)",
        "clearScreenBeforeNormalUi",
    ]:
        assert token in main
    assert '"DeskBuddy-Setup"' not in main
    assert '"STA:"' not in main
    assert "WiFi.localIP()" not in main.split("static void drawConfigPortalStatus", 1)[1].split("static void refreshWeatherNow", 1)[0]
    assert "if (configModeActive)" in main
    assert "drawConfigPortalStatus(now)" in main
    config_mode_body = main.split("if (configModeActive)", 1)[1].split("if (touch.wasSingleTap())", 1)[0]
    assert "touch.wasConfigPress()" in config_mode_body
    assert "return;" in main.split("if (configModeActive)", 1)[1].split("syncNetworkServicesIfReady", 1)[0]
    sync_body = main.split("static void syncNetworkServicesIfReady()", 1)[1].split("void setup()", 1)[0]
    assert "if (!configModeActive && portalStarted && configPortal.isRunning())" in sync_body
    exit_body = main.split("static void exitConfigPortal", 1)[1].split("static void startConfigPortalIfNeeded", 1)[0]
    assert "clearScreenBeforeNormalUi()" in exit_body
    assert "hal.display().fillScreen(TFT_BLACK)" in main
    assert "pages->markDirty()" in main
    assert "WiFi.disconnect(true)" in web_config_cpp
    assert "WiFi.mode(WIFI_AP)" in web_config_cpp
    assert "WIFI_AP_STA" not in web_config_cpp
    assert "stopTimeSync" in wifi_h
    assert "void WifiManager::stopTimeSync()" in wifi_cpp
    assert "esp_sntp_enabled()" in wifi_cpp
    assert "if (esp_sntp_enabled())" in wifi_cpp


def test_eyes_animation_has_blink_state():
    pages_h = read("src/ui/pages.h")
    pages_cpp = read("src/ui/pages.cpp")

    assert "_nextBlinkMs" in pages_h
    assert "_blinkUntilMs" in pages_h
    assert "drawEyelid" in pages_cpp
    assert "random(" in pages_cpp


def test_eye_expressions_and_config_page_refresh_are_stable():
    config_h = read("src/config/app_config.h")
    config_cpp = read("src/config/app_config.cpp")
    nvs = read("src/config/nvs_table.h")
    pages_h = read("src/ui/pages.h")
    pages_cpp = read("src/ui/pages.cpp")
    main = read("src/main.cpp")
    version = read("src/version.h")

    for token in [
        "Normal = 0",
        "Round",
        "Heart",
        "Star",
        "Sleep",
        "Angry",
        "eyeExpression",
    ]:
        assert token in config_h or token in pages_h or token in pages_cpp
    assert "Happy" not in pages_h
    for token in [
        "EyeExpression::Round",
        "EyeExpression::Heart",
        "EyeExpression::Star",
        "EyeExpression::Sleep",
        "EyeExpression::Angry",
    ]:
        assert token in pages_cpp
    assert "NVS_KEY_EYE_EXPRESSION" in nvs
    assert "prefs.getUChar(NVS_KEY_EYE_EXPRESSION" in config_cpp
    assert "prefs.putUChar(NVS_KEY_EYE_EXPRESSION" in config_cpp
    assert "drawHeart" in pages_cpp
    assert "drawStar" in pages_cpp
    assert "drawSleepEye" in pages_cpp
    assert "drawAngryBrow" in pages_cpp
    assert "drawHappyEye" not in pages_cpp
    assert "EXPRESSION_COUNT = 6" in main
    assert "config.eyeExpression = (config.eyeExpression + 1) % EXPRESSION_COUNT" in main
    assert 'DESKBUDDY_VERSION "0.3.2"' in version
    assert "lastConfigStaticDrawMs" in main
    assert "drawConfigPortalStatic" in main
    assert "drawConfigCountdown" in main
    assert "fillRect(70, 186, 100, 40, TFT_BLACK)" in main
    config_status_body = main.split("static void drawConfigPortalStatus", 1)[1].split("static void drawOtaStatus", 1)[0]
    assert "needsRedraw = false;" in config_status_body


def test_eye_expressions_keep_pupils_blink_and_animation():
    pages_h = read("src/ui/pages.h")
    pages_cpp = read("src/ui/pages.cpp")
    draw_eyes = pages_cpp.split("void Pages::drawEyes", 1)[1].split("void Pages::drawExpressionPupil", 1)[0]

    for token in [
        "drawExpressionPupil",
        "drawHeartPupil",
        "drawStarPupil",
        "drawSleepEye",
        "sleepPhase",
        "drawSleepZ",
        "FACE_W",
        "FACE_H",
    ]:
        assert token in pages_h or token in pages_cpp

    for expression in [
        "EyeExpression::Heart",
        "EyeExpression::Star",
    ]:
        if expression in draw_eyes:
            branch = draw_eyes.split(expression, 1)[1].split("EyeExpression::", 1)[0]
            assert "return;" not in branch

    assert "drawExpressionPupil(" in draw_eyes
    assert "if (blinking)" in draw_eyes
    assert "drawEyelid(x, y, 34)" in draw_eyes
    assert "offX" in draw_eyes
    assert "offY" in draw_eyes
    assert "_eyeSprite.fillSprite(TFT_BLACK)" in draw_eyes
    assert "_eyeSprite.pushSprite(FACE_X, FACE_Y)" in draw_eyes
    assert "_tft.fillRect(0, 38, 240, 132, TFT_BLACK)" not in draw_eyes
    assert "drawSleepZ(_eyeSprite" in draw_eyes
    assert "drawAngryBrow(_eyeSprite" in draw_eyes
    heart_body = pages_cpp.split("void Pages::drawHeart(TFT_eSprite", 1)[1].split("void Pages::drawStar(TFT_eSprite", 1)[0]
    star_body = pages_cpp.split("void Pages::drawStar(TFT_eSprite", 1)[1].split("void Pages::drawSleepEye", 1)[0]
    assert "drawFastHLine" in heart_body
    assert "fillTriangle(cx, cy" in star_body


def test_config_portal_entry_does_not_redraw_eyes_over_status():
    main = read("src/main.cpp")

    manual_entry = main.split("if (touch.wasConfigPress())", 1)[1].split("if (!wifiConnected", 1)[0]
    assert "startConfigPortalIfNeeded()" in manual_entry
    assert "return;" in manual_entry

    auto_entry = main.split("if (!wifiConnected && !portalStarted && !configPortalTimedOut)", 1)[1].split("syncNetworkServicesIfReady", 1)[0]
    assert "startConfigPortalIfNeeded()" in auto_entry
    assert "return;" in auto_entry

    start_body = main.split("static void startConfigPortalIfNeeded()", 1)[1].split("static void tryConnectWifi", 1)[0]
    assert "hal.display().fillScreen(TFT_BLACK)" in start_body


def test_info_pages_use_240x240_safe_content_area():
    pages_h = read("src/ui/pages.h")
    pages_cpp = read("src/ui/pages.cpp")

    assert "CONTENT_TOP" in pages_cpp
    assert "CONTENT_BOTTOM" in pages_cpp
    assert "clearContent()" in pages_h
    assert "void Pages::clearContent()" in pages_cpp
    assert "drawHeader(" in pages_cpp
    assert "_tft.fillScreen(TFT_BLACK);" not in pages_cpp.split("void Pages::drawClock", 1)[1].split("void Pages::drawCurrentWeather", 1)[0]
    for unsafe in [
        'drawString("5 Day Forecast", 10, 8)',
        'drawString("24h Temp / Rain", 10, 8)',
        'drawString("Air + Indoor", 10, 8)',
        'drawString(cfg.city, 10, 10)',
    ]:
        assert unsafe not in pages_cpp


def test_info_page_titles_are_large_and_centered_like_deskbuddy():
    pages_cpp = read("src/ui/pages.cpp")

    assert "drawPageTitle(" in pages_cpp
    assert "void Pages::drawPageTitle" in pages_cpp
    title_body = pages_cpp.split("void Pages::drawPageTitle", 1)[1].split("void Pages::draw(Page", 1)[0]
    assert "_tft.setTextSize(title.length() > 8 ? 2 : 3);" in title_body
    assert "_tft.setTextDatum(MC_DATUM);" in title_body
    for title in [
        'drawPageTitle("CURRENT"',
        'drawPageTitle("FORECAST"',
        'drawPageTitle("HOURLY 24H"',
        'drawPageTitle("AIR QUALITY"',
    ]:
        assert title in pages_cpp
    assert 'drawHeader("Current Weather"' not in pages_cpp
    assert 'drawHeader("5 Day Forecast"' not in pages_cpp
    assert 'drawHeader("24h Temp / Rain"' not in pages_cpp
    assert 'drawHeader("Air + Indoor"' not in pages_cpp


def test_hourly_graph_has_visible_text_labels():
    pages_cpp = read("src/ui/pages.cpp")
    hourly_body = pages_cpp.split("void Pages::drawHourlyGraph", 1)[1].split("void Pages::drawAirQuality", 1)[0]

    for label in [
        '"HOURLY 24H"',
        '"TEMP"',
        '"RAIN"',
        '"NOW"',
        '"+12H"',
        '"+24H"',
    ]:
        assert label in hourly_body
    assert "setCursor" in hourly_body
    assert ".print(" in hourly_body
    assert "TFT_WHITE" in hourly_body
    assert "RAIN_BAR_MAX_H = 52" in hourly_body
    assert "TEMP_GRAPH_H = 68" in hourly_body
    assert "map(constrain(h.precipitation_probability, 0, 100), 0, 100, 0, RAIN_BAR_MAX_H)" in hourly_body


def test_touch_has_5s_config_portal_gesture():
    touch_h = read("src/input/touch_gesture.h")
    touch_cpp = read("src/input/touch_gesture.cpp")
    main = read("src/main.cpp")

    assert "CONFIG_PRESS_MS = 5000" in touch_h
    assert "wasConfigPress" in touch_h
    assert "_configPressEmitted" in touch_h
    assert "_configPress = true" in touch_cpp
    assert "touch.wasConfigPress()" in main
    assert "startConfigPortalIfNeeded" in main


def test_config_supports_custom_color_and_cycle_mode():
    config_h = read("src/config/app_config.h")
    config_cpp = read("src/config/app_config.cpp")
    nvs = read("src/config/nvs_table.h")
    pages_cpp = read("src/ui/pages.cpp")
    form = read("src/app/web/config_form.cpp")
    web_server = read("src/app/web/web_server.cpp")

    for token in [
        "colorCycleMode",
        "customColor565",
    ]:
        assert token in config_h
        assert token in config_cpp
        assert token in web_server
    assert "NVS_KEY_COLOR_CYCLE" in nvs
    assert "NVS_KEY_CUSTOM_COLOR" in nvs
    assert "rgb565ToHtml" in form
    assert "htmlToRgb565" in form
    assert "Color mode" in form
    assert "Custom color" in form
    assert "type='color'" in form
    assert "cfg.colorCycleMode" in pages_cpp
    assert "cfg.customColor565" in pages_cpp


def test_air_quality_layout_uses_readable_sections():
    pages_cpp = read("src/ui/pages.cpp")
    aqi_body = pages_cpp.split("void Pages::drawAirQuality", 1)[1]

    for token in [
        "drawMetricBox",
        '"AQI"',
        '"PARTICLES"',
        '"GASES"',
        "TFT_DARKGREY",
    ]:
        assert token in aqi_body
    assert "drawFastHLine(CONTENT_LEFT, 158" not in aqi_body
    assert "TC_DATUM" in aqi_body


def test_forecast_page_uses_icon_rows_not_long_descriptions():
    pages_h = read("src/ui/pages.h")
    pages_cpp = read("src/ui/pages.cpp")
    forecast_body = pages_cpp.split("void Pages::drawForecast", 1)[1].split("void Pages::drawHourlyGraph", 1)[0]

    assert "drawForecastIcon" in pages_h
    assert "void Pages::drawForecastIcon" in pages_cpp
    assert "drawForecastIcon(d.weather_code" in forecast_body
    assert '"DATE"' in forecast_body
    assert '"TEMP"' in forecast_body
    assert '"RAIN"' in forecast_body
    assert "setTextSize(2)" in forecast_body
    assert "wmo_weather_text(d.weather_code)" not in forecast_body


def test_deskbuddy_complete_feature_tokens():
    main = read("src/main.cpp")
    pages_h = read("src/ui/pages.h")
    pages_cpp = read("src/ui/pages.cpp")
    web_config = read("src/app/web/config_form.cpp")
    web_server = read("src/app/web/web_server.cpp")
    for token in [
        "drawStartupSplash",
        "DeskBuddy",
        "cycleAccentSplash",
        "drawStatusBar",
        "drawPageIndicator",
        "celsiusToDisplay",
        "refreshWeatherNow",
        "needsRedraw",
    ]:
        assert token in main or token in pages_h or token in pages_cpp
    for token in [
        "WiFi SSID",
        "City display name",
        "Latitude",
        "Longitude",
        "Timezone offset",
        "Temperature units",
    ]:
        assert token in web_config
    assert "renderConfigForm" in web_server


def test_config_form_has_location_and_time_shortcuts():
    form = read("src/app/web/config_form.cpp")
    pages_cpp = read("src/ui/pages.cpp")

    for token in [
        "Location preset",
        "City quick search",
        "Cannot find your city?",
        "Latitude and Longitude Finder",
        "https://www.latlong.net/",
        "target='_blank'",
        "rel='noopener'",
        "citySearch",
        "locationPresets",
        "filterLocationPresets",
        "applyLocationPreset",
        "Timezone",
        "Time Format",
        "Date Format",
        "12h AM/PM",
        "24h",
        "YYYY-MM-DD",
        "DD-MM-YYYY",
        "MM-DD-YYYY",
        "YYYY-DD-MM",
        "DD/MM/YYYY",
        "MM/DD/YYYY",
        "YYYY/MM/DD",
        "YYYY/DD/MM",
        "NVS_KEY_TIME_FORMAT",
        "NVS_KEY_DATE_FORMAT",
    ]:
        assert token in form
    assert "Use browser location" not in form
    assert "navigator.geolocation" not in form
    assert "cfg.timeFormat = paramValue(req, NVS_KEY_TIME_FORMAT" in form
    assert "cfg.dateFormat = paramValue(req, NVS_KEY_DATE_FORMAT" in form
    assert "formatDate(" in pages_cpp
    assert "formatWeekday(" in pages_cpp
    assert "%Y" in pages_cpp
    assert "%a" in pages_cpp
    assert 'cfg.dateFormat == "dd-mm-yyyy"' in pages_cpp
    assert "return String(yyyy) + \"-\" + mm + \"-\" + dd" in pages_cpp
    assert "%p" in pages_cpp
    assert "String dateLine = formatWeekday(info) + \" \" + formatDate(cfg, info)" in pages_cpp


if __name__ == "__main__":
    tests = [
        test_platformio_nm_tv_154_environment_exists,
        test_ota_partition_table_and_config_portal_ota,
        test_extracted_modules_have_local_dependencies,
        test_open_meteo_model_supports_requested_pages,
        test_config_model_matches_deskbuddy_requirements,
        test_nm_tv_154_runtime_modules_exist,
        test_main_wires_six_pages_and_gestures,
        test_boot_path_is_observable_before_network,
        test_config_portal_does_not_block_main_ui_loop,
        test_config_portal_status_screen_persists_with_countdown,
        test_eyes_animation_has_blink_state,
        test_eye_expressions_and_config_page_refresh_are_stable,
        test_eye_expressions_keep_pupils_blink_and_animation,
        test_config_portal_entry_does_not_redraw_eyes_over_status,
        test_info_pages_use_240x240_safe_content_area,
        test_info_page_titles_are_large_and_centered_like_deskbuddy,
        test_hourly_graph_has_visible_text_labels,
        test_touch_has_5s_config_portal_gesture,
        test_config_supports_custom_color_and_cycle_mode,
        test_air_quality_layout_uses_readable_sections,
        test_forecast_page_uses_icon_rows_not_long_descriptions,
        test_deskbuddy_complete_feature_tokens,
        test_config_form_has_location_and_time_shortcuts,
    ]
    for test in tests:
        test()
        print(f"PASS {test.__name__}")
