#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"
#include "WS2812FX.h"

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
//WS2812FX ws2812fx24 = WS2812FX(LED_COUNT_24, LED_PIN_24, NEO_RGB + NEO_KHZ800);
WS2812FX ws2812fx60 = WS2812FX(LED_COUNT_60, LED_PIN_60, NEO_RGB + NEO_KHZ800);

static int bright = 50;
static unsigned long pMlis = 0;
unsigned long cMlis;
uint8_t second;
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
	timeClient.begin(); // Démarrage du client NTP - Start NTP client

	//ws2812init
	ws2812fx60.init();
	ws2812fx60.setBrightness(bright);
	ws2812fx60.start();
	ws2812fx60.stop();
}

// loop
void loop()
{

	cMlis = millis();
	if (cMlis - pMlis >= ATTENPTING)
	{
		pMlis = cMlis;
		timeClient.update();
		Serial.println(timeClient.getSeconds());

		ws2812fx60.setPixelColor(second, 0, 0, 0);
		second = uint8_t(timeClient.getSeconds());
		ws2812fx60.setPixelColor(second, 0, 0, 255);
		ws2812fx60.show();
	}
}
