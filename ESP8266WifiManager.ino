#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"
#include "WS2812FX.h"

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
WS2812FX ws2812fx24 = WS2812FX(LED_COUNT_24, LED_PIN_24, NEO_GRB + NEO_KHZ800);
WS2812FX ws2812fx60 = WS2812FX(LED_COUNT_60, LED_PIN_60, NEO_GRB + NEO_KHZ800);

#pragma region TIME
int cpt_animation;
int hour, minute, seconde;
uint32_t seconde_color = BLUE, minute_color = CYAN, hour_color = ORANGE;
#pragma endregion

// setup
void setup()
{
	Serial.begin(SERIALBAUDS);
	while (!Serial)
		continue;
#ifdef INFO
	delay(5000);
	Serial.print("Core version: ");
	Serial.println(ESP.getCoreVersion());
	Serial.print("Sdk version: ");
	Serial.println(ESP.getSdkVersion());
	Serial.print("MAC: ");
	Serial.println(WiFi.macAddress());
#endif
	WiFiManager wifiManager;
	//Reset setting
	//wifiManager.resetSettings();
	wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 255, 255, 0));
#ifdef WIFIDEBUG
	wifiManager.setDebugOutput(true);
#else
	wifiManager.setDebugOutput(false);
#endif

	if (!wifiManager.autoConnect(NETWORKNAME))
	{
#ifdef DEBUG
		Serial.println("Failed to connect");
#endif
		delay(1000);
		ESP.reset();
		delay(5000);
	}
	//Input configuration
	pinMode(PIN_BUTTON_1, INPUT_PULLUP);
	pinMode(PIN_BUTTON_2, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_1), pinDidChange, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_2), pinDidChange, CHANGE);

	timeClient.begin(); // Démarrage du client NTP - Start NTP client

	ws2812fx60.init();
	ws2812fx60.setBrightness(BRIGHTNESS_60);
	ws2812fx60.stop();

	ws2812fx24.init();
	ws2812fx24.setBrightness(BRIGHTNESS_24);
	ws2812fx24.setMode(FX_MODE_COMET);
	ws2812fx24.setSpeed(SPEED_EFFECT);
	ws2812fx24.setColor(ORANGE);
	ws2812fx24.stop();

	//Get time
	timeClient.update();
	hour = round((uint8_t(timeClient.getHours() % 12) * LED_COUNT_60) / 12);
	minute = uint8_t(timeClient.getMinutes());
	seconde = uint8_t(timeClient.getSeconds());

	ws2812fx60.setPixelColor(seconde, seconde_color);
	ws2812fx60.setPixelColor(hour, hour_color);
	ws2812fx60.setPixelColor(minute, minute_color);
	ws2812fx60.show();
}

// loop
void loop()
{

	ws2812fx24.service();

	ws2812fx60.setPixelColor(seconde, seconde_color);
	ws2812fx60.setPixelColor(hour, hour_color);
	ws2812fx60.setPixelColor(minute, minute_color);
	ws2812fx60.show();

	//Check time
	timeClient.update();
	if (timeClient.getHours() != hour)
	{
		ws2812fx60.setPixelColor(hour, 0, 0, 0);
		hour = round((uint8_t(timeClient.getHours() % 12) * LED_COUNT_60) / 12);
	}
	if (timeClient.getMinutes() != minute)
	{
		ws2812fx60.setPixelColor(minute, 0, 0, 0);
		minute = timeClient.getMinutes();
		//Animation
		ws2812fx24.start();
		cpt_animation = 0;
	}
	if (timeClient.getSeconds() != seconde)
	{
		ws2812fx60.setPixelColor(seconde, 0, 0, 0);
		seconde = timeClient.getSeconds();
		if (cpt_animation <= ANIMATION)
			cpt_animation++;
		else 
		  ws2812fx24.stop();	
	}
}

//pinDidChange
ICACHE_RAM_ATTR void pinDidChange()
{
	//digitalRead(PIN_BUTTON_1);
	//digitalRead(PIN_BUTTON_2);
}
