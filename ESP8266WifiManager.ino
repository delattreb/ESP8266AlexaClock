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
NTPClient timeClient(ntpUDP, NTP_URL, NTP_DECALLAGE, NTP_MAJ);
GButton touch(PIN_BUTTON_1, LOW_PULL, NORM_OPEN);
WS2812FX ws2812fx60 = WS2812FX(LED_COUNT_60, LED_PIN_60, NEO_GRB + NEO_KHZ800);

#pragma region TIME
bool bbright, bseconde;
int bright;
int hour, minute, seconde;
int coefh = LED_COUNT_60 / HOUR;
float coefm = (float)coefh / (float)LED_COUNT_60;
uint32_t seconde_color = CYAN, minute_color = BLUE, hour_color = ORANGE;
#pragma endregion

// setup
void setup()
{
#ifdef SERIALLOG
	Serial.begin(SERIALBAUDS);
	while (!Serial)
		continue;
#endif
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
	touch.setDebounce(DEBOUNCE);
	touch.setTimeout(TIMEOUT);
	bbright = false;
	bseconde = true;
	if (EEPROM.read(EEPROM_PLACE_BRIGHT) >= 10)
	{
		bright = EEPROM.read(EEPROM_PLACE_BRIGHT);
		bseconde = EEPROM.read(EEPROM_PLACE_SECONDE);
#ifdef DEBUG
		Serial.print("Load EEPROM: ");
		Serial.println(bright);
#endif
	}
	else
		bright = BRIGHTNESS;

	timeClient.begin(); // Démarrage du client NTP - Start NTP client

	//Get time
	timeClient.update();
	minute = uint8_t(timeClient.getMinutes());
	hour = round(timeClient.getHours() % 12 * coefh + minute * coefm);
	seconde = uint8_t(timeClient.getSeconds());

	ws2812fx60.init();
	ws2812fx60.setBrightness(bright);
	ws2812fx60.stop();
	ws2812fx60.setPixelColor(seconde, seconde_color);
	ws2812fx60.setPixelColor(hour, hour_color);
	ws2812fx60.setPixelColor(minute, minute_color);
	ws2812fx60.show();
}

// loop
void loop()
{
	touch.tick();
	timeClient.update();
	if (timeClient.getMinutes() != minute)
	{
		offws(hour);
		offws(minute);
		minute = timeClient.getMinutes();
		hour = round(timeClient.getHours() % 12 * coefh + minute * coefm);
	}
	if (timeClient.getSeconds() != seconde)
	{
		offws(seconde);
		seconde = timeClient.getSeconds();
		changews(hour, minute, seconde, bright);
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
		EEPROM.write(EEPROM_PLACE_SECONDE, bseconde);
		EEPROM.commit();
	}
	if (touch.isDouble())
	{
#ifdef DEBUG
		Serial.print("Double");
#endif
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
		changews(hour, minute, seconde, bright);
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

//Off Pixel
void offws(int pixel)
{
	ws2812fx60.setPixelColor(pixel, 0, 0, 0);
}

//Change on WS2812
void changews(int hour, int minute, int seconde, int bright)
{
	ws2812fx60.setBrightness(bright);
	if (bseconde)
		ws2812fx60.setPixelColor(seconde, seconde_color);
	ws2812fx60.setPixelColor(hour, hour_color);
	ws2812fx60.setPixelColor(minute, minute_color);
	ws2812fx60.show();
}

//pinDidChange
ICACHE_RAM_ATTR void pinDidChange()
{
	touch.tick();
}
