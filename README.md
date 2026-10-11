# Connected Clock for Arduino UNO R4 WiFi

A network-synced clock that runs on the **Arduino UNO R4 WiFi** board alone, with no extra components, wiring or sensors. It fetches the exact time from the Internet over NTP, keeps it in the board's built-in real-time clock (RTC), and scrolls it across the onboard 12×8 LED matrix.

## Features

- **Accurate time** retrieved from `pool.ntp.org` over WiFi
- **Keeps running offline**: the time is stored in the RTC, so the clock continues even if WiFi drops
- **Automatic resync** every 6 hours to correct any drift
- **Automatic daylight saving time** using the European rule (last Sunday of March / last Sunday of October)
- **Scrolling display** on the built-in LED matrix, with status messages during WiFi connection and NTP sync
- **Zero hardware required** beyond the board and a USB cable

## Hardware

- Arduino UNO R4 WiFi
- USB-C cable

That's it.

## Software requirements

- [Arduino IDE](https://www.arduino.cc/en/software) 2.x
- **Arduino UNO R4 Boards** package (Tools → Board → Boards Manager)
- Libraries (Tools → Manage Libraries):
  - `ArduinoGraphics`
  - `NTPClient` by Fabrice Weinberg

`WiFiS3`, `RTC` and `Arduino_LED_Matrix` ship with the UNO R4 board package and need no separate installation.

## Setup

1. Clone or download this repository.
2. Open `horloge_connectee/horloge_connectee.ino` in the Arduino IDE.
3. Edit your WiFi credentials at the top of the sketch:

   ```cpp
   const char SSID[] = "YOUR_WIFI_NAME";
   const char PASS[] = "YOUR_WIFI_PASSWORD";
   ```

4. Select **Arduino UNO R4 WiFi** as the board and choose the correct port.
5. Upload the sketch.
6. Optionally, open the Serial Monitor at **115200 baud** to follow the connection and sync logs.

> **Note:** the UNO R4 WiFi only supports **2.4 GHz** networks. 5 GHz WiFi will not work.

## How it works

1. On boot, the board connects to WiFi and the matrix scrolls `WiFi...` until it succeeds.
2. It queries an NTP server and stores the current **UTC** time in the RTC.
3. In the main loop, the UTC time is read from the RTC, converted to local time (UTC+1 in winter, UTC+2 in summer), and scrolled as `HH:MM`.
4. Every 6 hours, the time is resynced over NTP. Between syncs, the RTC keeps time on its own.

Storing UTC in the RTC and applying the offset only at display time means daylight saving changes take effect at exactly the right moment, without waiting for the next sync.

## Configuration

| Setting | Location | Default |
|---|---|---|
| WiFi credentials | `SSID`, `PASS` | placeholders |
| Resync interval | `INTERVALLE_SYNCHRO` | 6 hours |
| Scroll speed | `textScrollSpeed()` in `afficher()` | 60 (lower is faster) |
| NTP server | `NTPClient ntp(udp, "pool.ntp.org")` | `pool.ntp.org` |
| Time zone offset | `decalage` in `loop()` | UTC+1 / UTC+2 (Central European Time) |

### Using another time zone

The daylight saving logic follows the **European Union** rule. For another EU time zone, change the offsets in `loop()` (for example `0` and `1` for the UK or Portugal). For regions with different DST rules (such as the US) or no DST at all, adapt or replace the `heureEte()` function.

## Troubleshooting

- **Stuck on `WiFi...`**: check the credentials and make sure the network is 2.4 GHz.
- **`NTP ?` keeps scrolling**: the network may block outgoing UDP on port 123 (common on some corporate or school networks).
- **Compilation errors about missing headers**: make sure `ArduinoGraphics` and `NTPClient` are installed and that the UNO R4 board package is selected.

## Ideas for improvements

- Static display using a custom 3×5 pixel font so `HH:MM` fits on the matrix without scrolling
- Show the date or day of the week periodically
- Add weather information from a free API such as Open-Meteo
- Night mode that dims or turns off the display at certain hours

## License

This project is released under the MIT License. See `LICENSE` for details.
