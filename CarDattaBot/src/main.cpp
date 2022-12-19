#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "WiFI_Bot.hpp"

//ESP8266 hardware definitions
#define InternalLed 2
#define D6 12
//ESP8266 EEPROM parameters
#define EEPROM_SIZE 512
#define MemoryAddress 0

WiFiClientSecure client;

UniversalTelegramBot bot(BOTtoken, client);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const long utcOffsetInSeconds = 3600;

struct maint{
  String Name;
  bool activated;
  int AddKM;
  unsigned long FutureMaint;
};

const int DefinedMaints = 5;
struct car{ //Data structure for car data
  String Brand;
  String Model;
  String Plate;
  unsigned long TotalKMs;

  maint ArrayMaintenance[DefinedMaints]; //Parabrisas, Aire de las ruedas, Aceite lubricante, Correa
};

car Dacia;

void EEPROM_GetData(){ //Library data startup 
  EEPROM.get(MemoryAddress, Dacia);
  EEPROM.commit();
  EEPROM.end();
  //Force configuration
  
  Dacia.ArrayMaintenance[0].Name = "Parabrisas";
  Dacia.ArrayMaintenance[0].AddKM = 100;
  Dacia.ArrayMaintenance[1].Name = "Presión neumáticos";
  Dacia.ArrayMaintenance[1].AddKM = 100;
  Dacia.ArrayMaintenance[2].Name = "Revisar Aceite (5W40)";
  Dacia.ArrayMaintenance[2].AddKM = 1000;
  Dacia.ArrayMaintenance[3].Name = "Cambio de aceite";
  Dacia.ArrayMaintenance[3].AddKM = 20000;
  //Dacia.ArrayMaintenance[3].FutureMaint = 59290;
  Dacia.ArrayMaintenance[4].Name = "Correa";
  //Dacia.ArrayMaintenance[4].FutureMaint = 100000;
  Dacia.ArrayMaintenance[4].AddKM = 100000;
  Dacia.Brand = "Dacia";
  Dacia.Model = "Sandero Stepway";
  Dacia.Plate = "8239JPC";
}

void AutomaticMessages(int nmaint){ //To be developed
  int i = 0;
  String OuputText = "MANTENIMIENTOS PENDIENTES\n";
  for(i = 0; i < nmaint;i++){
    if(Dacia.ArrayMaintenance[i].FutureMaint < Dacia.TotalKMs){
      Dacia.ArrayMaintenance[i].activated = true;
      OuputText += "-";
      OuputText += String(i);
      switch(i){
        case 0:
          OuputText += " El limpia esta guarruzo, no veo TwT\n";
        break;
        case 1:
          OuputText += " Los neumaticos parecen sad, mira la precion :3\n";
        break;
        case 2:
          OuputText += " El aceite parece un poco horny, hazle bonk uwu\n";
        break;
        case 3:
          OuputText += " El aceite está más negro que los cojones de tutankamon, cambialo ewe\n";
        break;
        case 4:
          OuputText +=" ¿Correa? Pero qué correa, si no tengo perro, payaso";
        break;
      }
    }
    else{
      Dacia.ArrayMaintenance[i].activated = false;
    }
  }
  bot.sendMessage(CHAT_ID, OuputText);

}

