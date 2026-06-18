# deskbuddy-tv

A desktop weather/mood companion for the **NM-TV-154** (ESP32 + ST7789 240×240 display + capacitive touch key).  
基于 **NM-TV-154** 硬件的桌面天气 / 表情小摆件（ESP32 + ST7789 240×240 显示屏 + 电容触摸按键）。

[English Guide](#english-guide) | [中文说明](#中文说明)

---

## English Guide

### Pages

The device has 6 pages that cycle in order:

| Page | Description |
|------|-------------|
| **1. Eyes** | Animated eyes that blink and look around. |
| **2. Clock** | Current time, date, and the device IP address. |
| **3. Current Weather** | Temperature, condition, feels-like, humidity, wind speed, pressure. |
| **4. Forecast** | 5-day forecast with date, high/low temp, rain probability, and condition. |
| **5. Hourly Graph** | 24-hour temperature line and rain-probability bar chart. |
| **6. Air Quality** | US/EU AQI, PM2.5, PM10, O3, NO2, CO. |

### Touch Gestures

| Gesture | Function |
|---------|----------|
| **Single tap** | Go to the next page. |
| **Double tap** | Cycle the accent/eye color (only when "Color Cycle Mode" is enabled in settings). |
| **Long press ~0.8 s on Eyes page** | Toggle eye shape: circle ↔ rounded rectangle. |
| **Long press 5 s on any page** | Enter WiFi configuration mode. Another 5-second long press exits it. |

### WiFi & Web Configuration

#### First use / no WiFi configured

1. The device starts a hotspot named **DeskBuddy**.
2. Connect your phone or computer to that hotspot (no password by default).
3. Open **http://192.168.4.1** in a browser and fill in:
   - WiFi name / password
   - City display name
   - Latitude / longitude
   - Timezone offset (minutes)
   - Temperature unit (Celsius / Fahrenheit)
   - Eye color, color cycle mode, eye shape, etc.
4. Save. The device will restart and connect to your WiFi.

#### Already connected to WiFi

- The **Clock page** shows the device IP address.
- Open that IP from any device on the same network to change settings.

#### Enter config mode manually

If you need to change WiFi later, long-press the touch key for 5 seconds on any page. The config page will auto-exit after 180 seconds of inactivity and try to reconnect.

### Build & Upload

```bash
pio run -e nm-tv-154
pio run -e nm-tv-154 -t upload --upload-port COM1
```

The default environment `nm-tv-154` targets ESP32Dev + ST7789 240×240.

---

## 中文说明

### 页面说明

设备共 6 个页面，按顺序循环切换：

| 页面 | 说明 |
|------|------|
| **1. 表情页** | 一双会眨眼、会跟随角度转动的动态眼睛。 |
| **2. 时钟页** | 当前时间、日期，以及设备当前 IP 地址。 |
| **3. 当前天气** | 当前温度、天气状况、体感温度、湿度、风速、气压。 |
| **4. 5 天预报** | 未来 5 天的日期、高低温、降雨概率和天气状况。 |
| **5. 24 小时趋势** | 未来 24 小时温度折线与降雨概率柱状图。 |
| **6. 空气质量** | US/EU AQI、PM2.5、PM10、O3、NO2、CO。 |

### 触摸操作

| 手势 | 功能 |
|------|------|
| **单击** | 切换到下一页。 |
| **双击** | 切换主题色 / 眼睛颜色（需开启“颜色循环模式”，可在网页设置中开启）。 |
| **在表情页长按约 0.8 秒** | 切换眼睛形状：圆形 ↔ 圆角方形。 |
| **任意页面长按 5 秒** | 进入 WiFi 配置模式；在配置模式下再长按 5 秒可手动退出。 |

### WiFi 配置与网页设置

#### 首次使用 / 未配置 WiFi 时

1. 上电后若未配置 WiFi，设备会自动开启热点 **DeskBuddy**。
2. 用手机或电脑连接该热点（默认无密码）。
3. 浏览器打开 **http://192.168.4.1**，填写以下信息并保存：
   - WiFi 名称 / 密码
   - 城市显示名
   - 经纬度
   - 时区偏移（分钟）
   - 温度单位（摄氏度 / 华氏度）
   - 眼睛颜色、颜色循环模式、眼睛形状等
4. 保存后设备会自动重启并连接配置的 WiFi。

#### 已连接 WiFi 后

- 在**时钟页**会显示设备当前 IP 地址。
- 用同一局域网的设备访问该 IP，即可随时修改设置。

#### 手动进入配网模式

如果后续需要更换 WiFi，可在**任意页面长按触摸键 5 秒**，设备会重新进入 AP 配网模式。配置页面 180 秒无操作会自动退出并尝试连接已有配置。

### 编译与烧录

```bash
pio run -e nm-tv-154
pio run -e nm-tv-154 -t upload --upload-port COM1
```

默认目标环境为 `nm-tv-154`，对应 ESP32Dev + ST7789 240×240 屏幕。
