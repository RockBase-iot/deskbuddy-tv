#include "config_form.h"

#include "config/nvs_table.h"
#include "version.h"

static String htmlEscape(const String &in) {
    String out;
    out.reserve(in.length());
    for (char c : in) {
        switch (c) {
            case '&': out += F("&amp;"); break;
            case '<': out += F("&lt;"); break;
            case '>': out += F("&gt;"); break;
            case '"': out += F("&quot;"); break;
            default: out += c; break;
        }
    }
    return out;
}

static String paramValue(AsyncWebServerRequest *req, const char *name, const String &fallback = "") {
    if (!req->hasParam(name, true)) return fallback;
    return req->getParam(name, true)->value();
}

static String rgb565ToHtml(uint16_t color) {
    uint8_t r = ((color >> 11) & 0x1F) << 3;
    uint8_t g = ((color >> 5) & 0x3F) << 2;
    uint8_t b = (color & 0x1F) << 3;
    char out[8];
    snprintf(out, sizeof(out), "#%02X%02X%02X", r, g, b);
    return String(out);
}

static uint16_t htmlToRgb565(const String &html, uint16_t fallback) {
    if (html.length() != 7 || html[0] != '#') return fallback;
    char *end = nullptr;
    uint32_t rgb = strtoul(html.substring(1).c_str(), &end, 16);
    if (end == nullptr || *end != '\0') return fallback;
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static String selected(const String &actual, const char *expected) {
    return actual == expected ? " selected" : "";
}

static String selectedInt(int actual, int expected) {
    return actual == expected ? " selected" : "";
}

String renderConfigForm(const AppConfig &cfg, const char *message) {
    String html;
    html.reserve(8400);
    html += F("<!doctype html><html><head><meta charset='utf-8'>"
              "<meta name='viewport' content='width=device-width,initial-scale=1'>"
              "<title>DeskBuddy Setup</title><style>"
              "body{font-family:system-ui,sans-serif;max-width:420px;margin:0 auto;padding:18px;background:#050505;color:#e8f7ff}"
              "h1{font-size:24px}h2{font-size:16px;margin:22px 0 2px;color:#e8f7ff}label{display:block;margin-top:12px;font-size:13px;color:#9dd7ff}"
              "input,select{width:100%;box-sizing:border-box;padding:10px;border:1px solid #3a6b80;border-radius:6px;background:#111;color:#fff}"
              "button{width:100%;margin-top:18px;padding:12px;border:0;border-radius:6px;background:#35b5ff;color:#00131d;font-weight:700}"
              "button.secondary{margin-top:10px;background:#1d3340;color:#d8f4ff;border:1px solid #3a6b80}"
              ".msg{color:#8dffbd}.hint{font-size:12px;color:#9aa;line-height:1.35}.hint a{color:#35b5ff;text-decoration:none;font-weight:700}.row{display:grid;grid-template-columns:1fr 1fr;gap:10px}</style>"
              "</head><body><h1>DeskBuddy Setup</h1>");
    if (message && message[0]) {
        html += F("<p class='msg'>");
        html += message;
        html += F("</p>");
    }
    html += F("<form method='post' action='/save'>");
    html += F("<label>WiFi SSID</label><input name='wifi_ssid' maxlength='32' value='");
    html += htmlEscape(cfg.wifiSsid);
    html += F("'><label>WiFi Password</label><input name='wifi_pass' type='password' maxlength='64' placeholder='Leave blank to keep current'>");
    html += F("<h2>Location</h2><label>City quick search</label><input id='citySearch' placeholder='Type city name, e.g. Shanghai' oninput='filterLocationPresets()'>"
              "<label>Location preset</label><select id='locationPreset' onchange='applyLocationPreset()'>"
              "<option value=''>Manual / keep current</option>"
              "<option value='Shanghai|31.2304|121.4737|480'>Shanghai UTC+8</option>"
              "<option value='Beijing|39.9042|116.4074|480'>Beijing UTC+8</option>"
              "<option value='Guangzhou|23.1291|113.2644|480'>Guangzhou UTC+8</option>"
              "<option value='Shenzhen|22.5431|114.0579|480'>Shenzhen UTC+8</option>"
              "<option value='Hangzhou|30.2741|120.1551|480'>Hangzhou UTC+8</option>"
              "<option value='Nanjing|32.0603|118.7969|480'>Nanjing UTC+8</option>"
              "<option value='Chengdu|30.5728|104.0668|480'>Chengdu UTC+8</option>"
              "<option value='Wuhan|30.5928|114.3055|480'>Wuhan UTC+8</option>"
              "<option value='Xi'an|34.3416|108.9398|480'>Xi'an UTC+8</option>"
              "<option value='Hong Kong|22.3193|114.1694|480'>Hong Kong UTC+8</option>"
              "<option value='Taipei|25.0330|121.5654|480'>Taipei UTC+8</option>"
              "<option value='Tokyo|35.6762|139.6503|540'>Tokyo UTC+9</option>"
              "<option value='Seoul|37.5665|126.9780|540'>Seoul UTC+9</option>"
              "<option value='Singapore|1.3521|103.8198|480'>Singapore UTC+8</option>"
              "<option value='Bangkok|13.7563|100.5018|420'>Bangkok UTC+7</option>"
              "<option value='Sydney|-33.8688|151.2093|600'>Sydney UTC+10</option>"
              "<option value='Paris|48.8566|2.3522|60'>Paris UTC+1</option>"
              "<option value='Berlin|52.5200|13.4050|60'>Berlin UTC+1</option>"
              "<option value='London|51.5072|-0.1276|0'>London UTC+0</option>"
              "<option value='New York|40.7128|-74.0060|-300'>New York UTC-5</option>"
              "<option value='Chicago|41.8781|-87.6298|-360'>Chicago UTC-6</option>"
              "<option value='Los Angeles|34.0522|-118.2437|-480'>Los Angeles UTC-8</option>"
              "</select>"
              "<p class='hint'>Search or choose a preset to fill city, latitude, longitude, and timezone. Manual values are still supported.</p>");
    html += F("<label>City display name</label><input id='city' name='city' maxlength='48' value='");
    html += htmlEscape(cfg.city);
    html += F("'><p class='hint'>Cannot find your city? Enter coordinates manually below. <a href='https://www.latlong.net/' target='_blank' rel='noopener'>Latitude and Longitude Finder</a></p>"
              "<div class='row'><div><label>Latitude</label><input id='lat' name='lat' inputmode='decimal' value='");
    html += String(cfg.latitude, 6);
    html += F("'></div><div><label>Longitude</label><input id='lon' name='lon' inputmode='decimal' value='");
    html += String(cfg.longitude, 6);
    html += F("'></div></div><label>Timezone</label><select id='tz' name='tz_offset_min'>");
    const int zones[] = {-720, -660, -600, -540, -480, -420, -360, -300, -240, -180, -120, -60,
                         0, 60, 120, 180, 210, 240, 270, 300, 330, 345, 360, 390, 420, 480, 540, 570, 600, 660, 720, 780, 840};
    for (uint8_t i = 0; i < sizeof(zones) / sizeof(zones[0]); ++i) {
        int z = zones[i];
        int absMin = abs(z);
        char label[28];
        snprintf(label, sizeof(label), "UTC%+d:%02d (%d)", z / 60, absMin % 60, z);
        html += String("<option value='") + z + "'" + selectedInt(cfg.timezoneOffsetMinutes, z) + ">" + label + "</option>";
    }
    html += F("</select><p class='hint'>Timezone offset is stored as minutes from UTC, for example China UTC+8 = 480.</p>");
    html += F("<h2>Time</h2><label>Time Format</label><select name='time_format'>");
    html += String("<option value='24h'") + selected(cfg.timeFormat, "24h") + ">24h</option>";
    html += String("<option value='12h'") + selected(cfg.timeFormat, "12h") + ">12h AM/PM</option>";
    html += F("</select><label>Date Format</label><select name='date_format'>");
    html += String("<option value='yyyy-mm-dd'") + selected(cfg.dateFormat, "yyyy-mm-dd") + ">YYYY-MM-DD</option>";
    html += String("<option value='dd-mm-yyyy'") + selected(cfg.dateFormat, "dd-mm-yyyy") + ">DD-MM-YYYY</option>";
    html += String("<option value='mm-dd-yyyy'") + selected(cfg.dateFormat, "mm-dd-yyyy") + ">MM-DD-YYYY</option>";
    html += String("<option value='yyyy-dd-mm'") + selected(cfg.dateFormat, "yyyy-dd-mm") + ">YYYY-DD-MM</option>";
    html += String("<option value='dd/mm/yyyy'") + selected(cfg.dateFormat, "dd/mm/yyyy") + ">DD/MM/YYYY</option>";
    html += String("<option value='mm/dd/yyyy'") + selected(cfg.dateFormat, "mm/dd/yyyy") + ">MM/DD/YYYY</option>";
    html += String("<option value='yyyy/mm/dd'") + selected(cfg.dateFormat, "yyyy/mm/dd") + ">YYYY/MM/DD</option>";
    html += String("<option value='yyyy/dd/mm'") + selected(cfg.dateFormat, "yyyy/dd/mm") + ">YYYY/DD/MM</option>";
    html += F("</select>");
    html += F("<h2>Display</h2><label>Temperature units</label><select name='units_temp'>");
    html += String("<option value='celsius'") + (cfg.unitsTemp == "celsius" ? " selected" : "") + ">Celsius</option>";
    html += String("<option value='fahrenheit'") + (cfg.unitsTemp == "fahrenheit" ? " selected" : "") + ">Fahrenheit</option>";
    html += F("</select><label>Color mode</label><select name='color_cycle'>");
    html += String("<option value='1'") + (cfg.colorCycleMode ? " selected" : "") + ">Color rotation</option>";
    html += String("<option value='0'") + (!cfg.colorCycleMode ? " selected" : "") + ">Custom color</option>";
    html += F("</select><label>Custom color</label><input name='custom_color' type='color' value='");
    html += rgb565ToHtml(cfg.customColor565);
    html += F("'><button type='submit'>Save and restart</button></form>");
    html += F("<h2>OTA Update</h2><form method='post' action='/api/ota' enctype='multipart/form-data'>"
              "<label>Firmware .bin</label><input name='firmware' type='file' accept='.bin,application/octet-stream'>"
              "<button type='submit'>Upload firmware</button></form>"
              "<p class='hint'>OTA is available only in WiFi Config mode. Serial upload remains supported.</p>");
    html += F("<p class='hint'>Device version: ");
    html += DESKBUDDY_VERSION;
    html += F(" / ");
    html += DESKBUDDY_BUILD;
    html += F("</p><p class='hint'>JSON API: GET/POST /api/config, POST /api/system/restart.</p>");
    html += F("<script>"
              "const locationPresets=document.getElementById('locationPreset');"
              "const citySearch=document.getElementById('citySearch');"
              "const presetOptions=Array.from(locationPresets.options);"
              "function filterLocationPresets(){const q=citySearch.value.trim().toLowerCase();"
              "locationPresets.innerHTML='';presetOptions.forEach(o=>{if(!q||!o.value||o.text.toLowerCase().includes(q)||o.value.toLowerCase().includes(q)){locationPresets.add(o.cloneNode(true));}});}"
              "function applyLocationPreset(){const v=locationPresets.value;if(!v)return;const p=v.split('|');"
              "document.getElementById('city').value=p[0];document.getElementById('lat').value=Number(p[1]).toFixed(6);"
              "document.getElementById('lon').value=Number(p[2]).toFixed(6);document.getElementById('tz').value=p[3];}"
              "</script>");
    html += F("</body></html>");
    return html;
}

void applyFormToConfig(AsyncWebServerRequest *req, AppConfig &cfg) {
    cfg.wifiSsid = paramValue(req, NVS_KEY_WIFI_SSID, cfg.wifiSsid).substring(0, 32);
    String pass = paramValue(req, NVS_KEY_WIFI_PASSWORD);
    if (pass.length() > 0) cfg.wifiPassword = pass.substring(0, 64);
    cfg.city = paramValue(req, NVS_KEY_CITY, cfg.city).substring(0, 48);
    cfg.latitude = paramValue(req, NVS_KEY_LAT, String(cfg.latitude, 6)).toDouble();
    cfg.longitude = paramValue(req, NVS_KEY_LON, String(cfg.longitude, 6)).toDouble();
    cfg.timezoneOffsetMinutes = paramValue(req, NVS_KEY_TIMEZONE, String(cfg.timezoneOffsetMinutes)).toInt();
    cfg.timeFormat = paramValue(req, NVS_KEY_TIME_FORMAT, cfg.timeFormat);
    cfg.dateFormat = paramValue(req, NVS_KEY_DATE_FORMAT, cfg.dateFormat);
    cfg.unitsTemp = paramValue(req, NVS_KEY_UNITS_TEMP, cfg.unitsTemp);
    cfg.colorCycleMode = paramValue(req, NVS_KEY_COLOR_CYCLE, cfg.colorCycleMode ? "1" : "0") != "0";
    cfg.customColor565 = htmlToRgb565(paramValue(req, NVS_KEY_CUSTOM_COLOR, rgb565ToHtml(cfg.customColor565)), cfg.customColor565);
}
