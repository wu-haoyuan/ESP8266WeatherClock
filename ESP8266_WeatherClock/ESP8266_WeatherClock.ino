#include "main.h"


//####################################################################################################
// Setup
//####################################################################################################
void setup() {

  Serial.begin(115200);
  EEPROM.begin(1024);
  pinMode(BLK_BUILTIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(BLK_BUILTIN,LOW);    
  Web_html();
  while(WiFi.setSleepMode (WIFI_MODEM_SLEEP) == 0);
  tft.begin();
  tft.setSwapBytes(true);
  tft.setRotation(1);//4：自己添加的横屏+镜像（修改#include "TFT_Drivers/ST7735_Rotation.h"中TFT_MAD_MX、TFT_MAD_MY、TFT_MAD_MV的搭配）
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);                        //选择字体
  tft.setTextColor(TFT_WHITE, TFT_BLACK);   //设置字体颜色，背景色
  tft.setCursor(0,0);
  digitalWrite(BLK_BUILTIN,HIGH);    
  tft.println("Author: W.H.Y");

  bool wifiConfig = autoConfig();

  if (wifiConfig == false)
	htmlConfig();   
	                          //HTML配网  
  if(EEPROM.read(0) == 0)
  {
    Location = "101010100";
  }
  else
  {
    Location = get_String(EEPROM.read(0), 1);
  }
	#if Debug
	  Serial.println(Location);
	#endif	  
  weatherNow.config(UserKey, Location, Unit, Lang);			  // 配置实时天气请求信息
  weatherForecast.config(UserKey, Location, Unit, Lang);	// 配置天气预报请求信息
  airQuality.config(UserKey, Location, Unit, Lang);			  // 配置空气质量请求信息
  
  http.setTimeout(5000);
  http.begin(wificlient,GetUrl);										//开启苏宁的时间获取
  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  tft.fillScreen(TFT_BLACK);
}
//####################################################################################################
// Main loop
//####################################################################################################
void loop() {
	timeClient.update();									//获取时间信息
  hours = timeClient.getHours();
	minu = timeClient.getMinutes();
	sece = timeClient.getSeconds();	
	week = timeClient.getDay();
  
  if( ((hours >= 1) & (hours < 8)) | (hours == 0 & minu >= 10))
  {
    digitalWrite(BLK_BUILTIN,LOW);  
    sleep_flag = 1;
  }
  else
  {
    digitalWrite(BLK_BUILTIN,HIGH);  
    sleep_flag = 0;
  }
	if ((last_minu != minu) && (sleep_flag == 0))									//更新时间
	{
		update_time();
		if(weather_flag == 0)
			weather_flag = 1;
	}
	if (begin_flag == 1)									//第一次更新
	{
		update_time();

		update_weathernow();

		update_weatherforecast();

		begin_flag = 0;
	}
	if( ((minu%10 == 0)) && (weather_flag == 1) && (sleep_flag == 0))		//更新天气
	{
		#if Debug
		Serial.println("Weather update");
		#endif 

		update_weathernow();

		update_weatherforecast();

		weather_flag = 0;
	}
}
