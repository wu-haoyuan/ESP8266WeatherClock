# ESP8266_WeatherClock

  基于ESP8266的天气时钟，天气信息来自和风天气时间信息来自阿里云及苏宁授时

 - 和风天气私钥获取[链接](https://dev.heweather.com/docs/start/get-api-key)
 - 和风天气城市代码获取[链接](https://github.com/heweather/LocationList) 或至[城市代码查询网址](https://where.qweather.com/index.html)查询

## Arduino库的安装

- ESP8266库安装  参考太极创客的教程 https://www.bilibili.com/video/BV1L7411c7jw?t=991&p=5
- NTPClient、TFT_eSPI、ArduinoJson以上的库都能在ArduinoIDE中项目-加载库-管理库中找到
- 和风天气库的[链接](https://github.com/Ldufan/ESP8266_Heweather)

## 硬件
- 开发版使用NodeMCU开发板使用有线供电（单纯因为手上没有电池供电的元器件）
- 屏幕使用1.8寸TFT屏，驱动为ST7735，屏幕大小是128*160
- 硬件接线图![屏幕截图 2021-06-05 154525](https://user-images.githubusercontent.com/62695662/120884343-46d66280-c615-11eb-8fc6-8e34a5da1bfc.jpg)

## 使用

1. 下载程序后ESP8266会先尝试自动连接WiFi，无法连接WiFi则进入web配网模式。
2. web配网模式需连接ESP8266的WiFi，WiFi名为ESP8266，WiFi密码为123456789
3. 连接WiFi后浏览器打开网页192.168.4.1进入web配网页面，输入所需连接WiFi名称密码及所需查看天气的ID（天气ID查找看上方和风天气城市代码）
4. 完成后点击web界面连接按键并等待一会儿屏幕将自动刷新实时天气等信息
5. ![IMG_20210518_212511](https://user-images.githubusercontent.com/62695662/118659722-28542700-b820-11eb-9242-75e5715bf7a4.jpg)

## 参考资料
1. https://github.com/Ldufan/ESP8266_Heweather
2. https://blog.csdn.net/weixin_40660408/article/details/89284779
3. https://blog.csdn.net/qq_35274097/article/details/111501933?utm_medium=distribute.pc_relevant.none-task-blog-baidujs_utm_term-0&spm=1001.2101.3001.4242
4. https://blog.csdn.net/weixin_42880082/article/details/114366362?ops_request_misc=&request_id=&biz_id=102&utm_term=ESP8266web%E9%85%8D%E7%BD%91&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-6-.pc_search_result_no_baidu_js&spm=1018.2226.3001.4187

---

## 2021/06/05更新

- 增加屏幕背光控制设置为凌晨00：10熄灭早上8：00点亮
- 修改部分代码增加WiFi休眠减少电量消耗
- 添加TFT屏与NodeMCU转接板及3D打印外壳
