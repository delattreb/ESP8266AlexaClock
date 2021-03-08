//DEBUG INFORMATION
//#define SERIALLOG
//#define INFO
//#define DEBUG
//#define WIFIDEBUG

// Serial configuration
#define SERIALBAUDS 9600
#define HOUR 12
//ESP8266 configuration
#define NETWORKNAME "ESPClock"
#define ATTENPTING 1000

//NTP
#define NTP_DECALLAGE 3600 //GMT+1
#define NTP_MAJ 60000 // 60s
#define NTP_URL "europe.pool.ntp.org"

//Led configuration
#pragma region LED_CONFIGURATION
#define LED_PIN_24 5 
#define LED_PIN_60 4 
#define PIN_BUTTON_1 14

#define LED_COUNT_24 24
#define LED_COUNT_60 60
#define SPEED_EFFECT 1400
#define BRIGHTNESS 70

#define ANIMATION 2
#define INCREMENT 10
#pragma endregion


#pragma region SAVE_DATA
#define EEPROM_SIZE 1
#define EEPROM_PLACE_BRIGHT 0
#pragma endregion 