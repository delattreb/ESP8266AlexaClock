#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <GyverButton.h>
#include <EEPROM.h>
#include "config.h"
#include "WS2812FX.h"

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
GButton touch(PIN_BUTTON_1, LOW_PULL, NORM_OPEN);
WS2812FX ws2812fx24 = WS2812FX(LED_COUNT_24, LED_PIN_24, NEO_GRB + NEO_KHZ800);
WS2812FX ws2812fx60 = WS2812FX(LED_COUNT_60, LED_PIN_60, NEO_GRB + NEO_KHZ800);

#pragma region TIME
bool bbright, bseconde;
int cpt_animation, bright;
int hour, minute, seconde;
int coefh = LED_COUNT_60 / HOUR;
float coefm = (float)coefh/(float)LED_COUNT_60;
uint32_t seconde_color = CYAN, minute_color = BLUE, hour_color = ORANGE;
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
	// initialize EEPROM with predefined size
	EEPROM.begin(EEPROM_SIZE);

	//Input configuration
	pinMode(PIN_BUTTON_1, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_1), pinDidChange, CHANGE);
	touch.setDebounce(80);
	touch.setTimeout(200);
	bbright = false;
	bseconde = true;
	if (EEPROM.read(EEPROM_PLACE_BRIGHT) >= 10)
	{
		bright = EEPROM.read(EEPROM_PLACE_BRIGHT);
#ifdef DEBUG
		Serial.print("Load EEPROM: ");
		Serial.println(bright);
#endif
	}
	else
		bright = BRIGHTNESS;

	timeClient.begin(); // Démarrage du client NTP - Start NTP client

	ws2812fx60.init();
	ws2812fx60.setBrightness(bright);
	ws2812fx60.stop();

	ws2812fx24.init();
	ws2812fx24.setBrightness(bright);
	ws2812fx24.setMode(FX_MODE_COMET);
	ws2812fx24.setSpeed(SPEED_EFFECT);
	ws2812fx24.setColor(ORANGE);
	ws2812fx24.stop();

	//Get time
	timeClient.update();
	minute = uint8_t(timeClient.getMinutes());
	hour = round(timeClient.getHours() % 12 * coefh + minute * coefm);
	seconde = uint8_t(timeClient.getSeconds());

	ws2812fx60.setPixelColor(seconde, seconde_color);
	ws2812fx60.setPixelColor(hour, hour_color);
	ws2812fx60.setPixelColor(minute, minute_color);
	ws2812fx60.show();
}

// loop
void loop()
{
	touch.tick();
	ws2812fx24.service();

	if (bseconde)
		ws2812fx60.setPixelColor(seconde, seconde_color);
	ws2812fx60.setPixelColor(hour, hour_color);
	ws2812fx60.setPixelColor(minute, minute_color);
	ws2812fx24.setBrightness(bright);
	ws2812fx60.setBrightness(bright);
	ws2812fx60.show();

	//Check time
	timeClient.update();
	if (timeClient.getMinutes() != minute)
	{
		ws2812fx60.setPixelColor(hour, 0, 0, 0);
		ws2812fx60.setPixelColor(minute, 0, 0, 0);
		minute = timeClient.getMinutes();
		if (bseconde)
			animation();
		hour = round(timeClient.getHours() % 12 * coefh + minute * coefm);
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

	//Button gesture
	touch.tick();
	if (touch.isTriple())
	{
#ifdef DEBUG
		Serial.print("Save EEPROM: ");
		Serial.println(bright);
#endif
		EEPROM.write(EEPROM_PLACE_BRIGHT, bright);
		EEPROM.commit();
		animation();
	}
	if (touch.isDouble())
	{
		bseconde = !bseconde;
	}
	if (touch.isStep())
	{
		if (bbright)
		{
			if (bright + INCREMENT < 255)
				bright = bright + INCREMENT;
		}
		else
		{
			if (bright - INCREMENT > 5)
				bright = bright - INCREMENT;
		}
#ifdef DEBUG
		Serial.print("bright: ");
		Serial.println(bright);
#endif
	}
	if (touch.isRelease())
	{
		bbright = !bbright;
#ifdef DEBUG
		Serial.println("Change");
#endif
	}
}

//Animation
void animation()
{
	ws2812fx24.start();
	cpt_animation = 0;
}

//pinDidChange
ICACHE_RAM_ATTR void pinDidChange()
{
	touch.tick();
}
