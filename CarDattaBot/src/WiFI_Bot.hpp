//Telegram Bot settings
#include <EEPROM.h>
#define BOTtoken ""
String CHAT_ID = "";

//WiFi credentials

const char* ssid = "";
const char* password = "";

void EEPROM_Clear(int total_memory){
    int i = 0;
    for(i = 0; i < total_memory; i++){
        EEPROM.write(i, 0);
        EEPROM.commit();
    }
}