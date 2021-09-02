#ifndef __main_H
#define __main_H

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
#include <User_Setup.h>                     //根据屏幕驱动及大小
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include "jpeg1.h"
#include "hanzi.h"

#ifndef STASSID
#define STASSID "ESP8266"
#define STAPSK  "123456789"
#endif

#define BLK_BUILTIN 5
#define LED_BUILTIN 2
#define Debug 0                             
#define LineWidth 2	    
                   
const char* ssid = STASSID;                  //wifi名称
const char* password = STAPSK;              //wifi密码
IPAddress apIP(192, 168, 4, 1);             //web配网固定IP
ESP8266WebServer server(80);
const int analogInPin = A0;
bool LED_Flag = false;
String last_update, time_string, minu_string, hours_string,str_value;
int hours, minu, sece, last_minu = 0, week, last_week = 0, temp = 0,sensorValue = 0;
bool begin_flag = 1, weather_flag = 1, sleep_flag = 0;
int days_begin, days_end, days_flag,httpCode;
String days, days_char; 
String response;
float power_value = 0,value = 0;

WiFiUDP ntpUDP;                     //时间获取实例化
WeatherNow weatherNow;              //天气获取实例化
WeatherForecast weatherForecast;
AirQuality airQuality;
HTTPClient http;
WiFiClient wificlient;
String GetUrl = "http://quan.suning.com/getSysTime.do";//用苏宁的授时获取日期

NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000); //使用阿里云的校时服务器
String UserKey = "***************";  //私钥获取地址 https://dev.heweather.com/docs/start/get-api-key
String Location = "*************";        // 城市代码 https://github.com/heweather/LocationList ,表中的Location_ID，或至 https://where.qweather.com/index.html 查询     
String Unit = "m";                    // 公制-m/英制-i
String Lang = "zh";                   // 语言 英文-en/中文-zh

TFT_eSPI tft = TFT_eSPI();

String Web_str;
void Web_html(void)
{
  Web_str ="<!DOCTYPE html>";
  Web_str  += "<html>";
  Web_str  += "<head>";
  Web_str  += "<meta charset=\"UTF-8\">";
  Web_str  += "<meta name=\"viewport\"content=\"width=device-width, initial-scale=1.0\">";
  Web_str  += "<meta http-equiv=\"X-UA-Compatible\"content=\"ie=edge\">";
  Web_str  += "<title>ESP8266网页配网</title></head><body><form name=\"my\">";
  Web_str  += "WiFi名称：<input type=\"text\"name=\"s\"placeholder=\"请输入您WiFi的名称\"id=\"aa\">";
  Web_str  += "<br>WiFi密码：<input type=\"text\"name=\"p\"placeholder=\"请输入您WiFi的密码\"id=\"bb\">";
  Web_str  += "<br>城市代码：<input type=\"text\"name=\"p\"placeholder=\"请输入您的城市代码\"id=\"cc\">";
  Web_str  += "<br><input type=\"button\"value=\"连接\"onclick=\"wifi()\">";
  Web_str  += "</form>";
  Web_str  += "<script language=\"javascript\">";
  Web_str  += "function wifi()";
  Web_str  += "{";
  Web_str  += "var ssid=my.s.value;";
  Web_str  += "var password=bb.value;";
  Web_str  += "var location=cc.value;var xmlhttp=new XMLHttpRequest();";
  Web_str  += "xmlhttp.open(\"GET\",\"/HandleVal?ssid=\"+ssid+\"&password=\"+password+\"&location=\"+location,true);xmlhttp.send()";
  Web_str  += "}";
  Web_str  += "</script>";
  Web_str  += "</body>";
  Web_str  += "</html>" ;
}
void set_String(int a, int b, String str)   //a是长度，b开始存str的值
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
  server.send(200, "text/html", Web_str);
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
  tft.println(wifis); tft.println(wifip);  tft.println(wifil);
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
/*****************************************************
 * 函数名称：printString(int32_t x,int32_t y,int32_t size,const String &string)
 * 函数说明：TFT屏字符绘制函数
 * 参数说明：x 起始x坐标，y 起始y坐标，size 字符尺寸参数，string 所需绘制的字符
******************************************************/
void printString(int32_t x,int32_t y,int32_t size,const String &string)
{
  tft.setTextSize(size);              
  tft.setCursor(x, y);              
  tft.println(string);
}

