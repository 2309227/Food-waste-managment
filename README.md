
# Food waste management

This was create for the joint assesment between CMP101 & CMP104.

This system is to help manage food waste, it has a easy to use and user frendly interface to help track and manage items which perish quicky
these dates are stored in a .txt file stored on the wemos itself. A rotary dial is used for easier month/date inputs and leds count down from 8 days
to the selected food expiry date which provides some visual indication. A buzzer is also implemented to relay a success and error messages to users.
All text and graphic outputs are sent to a OLED screen. 

All the features are listed below along with hardware used and libraries. There is also a section detail parts you need to change to get things to work.

## Features

- Display text via a oled scren.
- Tracking of item dates.
- Modifying dates.
- Clock mode using rtc.
- LED count down from 8 days.
- Dial used to select days and months. 
- Buzzer to indicate when a operation has been successful or unsuccesful.
- Send items to phone using telegram.


## Library's used

- Streaming
- iostream
- sstream
- LittleFS
- tuple
- Adafruit_GFX
- Adafruit_SSD1306
- TM1638plus
- DS3231
- Wire
- UniversalTelegramBot
- ESP8266WiFi
- WiFiClientSecure
- Stream
## Hardware

- Wemos D1
- Adafruit SSD130 - OLED screen
- TM1638plus model 2 - Expansion board
- DS3231 - RTC (Real Time Clock)
- Buzzer
- Potentiometer
- Bread board
## Things that need modified to work


| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `BOT-TOKEN` | `string` | Telegram bot token |
| `CHAT-ID` | `string` | Telegram chat ID |
| `SSID` | `string` | SSID of wireless network |
| `Password` | `string` | Wireless network password.|





## Authors

- [@Cameron Mitchell](https://github.com/2309227)

