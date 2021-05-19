#include <ESP8266_Heweather.h>							//和风天气获取的支持库
#include <NTPClient.h>									    //时间获取的支持库
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <User_Setup.h>                     //根据屏幕驱动及大小修改
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "ESP8266"
#define STAPSK  "123456789"
#endif

#define Debug 0   //debug标志位
#define LineWidth 2	//分割线宽度

const char* ssid = STASSID;								//wifi名称
const char* password = STAPSK;							//wifi密码
IPAddress apIP(192, 168, 4, 1);							//web配网固定IP
ESP8266WebServer server(80);
bool LED_Flag = false;

String str =
"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\"content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\"content=\"ie=edge\"><title>ESP8266网页配网</title></head><body><form name=\"my\">WiFi名称：<input type=\"text\"name=\"s\"placeholder=\"请输入您WiFi的名称\"id=\"aa\"><br>WiFi密码：<input type=\"text\"name=\"p\"placeholder=\"请输入您WiFi的密码\"id=\"bb\"><br>城市代码：<input type=\"text\"name=\"p\"placeholder=\"请输入您的城市代码\"id=\"cc\"><br><input type=\"button\"value=\"连接\"onclick=\"wifi()\"></form><script language=\"javascript\">function wifi(){var ssid=my.s.value;var password=bb.value;var location=cc.value;var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/HandleVal?ssid=\"+ssid+\"&password=\"+password+\"&location=\"+location,true);xmlhttp.send()}</script></body></html>";
//web配网HTML代码

String last_update, time_string, minu_string, hours_string;
int hours, minu, sece, last_minu = 0, week, last_week = 0;
int begin_flag = 1, weather_flag = 1, break_flag = 0;

int days_begin, days_end, days_flag,httpCode;
String days, days_char; 

WiFiUDP ntpUDP;											//时间获取实例化
WeatherNow weatherNow;									//天气获取实例化
WeatherForecast weatherForecast;
AirQuality airQuality;

HTTPClient http;
String GetUrl = "http://quan.suning.com/getSysTime.do";//用苏宁的授时获取日期
String response;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);	//使用阿里云的校时服务器

String UserKey = "xxxxxxxxxxx";	//私钥获取地址 https://dev.heweather.com/docs/start/get-api-key
String Location = "101010100";							// 城市代码 https://github.com/heweather/LocationList ,表中的Location_ID ，或至 https://where.qweather.com/index.html 查询
String Unit = "m";										// 公制-m/英制-i
String Lang = "zh";										// 语言 英文-en/中文-zh

TFT_eSPI tft = TFT_eSPI();

// Include the sketch header file that contains the image stored as an array of bytes
// More than one image array could be stored in each header file.
#include "jpeg1.h"

void set_String(int a, int b, String str)//a是长度，b开始存str的值
{
	EEPROM.write(a, str.length());
	for (int j = 0; j < str.length(); j++)
	{
		EEPROM.write(b + j, str[j]);
	}
	EEPROM.commit();
}

String get_String(int a, int b)
{
	String data = "";
	for (int j = 0; j < a; j++)
	{
		data += char(EEPROM.read(b + j));
	}
	return data;
}

