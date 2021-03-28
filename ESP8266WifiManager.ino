#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <GyverButton.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include <Espalexa.h>
#include "config.h"

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_URL, NTP_DECALLAGE, NTP_MAJ);
GButton touch(PIN_BUTTON_1, LOW_PULL, NORM_OPEN);
Adafruit_NeoPixel pixels(LED_COUNT_60, LED_PIN_60, NEO_GRB + NEO_KHZ800);

#pragma region ALEXA
//callback functions
void deltaChanged(EspalexaDevice *dev);
Espalexa espalexa;
EspalexaDevice *epsilon;
#pragma endregion

#pragma region TIME
bool bbright, bseconde, bsummer;
int bright;
int hour, minute, seconde, dayweek, daymonth, month;
unsigned long epochTime;
int coefh = LED_COUNT_60 / HOUR;
float coefm = (float)coefh / (float)LED_COUNT_60;
#pragma endregion

#pragma region COLOR
uint32_t seconde_color = pixels.Color(0, 0, 128);
uint32_t minute_color = pixels.Color(0, 255, 255);
uint32_t hour_color = pixels.Color(255, 48, 0);
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
	bsummer = false;
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
	timeClient.update();
	epochTime = timeClient.getEpochTime();
	struct tm *ptm = gmtime((time_t *)&epochTime);
	dayweek = timeClient.getDay();
	daymonth = ptm->tm_mday;
	month = ptm->tm_mon + 1;
	bsummer = isSummer(dayweek, daymonth, month, timeClient.getHours());
#ifdef DEBUG
	Serial.print("JourSemaine: ");
	Serial.println(dayweek);
	Serial.print("JourMois: ");
	Serial.println(daymonth);
	Serial.print("Mois: ");
	Serial.println(month);
	Serial.print("Heure: ");
	Serial.println(timeClient.getHours());
	Serial.print("Ete: ");
	Serial.println(bsummer);
#endif

	pixels.begin();
	pixels.clear();
	pixels.setPixelColor(hour, hour_color);
	pixels.setPixelColor(minute, minute_color);
	pixels.setPixelColor(seconde, seconde_color);
	pixels.setBrightness(bright);
	pixels.show();

	// Define your devices here.
	espalexa.addDevice(DEVICE_NAME, deltaChanged, EspalexaDeviceType::color);
	espalexa.begin();
}

// loop
void loop()
{
	touch.tick();
	timeClient.update();

	//Alexa
	espalexa.loop();
	delay(1);

	if (timeClient.getSeconds() != seconde)
	{
		if (timeClient.getMinutes() != minute)
		{
			offws(hour);
			offws(minute);
			minute = timeClient.getMinutes();
			hour = round(timeClient.getHours() % 12 * coefh + minute * coefm);
			if (bsummer)
				hour += 1;
		}
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
	pixels.setPixelColor(pixel, pixels.Color(0, 0, 0));
}

//Change on WS2812
void changews(int hour, int minute, int seconde, int bright)
{
	pixels.setBrightness(bright);
	if (bseconde)
		pixels.setPixelColor(seconde, seconde_color);
	pixels.setPixelColor(hour, hour_color);
	pixels.setPixelColor(minute, minute_color);

	pixels.show();
}

//pinDidChange
ICACHE_RAM_ATTR void pinDidChange()
{
	touch.tick();
}

//Callback
void deltaChanged(EspalexaDevice *d)
{
	if (d == nullptr)
		return;
	if (d->getName() == DEVICE_NAME)
	{
		//Get color
		bright = d->getValue();
		seconde_color = pixels.Color(d->getR(), d->getG(), d->getB());
#ifdef DEBUG
		Serial.print("Value: ");
		Serial.print(bright);
		Serial.print(", color R");
		Serial.print(d->getR());
		Serial.print(", G");
		Serial.print(d->getG());
		Serial.print(", B");
		Serial.println(d->getB());
#endif
	}
}

// isSummer
bool isSummer(int dayweek, int daymonth, int month, int hour)
{
	if (month < 3 || month > 11)
		return false;
	if (month > 3 && month < 11)
		return true;
	//In march, we are DST if our previous sunday was on or after the 8th.
	if (month == 3)
		return (daymonth - dayweek) >= 8;
	//In november we must be before the first sunday to be dst.
	//That means the previous sunday must be before the 1st.
	return (daymonth - dayweek) <= 0;
}