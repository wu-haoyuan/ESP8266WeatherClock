# ESP8266_WeatherClock

  基于ESP8266的天气时钟，天气信息来自和风天气时间信息来自阿里云

 - 和风天气私钥获取[链接](https://dev.heweather.com/docs/start/get-api-key)
 - 和风天气城市代码获取[链接](https://github.com/heweather/LocationList) 或至[城市代码查询网址](https://where.qweather.com/index.html)查询

## Arduino库的安装

- ESP8266库安装  参考太极创客的教程 https://www.bilibili.com/video/BV1L7411c7jw?t=991&p=5
- NTPClient、TFT_eSPI、ArduinoJson以上的库都能在ArduinoIDE中项目-加载库-管理库中找到
- 和风天气库的[链接](https://github.com/Ldufan/ESP8266_Heweather)

## 硬件
- 开发版使用NodeMCU开发板使用有线供电（单纯因为手上没有电池供电的元器件）
- 屏幕使用1.8寸TFT屏，驱动为ST7735，屏幕大小是128*160
- 硬件接线图![接线图](https://user-images.githubusercontent.com/62695662/118657744-5173b800-b81e-11eb-8425-1bc0a8ef24cf.PNG)


## 使用

1. 下载程序后ESP8266会先尝试自动连接WiFi，无法连接WiFi则进入web配网模式。
2. web配网模式需连接ESP8266的WiFi，WiFi名为ESP8266，WiFi密码为123456789
3. 连接WiFi后浏览器打开网页192.168.4.1进入web配网页面，输入所需连接WiFi名称密码及所需查看天气的ID（天气ID查找看上方和风天气城市代码）
4. 完成后点击web界面连接按键并等待一会儿屏幕将自动刷新实时天气等信息