/*****************************************************
 * 函数名称：handleRoot()
 * 函数说明：客户端请求回调函数
 * 参数说明：无
******************************************************/
void handleRoot() {
	server.send(200, "text/html", str);
}
/*****************************************************
 * 函数名称：HandleVal()
 * 函数说明：对客户端请求返回值处理
 * 参数说明：无
******************************************************/
void HandleVal()
{
	String wifis = server.arg("ssid"); //从JavaScript发送的数据中找ssid的值
	String wifip = server.arg("password"); //从JavaScript发送的数据中找password的值
	String wifil = server.arg("location"); //从JavaScript发送的数据中找location的值
	Location = server.arg("location");
	set_String(0, 1, Location);
	#if Debug
		Serial.println(wifis); Serial.println(wifip); Serial.println(wifil);
	#endif
	tft.println(wifis);	tft.println(wifip);	 tft.println(wifil);
	WiFi.begin(wifis, wifip);
}
/*****************************************************
 * 函数名称：handleNotFound()
 * 函数说明：响应失败函数
 * 参数说明：无
******************************************************/
void handleNotFound() {
	digitalWrite(LED_BUILTIN, 0);
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
	digitalWrite(LED_BUILTIN, 1);
}
/*****************************************************
 * 函数名称：autoConfig()
 * 函数说明：自动连接WiFi函数
 * 参数说明：无
 * 返回值说明:true：连接成功 false：连接失败
******************************************************/
bool autoConfig()
{
	WiFi.mode(WIFI_STA);
	WiFi.begin();
	#if Debug
		Serial.println();
		Serial.print("AutoConfig Waiting......");
	#endif
	tft.println("AutoConfig Waiting......");
	for (int i = 0; i < 20; i++)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			#if Debug
				Serial.println("AutoConfig Success");
			#endif
			tft.println("AutoConfig Success");
			#if Debug
				Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
			#endif
			tft.print("SSID:");
			tft.println(WiFi.SSID().c_str());
			#if Debug
				Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
			#endif
			tft.print("PSW:");
			tft.println(WiFi.psk().c_str());
			WiFi.printDiag(Serial);
			return true;
			//break;
		}
		else
		{
			#if Debug
				Serial.print(".");
			#endif
			LED_Flag = !LED_Flag;
			if (LED_Flag)
				digitalWrite(LED_BUILTIN, HIGH);
			else
				digitalWrite(LED_BUILTIN, LOW);
			delay(500);
		}
	}
	#if Debug
		Serial.println("AutoConfig Faild!");
	#endif
	return false;
	//WiFi.printDiag(Serial);
}
/*****************************************************
 * 函数名称：htmlConfig()
 * 函数说明：web配置WiFi函数
 * 参数说明：无
******************************************************/
void htmlConfig()
{
	WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
	WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
	digitalWrite(LED_BUILTIN, LOW);
	WiFi.softAP(ssid, password);
	#if Debug
		Serial.println("AP设置完成");
	#endif

	IPAddress myIP = WiFi.softAPIP();
	#if Debug
		Serial.print("AP IP address: ");
	#endif
	tft.setTextFont(1);
	tft.print("AP IP address:");
	#if Debug
		Serial.println(myIP);
	#endif
	tft.println(myIP);
	tft.setTextFont(2);

	if (MDNS.begin("esp8266")) {
	#if Debug
			Serial.println("MDNS responder started");
	#endif
	}

	server.on("/", handleRoot);
	server.on("/HandleVal", HTTP_GET, HandleVal);
	server.onNotFound(handleNotFound);//请求失败回调函数

	server.begin();//开启服务器
	#if Debug
		Serial.println("HTTP server started");
	#endif
	while (1)
	{
		server.handleClient();
		MDNS.update();
		if (WiFi.status() == WL_CONNECTED)
		{
			#if Debug
				Serial.println("HtmlConfig Success");
				Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
			#endif
			tft.print("SSID:");
			tft.println(WiFi.SSID().c_str());
			#if Debug
						Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
			#endif
			tft.print("PSW:");
			tft.println(WiFi.psk().c_str());
			#if Debug
				Serial.println("HTML连接成功");
			#endif
			break;
		}
	}
	digitalWrite(LED_BUILTIN, HIGH);
}

void printString(int32_t x,int32_t y,int32_t size,const String &string)
{
	tft.setTextSize(size);							
	tft.setCursor(x, y);							//设置光标位置
	tft.println(string);
}


//####################################################################################################
// Setup
//####################################################################################################
void setup() {

  Serial.begin(115200);
  EEPROM.begin(1024);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
   
  tft.begin();
  tft.setSwapBytes(true);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);							//选择字体1（太潦草）、2（一点潦草）、4（1号就很大）、6（超屏幕了）、7（也超了）、8
  tft.setTextColor(TFT_WHITE, TFT_BLACK);		//设置字体颜色，背景色
  tft.setCursor(0,0);
  tft.println("Author: W.H.Y");

  bool wifiConfig = autoConfig();

  if (wifiConfig == false)
	htmlConfig();//HTML配网
   

  Location = get_String(EEPROM.read(0), 1);
	#if Debug
	  Serial.println(Location);
	#endif
  weatherNow.config(UserKey, Location, Unit, Lang);			// 配置实时天气请求信息
  weatherForecast.config(UserKey, Location, Unit, Lang);	// 配置天气预报请求信息
  airQuality.config(UserKey, Location, Unit, Lang);			// 配置空气质量请求信息
  
  http.setTimeout(5000);
  http.begin(GetUrl);										//开启苏宁的时间获取	
  timeClient.begin();										//开启阿里云的时间获取
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  tft.fillScreen(TFT_BLACK);
}

