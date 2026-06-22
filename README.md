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
| **1. Eyes** | Animated eyes with 6 expressions that blink and look around. |
| **2. Clock** | Current time, date, and the device IP address. |
| **3. Current Weather** | Temperature, condition, feels-like, humidity, wind speed, pressure. |
| **4. Forecast** | 5-day forecast with date, high/low temp, rain probability, and condition. |
| **5. Hourly Graph** | 24-hour temperature line and rain-probability bar chart. |
| **6. Air Quality** | US/EU AQI, PM2.5, PM10, O3, NO2, CO. |

### Expressions

On the **Eyes page**, long-press to cycle through the following expressions:

1. **Normal** – classic oval eye with round pupil  
2. **Round** – fully round eye  
3. **Heart** – heart-shaped pupil  
4. **Star** – star-shaped pupil  
5. **Sleep** – closed eye with floating "Z"  
6. **Angry** – angry eyebrow

### Touch Gestures

| Gesture | Function |
|---------|----------|
| **Single tap** | Go to the next page. |
| **Double tap** | Cycle the accent/eye color (only when **Color rotation** mode is enabled in settings). |
| **Long press ~0.8 s on Eyes page** | Cycle to the next eye expression. |
| **Long press 5 s on any page** | Enter **WiFi Config / OTA mode**. Another 5-second long press exits it. |

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
   - Color mode, custom color, etc.
4. Save. The device will restart and connect to your WiFi.

#### Already connected to WiFi

- The **Clock page** shows the device IP address.
- Open that IP from any device on the same network to change settings.

#### Enter config mode manually

If you need to change WiFi later, long-press the touch key for 5 seconds on any page. The config page will auto-exit after 180 seconds of inactivity and try to reconnect.

### OTA Update

OTA firmware upload is **only available while the device is in WiFi Config mode**:

1. Long-press the touch key for 5 seconds to enter WiFi Config mode.
2. Connect to the **DeskBuddy** hotspot and open **http://192.168.4.1**.
3. Scroll to **OTA Update**, select the compiled `firmware.bin`, and click **Upload firmware**.
4. Wait for the upload to complete; the device will restart automatically.

> Serial upload (`pio run -e nm-tv-154 -t upload`) is still supported at any time.

### Build & Upload

```bash
pio run -e nm-tv-154
pio run -e nm-tv-154 -t upload --upload-port COM1
```

The default environment `nm-tv-154` targets ESP32Dev + ST7789 240×240.

After `pio run`, a single merged image is also generated at:

```
.pio/build/nm-tv-154/full_image.bin
```

You can flash it directly from address `0x0` (includes bootloader, partition table and firmware):

```bash
esptool.py --chip esp32 write_flash 0x0 .pio/build/nm-tv-154/full_image.bin
```

---

## 中文说明

### 页面说明

设备共 6 个页面，按顺序循环切换：

| 页面 | 说明 |
|------|------|
| **1. 表情页** | 支持 6 种动态表情，会眨眼、会跟随角度转动。 |
| **2. 时钟页** | 当前时间、日期，以及设备当前 IP 地址。 |
| **3. 当前天气** | 当前温度、天气状况、体感温度、湿度、风速、气压。 |
| **4. 5 天预报** | 未来 5 天的日期、高低温、降雨概率和天气状况。 |
| **5. 24 小时趋势** | 未来 24 小时温度折线与降雨概率柱状图。 |
| **6. 空气质量** | US/EU AQI、PM2.5、PM10、O3、NO2、CO。 |

### 表情说明

在**表情页**长按可循环切换以下 6 种表情：

1. **普通** – 经典椭圆眼 + 圆形瞳孔  
2. **圆眼** – 正圆形眼睛  
3. **爱心** – 爱心形瞳孔  
4. **星星** – 星星形瞳孔  
5. **睡觉** – 闭眼并飘出 "Z"  
6. **生气** – 生气眉毛

### 触摸操作

| 手势 | 功能 |
|------|------|
| **单击** | 切换到下一页。 |
| **双击** | 切换主题色 / 眼睛颜色（仅在网页设置中开启“颜色循环”模式时有效）。 |
| **在表情页长按约 0.8 秒** | 切换到下一个表情。 |
| **任意页面长按 5 秒** | 进入 **WiFi 配置 / OTA 升级模式**；在配置模式下再长按 5 秒可手动退出。 |

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
   - 颜色模式、自定义颜色等
4. 保存后设备会自动重启并连接配置的 WiFi。

#### 已连接 WiFi 后

- 在**时钟页**会显示设备当前 IP 地址。
- 用同一局域网的设备访问该 IP，即可随时修改设置。

#### 手动进入配网模式

如果后续需要更换 WiFi，可在**任意页面长按触摸键 5 秒**，设备会重新进入 AP 配网模式。配置页面 180 秒无操作会自动退出并尝试连接已有配置。

### OTA 升级

OTA 固件上传**仅在设备处于 WiFi 配置模式（AP 模式）时可用**：

1. 长按触摸键 5 秒进入 WiFi 配置模式。
2. 连接 **DeskBuddy** 热点，打开 **http://192.168.4.1**。
3. 在页面底部找到 **OTA Update**，选择编译好的 `firmware.bin`，点击上传。
4. 等待上传完成，设备会自动重启。

> 任何时候仍可使用串口烧录：`pio run -e nm-tv-154 -t upload`。

### 编译与烧录

```bash
pio run -e nm-tv-154
pio run -e nm-tv-154 -t upload --upload-port COM1
```

默认目标环境为 `nm-tv-154`，对应 ESP32Dev + ST7789 240×240 屏幕。

`pio run` 完成后会额外生成一个合并镜像：

```
.pio/build/nm-tv-154/full_image.bin
```

可直接从地址 `0x0` 写入（已包含 bootloader、分区表和固件）：

```bash
esptool.py --chip esp32 write_flash 0x0 .pio/build/nm-tv-154/full_image.bin
```

> `full_image.bin` 仅用于串口全量烧录，不用于 OTA 升级。