bool NewMessage(int NMessage){
    int i = 0; //requestID;
    String guest_chat_id = " ";
    String InputText = " ";
    String OutputText = " ";
    String from_name = " ";
    bool safety = true;

    EEPROM.begin(EEPROM_SIZE);//Start EEPROM so data can be stored
    for(i = 0; i < NMessage; i++){

      guest_chat_id = String(bot.messages[i].chat_id); //Sender chat ID
      InputText = bot.messages[i].text; //Message sent from sender
      from_name = bot.messages[i].from_name; //Senders name 
      //Both sender's name and ID are sent to authorized chat ID just when an unknown user sends a request
      if(CHAT_ID != guest_chat_id) {
        bot.sendMessage(guest_chat_id, "I don't know who you are, leave me alone plox TwT"); //Answer to unknown user
        //Sender's info to be sent to autorized user
        OutputText = "Unauthorized access detected, safety lock enabled";
        OutputText += "\nSender's Chat ID: ";
        OutputText += guest_chat_id;
        OutputText += "\nUser's name: ";
        OutputText += from_name;
        bot.sendMessage(CHAT_ID, OutputText);
        delay(2000);
        //Warning to send to the authorized user and through the debbuging interface
        bot.sendMessage(CHAT_ID, "SAFETY LOCK ENABLED: PLEASE, UNLOCK MANUALLY");
        bot.sendChatAction(CHAT_ID, "typing");
        Serial.println("unauthorized access attempt, automatic lock enabled, set D6 to low to unlock");
        safety = false;
      }
    }
    Serial.println(InputText);
    if(safety){ //Gets current kilometres runned by the car and sends them to user
      if(InputText == "/GetKM"){
        OutputText = "Car chan has gone brrrrr for: ";
        OutputText += String(Dacia.TotalKMs);
        OutputText += " km";
        bot.sendMessage(guest_chat_id, OutputText);
      }
      if(InputText.startsWith("/MaintDone")){
        char MaintDSelection = InputText.charAt(10);
        Dacia.ArrayMaintenance[MaintDSelection - 48].activated = 0;
        Dacia.ArrayMaintenance[MaintDSelection - 48].FutureMaint = Dacia.TotalKMs + Dacia.ArrayMaintenance[MaintDSelection - 48].AddKM;
        OutputText = "Maintance ID ";
        OutputText += String(MaintDSelection - 48);
        OutputText += "\nName: ";
        OutputText += Dacia.ArrayMaintenance[MaintDSelection - 48].Name;
        OutputText += "\nNext warning when Car chan reaches: ";
        OutputText += Dacia.ArrayMaintenance[MaintDSelection - 48].FutureMaint;
        OutputText += " km";
        bot.sendMessage(guest_chat_id, OutputText);
      }
      if(InputText.startsWith("/SetKM")){ //User must send /SetKM followed by the number of KMs that are currently in the car
        //int lastKms = Dacia.TotalKMs;
        String StrKMs = " ";
        StrKMs = InputText.substring(6);
        Dacia.TotalKMs = StrKMs.toInt();
        OutputText += "Car chan has gone brrr for: ";
        OutputText += Dacia.TotalKMs;
        OutputText += " km";
        OutputText += "\nSaving into memory";
        bot.sendMessage(guest_chat_id, OutputText);
        AutomaticMessages(DefinedMaints);
        Serial.println("#Write kilometres request");
      }
      if(InputText == "/help"){
        OutputText = "COMMAND LIST AND INFO\n";
        OutputText += "CarAdministration bot, helps you with the maintenance of your car. You must remember to write car total kilometres so the bot can tell what maintenance must be done. Each day, at 16:00, you'll get a list of pending maintanances\n";
        OutputText += "-/GetKM -> Prints car's total stored kms\n";
        OutputText += "-/SetKMXXXX -> Writes car's total stored km into memory, XXXX would be in this case. Remember to upadte regularly as bot works with this data\n";
        OutputText += "-/InformeSerio -> As the rest of the functions are a fckn joke, this one prints a serious reports for visitors\n";
        OutputText += "-/MaintDoneX -> Sets maint ID X done, which must be active first. Also prints when it will be reactivated\n";
        bot.sendMessage(guest_chat_id, OutputText);
      }
      if(InputText == "/InformeSerio"){
        int j = 0;
        Serial.println("#Comenzando informe serio");
        OutputText = "INFORME GENERAL DE MANTENIMIENTO DEL VEHÍCULO";
        OutputText += "\nTipo de aceite: 5W40\n";
        OutputText += "\nKilómetros totales registrados: ";
        OutputText += String(Dacia.TotalKMs);
        OutputText += " km";
        bot.sendMessage(CHAT_ID, OutputText);
        OutputText = "AVISOS ACTIVOS";
        for(j = 0; j < DefinedMaints; j++){
            OutputText +="\n-";
            OutputText += Dacia.ArrayMaintenance[j].Name;
            if(Dacia.ArrayMaintenance[j].activated){
                OutputText += (" - SI");
            }
            else{
              OutputText += (" - NO - ");
              OutputText +=(Dacia.TotalKMs);
              OutputText += ("/");
              OutputText += (Dacia.ArrayMaintenance[j].FutureMaint);
              OutputText += " km";
            }
        }
        
        bot.sendMessage(CHAT_ID, OutputText);
        Serial.println("#Finalizado informe serio");
        
      }

    }
    Serial.print("#Ouput: ");
    Serial.println(OutputText);
    //EEPROM update
    EEPROM.put(MemoryAddress, Dacia);
    EEPROM.commit();
    EEPROM.end();
    return (safety); 
}

void setup() {
  client.setInsecure();
  //Hardware configuration of MCU
  pinMode(InternalLed, OUTPUT);
  pinMode(D6, INPUT_PULLUP);
  digitalWrite(InternalLed, LOW);
  //Serial as debbuging interface
  Serial.begin(115200);
  while(!Serial);
  Serial.println("-MACHINE STARTED!");
  //EEPROM start READ LINE 123
  Serial.println("-STARTING EEPROM MEMORY AND GETTING DATA");
  EEPROM.begin(EEPROM_SIZE);
  //EEPROM_Clear(EEPROM_SIZE); Uncomment just in case the EEPROM must be cleared. Then reload the code with this line commented again.
  EEPROM_GetData();
  Serial.println("  Eeprom memory loaded");
  //WIFI start
  Serial.println("-Setting up WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.print("  Trying to connect to: ");
  Serial.println(ssid);
  Serial.print("  ");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  timeClient.begin();
  Serial.print("Bot local IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("-Starting telegram bot");
  bot.sendChatAction(CHAT_ID, "typing");
  delay(2000);
  bot.sendMessage(CHAT_ID,"OwO, I'm rebooted, hi miss uwu");
  digitalWrite(InternalLed, HIGH);
}

void loop() {
  bool safety = true;
  int numNewMessages = 0;
  while(safety){
    safety = NewMessage(numNewMessages);
    timeClient.update();
    Serial.print(timeClient.getHours());
    Serial.print(timeClient.getMinutes());
    Serial.println(timeClient.getSeconds());
    if(timeClient.getHours() == 15 && timeClient.getMinutes() == 0 && (5 - timeClient.getSeconds() > 0)){
      Serial.println("Preparing automatic messages");
      AutomaticMessages(DefinedMaints);
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    EEPROM.end();
    digitalWrite(InternalLed, HIGH);
    Serial.println("#NORMAL OPERATION#");      
  }
   while (!safety){
    digitalWrite(InternalLed, LOW);
    safety = !digitalRead(D6);
    Serial.println("#EMERGENCY OPERATION#");
    Serial.print("#Reboot status: ");
    Serial.println(safety);
  } 
}

 
  