/*****************************************************
 * 函数名称：update_picture(int Weather_daima,int32_t x,int32_t y)
 * 函数说明：天气图标绘制函数
 * 参数说明：Weather_daima 天气图标代码， x，y天气图标绘制位置
******************************************************/
void update_picture(int Weather_daima,int32_t x,int32_t y)
{
  switch (Weather_daima)
  {
    case 100:  tft.pushImage(x, y, 43, 43, Weather_100_43);   break;
    case 101:  tft.pushImage(x, y, 43, 43, Weather_101_43);   break;
    case 102:  tft.pushImage(x, y, 43, 43, Weather_102_43);   break;
    case 103:  tft.pushImage(x, y, 43, 43, Weather_103_43);   break;
    case 104:  tft.pushImage(x, y, 43, 43, Weather_104_43);   break;
    case 150:  tft.pushImage(x, y, 43, 43, Weather_150_43);   break;
    case 153:  tft.pushImage(x, y, 43, 43, Weather_153_43);   break;
    case 154:  tft.pushImage(x, y, 43, 43, Weather_154_43);   break;
    case 300:  tft.pushImage(x, y, 43, 43, Weather_300_43);   break;
    case 301:  tft.pushImage(x, y, 43, 43, Weather_301_43);   break;
    case 302:  tft.pushImage(x, y, 43, 43, Weather_302_43);   break;
    case 304:  tft.pushImage(x, y, 43, 43, Weather_304_43);   break;
    case 305:  tft.pushImage(x, y, 43, 43, Weather_305_43);   break;
    case 306:  tft.pushImage(x, y, 43, 43, Weather_306_43);   break;
    case 307:  tft.pushImage(x, y, 43, 43, Weather_307_43);   break;
    case 308:  tft.pushImage(x, y, 43, 43, Weather_308_43);   break;
    case 309:  tft.pushImage(x, y, 43, 43, Weather_309_43);   break;
    case 310:  tft.pushImage(x, y, 43, 43, Weather_310_43);   break;
    case 311:  tft.pushImage(x, y, 43, 43, Weather_311_43);   break;
    case 312:  tft.pushImage(x, y, 43, 43, Weather_312_43);   break;
    case 313:  tft.pushImage(x, y, 43, 43, Weather_313_43);   break;
    case 314:  tft.pushImage(x, y, 43, 43, Weather_314_43);   break;
    case 315:  tft.pushImage(x, y, 43, 43, Weather_315_43);   break;
    case 316:  tft.pushImage(x, y, 43, 43, Weather_316_43);   break;
    case 317:  tft.pushImage(x, y, 43, 43, Weather_317_43);   break;
    case 318:  tft.pushImage(x, y, 43, 43, Weather_318_43);   break;
    case 350:  tft.pushImage(x, y, 43, 43, Weather_350_43);   break;
    case 351:  tft.pushImage(x, y, 43, 43, Weather_351_43);   break;
    case 399:  tft.pushImage(x, y, 43, 43, Weather_399_43);   break;
    case 400:  tft.pushImage(x, y, 43, 43, Weather_400_43);   break;
    case 401:  tft.pushImage(x, y, 43, 43, Weather_401_43);   break;
    case 402:  tft.pushImage(x, y, 43, 43, Weather_402_43);   break;
    case 404:  tft.pushImage(x, y, 43, 43, Weather_404_43);   break;
    case 405:  tft.pushImage(x, y, 43, 43, Weather_405_43);   break;
    case 406:  tft.pushImage(x, y, 43, 43, Weather_406_43);   break;
    case 407:  tft.pushImage(x, y, 43, 43, Weather_407_43);   break;
    case 408:  tft.pushImage(x, y, 43, 43, Weather_408_43);   break;
    case 409:  tft.pushImage(x, y, 43, 43, Weather_409_43);   break;
    case 410:  tft.pushImage(x, y, 43, 43, Weather_410_43);   break;
    case 456:  tft.pushImage(x, y, 43, 43, Weather_456_43);   break;
    case 457:  tft.pushImage(x, y, 43, 43, Weather_457_43);   break;
    case 499:  tft.pushImage(x, y, 43, 43, Weather_499_43);   break;
    case 501:  tft.pushImage(x, y, 43, 43, Weather_501_43);   break;
    case 502:  tft.pushImage(x, y, 43, 43, Weather_502_43);   break;
    case 504:  tft.pushImage(x, y, 43, 43, Weather_504_43);   break;
    case 507:  tft.pushImage(x, y, 43, 43, Weather_507_43);   break;
    case 508:  tft.pushImage(x, y, 43, 43, Weather_508_43);   break;
    case 509:  tft.pushImage(x, y, 43, 43, Weather_509_43);   break;
    case 510:  tft.pushImage(x, y, 43, 43, Weather_510_43);   break;
    case 511:  tft.pushImage(x, y, 43, 43, Weather_511_43);   break;
    case 512:  tft.pushImage(x, y, 43, 43, Weather_512_43);   break;
    case 513:  tft.pushImage(x, y, 43, 43, Weather_513_43);   break;
    case 514:  tft.pushImage(x, y, 43, 43, Weather_514_43);   break;
    case 515:  tft.pushImage(x, y, 43, 43, Weather_515_43);   break;
    case 900:  tft.pushImage(x, y, 43, 43, Weather_900_43);   break;
    case 901:  tft.pushImage(x, y, 43, 43, Weather_901_43);   break;
    default:  tft.pushImage(x, y, 43, 43, Weather_999_43);    break;
  }
}
/*****************************************************
 * 函数名称：update_week(int week_data,int32_t x,int32_t y)
 * 函数说明：星期绘制函数
 * 参数说明：week_data 星期参数， x，y天气图标绘制位置
******************************************************/
void update_week(int week_data,int32_t x,int32_t y)
{
  switch (week_data)
  {
  case 0:    printString(x, y, 1, "Sun.");    break;
  case 1:    printString(x, y, 1, "Mon.");    break;
  case 2:    printString(x, y, 1, "Tues.");   break;
  case 3:    printString(x, y, 1, "Wed.");    break;
  case 4:    printString(x, y, 1, "Thur.");   break;
  case 5:    printString(x, y, 1, "Fri.");    break;
  case 6:    printString(x, y, 1, "Sat.");    break;
  case 7:    printString(x, y, 1, "Sun.");    break;
  case 8:    printString(x, y, 1, "Mon.");    break;
  }
}
void print_number(int number,int32_t x,int32_t y)
{
  tft.fillRect(x,y,Numeber_length,Numeber_height,TFT_BLACK);   
  switch(number)
  {
    case 0:    tft.drawBitmap(x, y, hz32_0, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 1:    tft.drawBitmap(x, y, hz32_1, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 2:    tft.drawBitmap(x, y, hz32_2, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 3:    tft.drawBitmap(x, y, hz32_3, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 4:    tft.drawBitmap(x, y, hz32_4, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 5:    tft.drawBitmap(x, y, hz32_5, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 6:    tft.drawBitmap(x, y, hz32_6, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 7:    tft.drawBitmap(x, y, hz32_7, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 8:    tft.drawBitmap(x, y, hz32_8, Numeber_length, Numeber_height, TFT_WHITE); break;
    case 9:    tft.drawBitmap(x, y, hz32_9, Numeber_length, Numeber_height, TFT_WHITE); break;
  }
}
/*****************************************************
 * 函数名称：get_suning_day(void)
 * 函数说明：苏宁授时获取函数
 * 参数说明：
******************************************************/
void get_suning_day(void)
{
  httpCode = http.GET();
  if (httpCode > 0)
  {
#if Debug
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
#endif
    if (httpCode == HTTP_CODE_OK)       //读取响应内容
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
}
/*****************************************************
 * 函数名称：updata_ali_time(void)
 * 函数说明：阿里云授时获取函数
 * 参数说明：
******************************************************/
void updata_ali_time(void)
{
  if(hours < 10)
  {
    print_number( 0, 83, 13);
    print_number( hours, 83+Numeber_length, 13);
  }
  else
  {
    print_number( hours/10, 83, 13);
    print_number( hours%10, 83+Numeber_length, 13);
  }
  printString(83+Numeber_length*2+5,13,2,":");
  if(minu < 10)
  {
    print_number( 0, 83+Numeber_length*2+6*2, 13);
    print_number( minu, 83+Numeber_length*2+6*2+Numeber_length, 13);
  }
  else
  {
    print_number( minu/10, 83+Numeber_length*2+6*2, 13);
    print_number( minu%10, 83+Numeber_length*2+6*2+Numeber_length, 13);
  }
  for (int i = 0; i <= LineWidth - 1; i++)      
    tft.drawFastVLine(78 + i, 0, 53, TFT_WHITE);
  update_week(week + 1, 56 + 8, 59);
  update_week(week + 2, 112 + 8, 59);
  update_week(week, 120, 38);
}

void update_time(void)//屏幕更新时间信息
{
#if Debug
	Serial.printf("time = %d:%d\n", hours, minu);
	Serial.printf("Week = %d\n", week);
#endif

	updata_ali_time();
	get_suning_day();

	last_minu = minu;
}

void update_weathernow(void)//屏幕更新当前天气信息
{
	while (weatherNow.get() != 1)
	{
	#if Debug
		Serial.println("weatherNowget failed");
	#endif
	}
	weatherNow.getServerCode();
	
	update_picture(weatherNow.getIcon(), 0, 5);//打印当前天气图标

  tft.drawBitmap(38, 0, hz16_WEN, Word_length, Word_length, TFT_WHITE);
  tft.drawBitmap(38+Word_length, 0, hz16_DU, Word_length, Word_length, TFT_WHITE);  
 	printString( 38+Word_length*2, 0, 1, ":");//打印汉字温度
  temp = weatherNow.getFeelLike();
  tft.fillRect(45,16,Numeber_length*2,Numeber_height,TFT_BLACK);   
  tft.fillRect(40,30,6,3,TFT_BLACK);   
  if(temp < 0)
  {
    temp = -temp;
    if(temp < 10)
    {
      for (int i = 0; i <= 1; i++)      
        tft.drawFastHLine(45, 30+i, 5, TFT_WHITE);
      print_number( temp, 45+Numeber_length, 15);
    }
    else
    {
      print_number( temp/10, 45, 15);
      print_number( temp%10, 45+Numeber_length, 15);
      for (int i = 0; i <= 1; i++)      
        tft.drawFastHLine(40, 30+i, 5, TFT_WHITE);
    }
  }
  else
  {
    if(temp < 10)
    {
      print_number( temp, 45+Numeber_length, 15);
    }
    else
    {
      print_number( temp/10, 45, 15);
    print_number( temp%10, 45+Numeber_length, 15);
    }
  }

	tft.drawBitmap(0, 59 + Word_length * 0, hz16_SHI, Word_length, Word_length, TFT_WHITE);
  tft.drawBitmap(0+Word_length, 59 + Word_length * 0, hz16_DU, Word_length, Word_length, TFT_WHITE);  //打印汉字湿度
	printString(0, 59 + Word_length * 1, 1, String(weatherNow.getHumidity()) + "%");//打印湿度值
}

void update_weatherforecast(void)//更新天气预报信息
{
  int k = 0;
	while (weatherForecast.get() != 1)
	{
    k++;
	#if Debug
		Serial.println("weatherForecast failed");
	#endif
    if(k >= 10)
    {
      break;
    }
	}
  if(k < 10)
  {
    for (int i = 0; i < 3; i++)
    {
      weatherForecast.getServerCode();
      weatherForecast.getIconDay(i);				// 获取天气图标代码
      switch (i)
      {
      case 0:
        printString(36, 45, 1, String(weatherForecast.getTempMin(i)) + "~" + String(weatherForecast.getTempMax(i)));
        tft.drawBitmap(0, 59 + Word_length * 2, hz16_JIANG, Word_length, Word_length, TFT_WHITE);
        tft.drawBitmap(0+Word_length, 59 + Word_length * 2, hz16_YU, Word_length, Word_length, TFT_WHITE);
        tft.drawBitmap(0+Word_length*2, 59 + Word_length * 2, hz16_LIANG, Word_length, Word_length, TFT_WHITE);   
        printString( 0+Word_length*3, 59 + Word_length * 2, 1, ":");//打印汉字降雨量
        printString(0, 59 + Word_length * 3, 1,String( weatherForecast.getPrecip(i)) + "mm");
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
}

void update_powervalue(void)
{
	sensorValue = analogRead(analogInPin);
  	power_value = sensorValue*0.0009765625;
  	power_value = 1-power_value-0.05;
	#if Debug
  	Serial.print("Power value = ");
  	Serial.println(power_value);
	#endif
  	power_value = power_value*5-0.2;
  	if(power_value >= 3.9)
    	value = 1;
  	else
    	value = (power_value-3.3)/0.6;
  	// print the readings in the Serial Monitor
	#if Debugd
  	Serial.print("Power value = ");
  	Serial.println(power_value);
  	Serial.print("value = ");
  	Serial.println(value);
  	Serial.print("sensorValue = ");
  	Serial.println(sensorValue);
  	Serial.println();
	#endif
  str_value = String(value*100);
	str_value = str_value.substring(0, str_value.length()-3);
  str_value = str_value + "%";
	/*if(value > 0.30)
	{
		tft.setTextColor(TFT_GREEN, TFT_BLACK);		//设置字体颜色，背景色
	}
	else if(value >= 0.20 & value <= 0.30)
	{
		tft.setTextColor(TFT_YELLOW, TFT_BLACK);		//设置字体颜色，背景色
	}
	else if(value < 0.20)
	{
		tft.setTextColor(TFT_RED, TFT_BLACK);		//设置字体颜色，背景色
	}*/
  printString(82,38,1,String(power_value));
	tft.setTextColor(TFT_WHITE, TFT_BLACK);		//设置字体颜色，背景色
}

#endif
