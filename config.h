//DEBUG INFORMATION
//#define SERIALLOG
//#define INFO
//#define DEBUG
//#define WIFIDEBUG

// Serial configuration
#define SERIALBAUDS 115200
#define HOUR 12
//ESP8266 configuration
#define NETWORKNAME "Horloge"
#define ATTENPTING 1000
#define DEBOUNCE 80
#define TIMEOUT 200

//Alexa
#define DEVICE_NAME "Horloge"

//NTP
#define NTP_DECALLAGE 3600 //GMT+1
#define NTP_MAJ 60000 // 60s
#define NTP_URL "europe.pool.ntp.org"

//Led configuration
#pragma region LED_CONFIGURATION
#define LED_PIN 4 
#define PIN_BUTTON_1 14
#define LED_COUNT 60
#define BRIGHTNESS 70
#define INCREMENT 10
#pragma endregion

//Save Data
#pragma region SAVE_DATA
#define EEPROM_SIZE 1
#define EEPROM_PLACE_BRIGHT 0
#define EEPROM_PLACE_SECONDE 1
#pragma endregion 