//####################################################################################################
// Main loop
//####################################################################################################
void update_picture(int Weather_daima,int32_t x,int32_t y)
{
	switch (Weather_daima)
	{
		case 100:  tft.pushImage(x, y, 43, 43, Weather_100_43);		break;
		case 101:  tft.pushImage(x, y, 43, 43, Weather_101_43);		break;
		case 102:  tft.pushImage(x, y, 43, 43, Weather_102_43);		break;
		case 103:  tft.pushImage(x, y, 43, 43, Weather_103_43);		break;
		case 104:  tft.pushImage(x, y, 43, 43, Weather_104_43);		break;
		case 150:  tft.pushImage(x, y, 43, 43, Weather_150_43);		break;
		case 153:  tft.pushImage(x, y, 43, 43, Weather_153_43);		break;
		case 154:  tft.pushImage(x, y, 43, 43, Weather_154_43);		break;
		case 300:  tft.pushImage(x, y, 43, 43, Weather_300_43);		break;
		case 301:  tft.pushImage(x, y, 43, 43, Weather_301_43);		break;
		case 302:  tft.pushImage(x, y, 43, 43, Weather_302_43);		break;
		case 304:  tft.pushImage(x, y, 43, 43, Weather_304_43);		break;
		case 305:  tft.pushImage(x, y, 43, 43, Weather_305_43);		break;
		case 306:  tft.pushImage(x, y, 43, 43, Weather_306_43);		break;
		case 307:  tft.pushImage(x, y, 43, 43, Weather_307_43);		break;
		case 308:  tft.pushImage(x, y, 43, 43, Weather_308_43);		break;
		case 309:  tft.pushImage(x, y, 43, 43, Weather_309_43);		break;
		case 310:  tft.pushImage(x, y, 43, 43, Weather_310_43);		break;
		case 311:  tft.pushImage(x, y, 43, 43, Weather_311_43);		break;
		case 312:  tft.pushImage(x, y, 43, 43, Weather_312_43);		break;
		case 313:  tft.pushImage(x, y, 43, 43, Weather_313_43);		break;
		case 314:  tft.pushImage(x, y, 43, 43, Weather_314_43);		break;
		case 315:  tft.pushImage(x, y, 43, 43, Weather_315_43);		break;
		case 316:  tft.pushImage(x, y, 43, 43, Weather_316_43);		break;
		case 317:  tft.pushImage(x, y, 43, 43, Weather_317_43);		break;
		case 318:  tft.pushImage(x, y, 43, 43, Weather_318_43);		break;
		case 350:  tft.pushImage(x, y, 43, 43, Weather_350_43);		break;
		case 351:  tft.pushImage(x, y, 43, 43, Weather_351_43);		break;
		case 399:  tft.pushImage(x, y, 43, 43, Weather_399_43);		break;
		case 400:  tft.pushImage(x, y, 43, 43, Weather_400_43);		break;
		case 401:  tft.pushImage(x, y, 43, 43, Weather_401_43);		break;
		case 402:  tft.pushImage(x, y, 43, 43, Weather_402_43);		break;
		case 404:  tft.pushImage(x, y, 43, 43, Weather_404_43);		break;
		case 405:  tft.pushImage(x, y, 43, 43, Weather_405_43);		break;
		case 406:  tft.pushImage(x, y, 43, 43, Weather_406_43);		break;
		case 407:  tft.pushImage(x, y, 43, 43, Weather_407_43);		break;
		case 408:  tft.pushImage(x, y, 43, 43, Weather_408_43);		break;
		case 409:  tft.pushImage(x, y, 43, 43, Weather_409_43);		break;
		case 410:  tft.pushImage(x, y, 43, 43, Weather_410_43);		break;
		case 456:  tft.pushImage(x, y, 43, 43, Weather_456_43);		break;
		case 457:  tft.pushImage(x, y, 43, 43, Weather_457_43);		break;
		case 499:  tft.pushImage(x, y, 43, 43, Weather_499_43);		break;
		case 501:  tft.pushImage(x, y, 43, 43, Weather_501_43);		break;
		case 502:  tft.pushImage(x, y, 43, 43, Weather_502_43);		break;
		case 504:  tft.pushImage(x, y, 43, 43, Weather_504_43);		break;
		case 507:  tft.pushImage(x, y, 43, 43, Weather_507_43);		break;
		case 508:  tft.pushImage(x, y, 43, 43, Weather_508_43);		break;
		case 509:  tft.pushImage(x, y, 43, 43, Weather_509_43);		break;
		case 510:  tft.pushImage(x, y, 43, 43, Weather_510_43);		break;
		case 511:  tft.pushImage(x, y, 43, 43, Weather_511_43);		break;
		case 512:  tft.pushImage(x, y, 43, 43, Weather_512_43);		break;
		case 513:  tft.pushImage(x, y, 43, 43, Weather_513_43);		break;
		case 514:  tft.pushImage(x, y, 43, 43, Weather_514_43);		break;
		case 515:  tft.pushImage(x, y, 43, 43, Weather_515_43);		break;
		case 900:  tft.pushImage(x, y, 43, 43, Weather_900_43);		break;
		case 901:  tft.pushImage(x, y, 43, 43, Weather_901_43);		break;
		default:  tft.pushImage(x, y, 43, 43, Weather_999_43);		break;
	}
}
void update_week(int week_data,int32_t x,int32_t y)
{
	switch (week_data)
	{
	case 0:
		printString(x, y, 1, "Sun.");
		break;
	case 1:
		printString(x, y, 1, "Mon.");
		break;
	case 2:
		printString(x, y, 1, "Tues.");
		break;
	case 3:
		printString(x, y, 1, "Wed.");
		break;
	case 4:
		printString(x, y, 1, "Thur.");
		break;
	case 5:
		printString(x, y, 1, "Fri.");
		break;
	case 6:
		printString(x, y, 1, "Sat.");
		break;
	case 7:
		printString(x, y, 1, "Sun.");
		break;
	case 8:
		printString(x, y, 1, "Mon.");
		break;
	}
}

void loop() {
	timeClient.update();									//获取时间信息
	hours = timeClient.getHours();
	minu = timeClient.getMinutes();
	sece = timeClient.getSeconds();	
	week = timeClient.getDay();

	if (last_minu != minu)									//更新时间
	{
		#if Debug
		Serial.printf("time = %d:%d\n", hours, minu);
		Serial.printf("Week = %d\n", week);
		#endif

		if (minu < 10)
		{
			minu_string = "0" + String(minu);
		}
		else
		{
			minu_string = String(minu);
		}

		if (hours < 10)
		{
			hours_string = "0" + String(hours);
		}
		else
		{
			hours_string = String(hours);
		}
		time_string = hours_string + ":" + minu_string;
		for (int i = 0; i <= LineWidth-1 ; i++)
		{
			tft.drawFastVLine(78 + i, 0, 53, TFT_WHITE);
		}
		printString(83, 13, 2, time_string);
		//update_week(week, 0 + 8, 55);
		update_week(week + 1, 56 + 8, 59);
		update_week(week + 2, 112 + 8, 59);
		update_week(week, 95, 38);

		httpCode = http.GET();
		if (httpCode > 0)
		{
			#if Debug
			Serial.printf("[HTTP] GET... code: %d\n", httpCode);
			#endif
			if (httpCode == HTTP_CODE_OK)				//读取响应内容
			{
				response = http.getString();
				days_begin = 0;
				days_flag = 0;
				while (days_flag < 2)
				{
					days_begin++;
					days_char = response.substring(days_begin, days_begin + 1);
					if (days_char == "2")
					{
						days_flag++;
					}
				}
				days_end = days_begin;
				while (days_char != " ")
				{
					days_end++;
					days_char = response.substring(days_end, days_end + 1);
				}
				days = response.substring(days_begin, days_end);
				printString(82, 0, 1, days);
				#if Debug
				Serial.println(days);
				#endif
			}
			http.end();
		}

		last_minu = minu;
		weather_flag = 1;
	}
	if (begin_flag == 1)									//第一次更新
	{

		if (minu < 10)
		{
			minu_string = "0" + String(minu);
		}
		else
		{
			minu_string = String(minu);
		}

		if (hours < 10)
		{
			hours_string = "0" + String(hours);
		}
		else
		{
			hours_string = String(hours);
		}
		time_string = hours_string + ":" + minu_string;
		for (int i = 0; i <= LineWidth - 1; i++)
		{
			tft.drawFastVLine(78 + i, 0, 53, TFT_WHITE);
		}
		printString(83, 13, 2, time_string);
		//update_week(week, 0 + 8, 55);
		update_week(week + 1, 56 + 8, 59);
		update_week(week + 2, 112 + 8, 59);
		update_week(week, 95, 38);

		httpCode = http.GET();
		if (httpCode > 0)
		{
#if Debug
			Serial.printf("[HTTP] GET... code: %d\n", httpCode);
#endif
			if (httpCode == HTTP_CODE_OK)				//读取响应内容
			{
				response = http.getString();
				days_begin = 0;
				days_flag = 0;
				while (days_flag < 2)
				{
					days_begin++;
					days_char = response.substring(days_begin, days_begin + 1);
					if (days_char == "2")
					{
						days_flag++;
					}
				}
				days_end = days_begin;
				while (days_char != " ")
				{
					days_end++;
					days_char = response.substring(days_end, days_end + 1);
				}
				days = response.substring(days_begin, days_end);
				printString(82, 0, 1, days);
#if Debug
				Serial.println(days);
#endif
			}
			http.end();
		}

		last_minu = minu;

		while (weatherNow.get() != 1)
		{
		#if Debug
			Serial.println("weatherNowget failed");
		#endif
		}
		weatherNow.getServerCode();
		printString(43, 15, 2, String(weatherNow.getTemp()));
		update_picture(weatherNow.getIcon(), 0, 5);
		printString(38, 0, 1, "TEMP:");
		printString(0, 59 + 14 * 1, 1, "Humidity");
		printString(0, 59 + 14 * 2, 1, String(weatherNow.getHumidity()) + "%");

		while (weatherForecast.get() != 1)
		{
		#if Debug
			Serial.println("weatherForecast failed");
		#endif
		}
		for (int i = 0; i < 3; i++)
		{
			weatherForecast.getServerCode();
			weatherForecast.getIconDay(i);				// 获取天气图标代码
			switch (i)
			{
			case 0:
				printString(36, 45, 1, String(weatherForecast.getTempMin(i)) + "~" + String(weatherForecast.getTempMax(i)));
				//update_picture(weatherForecast.getIconDay(i), 0, 70);
				printString(0, 59 + 14 * 3, 1, "Precip");
				printString(0, 59 + 14 * 4, 1,String( weatherForecast.getPrecip(i)) + "mm");
				break;
			case 1:
				printString(56, 115, 1, String(weatherForecast.getTempMin(i)) + "~" + String(weatherForecast.getTempMax(i)));
				update_picture(weatherForecast.getIconDay(i), 56, 72);
				break;
			case 2:
				printString(112, 115, 1, String(weatherForecast.getTempMin(i)) + "~" + String(weatherForecast.getTempMax(i)));
				update_picture(weatherForecast.getIconDay(i), 112, 72);
				break;
			}
		}
		begin_flag = 0;
	}

	if(((minu == 10) ||(minu == 40)) && weather_flag == 1)		//更新天气
	{
		#if Debug
		Serial.println("Weather update");
		#endif 

		break_flag = 0;
		while (weatherNow.get() != 1)
		{
			#if Debug
				Serial.println("weatherNowget failed");
			#endif
				break_flag++;
				if (break_flag >= 10)
				{
					break;
				}
		}
		if (break_flag < 10)
		{
			weatherNow.getServerCode();
			last_update = weatherNow.getLastUpdate();
			printString(43, 15, 2, String(weatherNow.getTemp()));
			update_picture(weatherNow.getIcon(), 0, 5);
			printString(38, 0, 1, "TEMP:");
			printString(0, 59 + 14 * 1, 1, "Humidity");
			printString(0, 59 + 14 * 2, 1, String(weatherNow.getHumidity()) + "%");
		}
		break_flag = 0;
		while(weatherForecast.get() != 1)
		{
		#if Debug
			Serial.println("weatherForecast failed");
		#endif
			break_flag++;
			if (break_flag >= 10)
			{
				break;
			}
		}
		if (break_flag < 10)
		{
			for (int i = 0; i < 3; i++)
			{
				weatherForecast.getServerCode();
				weatherForecast.getIconDay(i);      // 获取天气图标代码
				switch (i)
				{
				case 0:
					printString(36, 45, 1, String(weatherForecast.getTempMin(i)) + "~" + String(weatherForecast.getTempMax(i)));
					//update_picture(weatherForecast.getIconDay(i), 0, 70);
					printString(0, 59 + 14 * 3, 1, "Precip");
					printString(0, 59 + 14 * 4, 1, String(weatherForecast.getPrecip(i)) + "mm");
					break;
				case 1:
					printString(56, 115, 1, String(weatherForecast.getTempMin(i)) + "~" + String(weatherForecast.getTempMax(i)));
					update_picture(weatherForecast.getIconDay(i), 56, 72);
					break;
				case 2:
					printString(112, 115, 1, String(weatherForecast.getTempMin(i)) + "~" + String(weatherForecast.getTempMax(i)));
					update_picture(weatherForecast.getIconDay(i), 112, 72);
					break;
				}
			}
		}
		weather_flag = 0;
	}
}
