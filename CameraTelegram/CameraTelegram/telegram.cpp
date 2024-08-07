#include "telegram.h"

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
#define FLASH_LED_PIN 4

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

extern bool sendPhotoTele = false;
extern bool statePhotoFlashTele = false;
extern bool stateFlashTele = false;;



// Initialize Telegram BOT
extern String BOTtoken = "7371552171:AAEsp0kzv_3knU0AdLYPfR2vA0j3C7DwVY8";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
extern String CHAT_ID = "1286685600";


// Función para inicializar el bot
void initBotTelegram(UniversalTelegramBot &bot) {
  pinMode(FLASH_LED_PIN, OUTPUT);
  Serial.println("Bot TELEGRAM initialized");  
}


void handleNewMessages(UniversalTelegramBot &bot) {
    if (millis() > lastTimeBotRan + botRequestDelay){
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        if (numNewMessages > 0){
          Serial.print("Handle New Messages: ");
          Serial.println(numNewMessages);

          for (int i = 0; i < numNewMessages; i++) {
              String chat_id = String(bot.messages[i].chat_id);
              if (chat_id != CHAT_ID){
              bot.sendMessage(chat_id, "Unauthorized user", "");
              continue;
              }
              
              // Print the received message
              String text = bot.messages[i].text;
              Serial.println(text);
              
              String from_name = bot.messages[i].from_name;
              if (text == "/start") {
              String welcome = "Welcome , " + from_name + "\n";
              welcome += "Use the following commands to interact with the ESP32-CAM \n";
              welcome += "/photo : takes a new photo\n";
              welcome += "/photoflash : takes a new photo with flash LED\n";
              welcome += "/flashON : toggles flash LED ON \n";
              welcome += "/flashOFF : toggles flash LED OFF \n";
              bot.sendMessage(CHAT_ID, welcome, "");
              }
              if (text == "/flashON") {
                digitalWrite(FLASH_LED_PIN, 1);
                delay(100);
                Serial.println("Change flash LED ON");
              }
              if (text == "/flashOFF") {
                digitalWrite(FLASH_LED_PIN, 0);
                delay(100);
                Serial.println("Change flash LED OFF");
              }
              if (text == "/photo") {
                sendPhotoTele = true;
                statePhotoFlashTele = false;
                Serial.println("New photo request");
              }
              if (text == "/photoflash") {
                sendPhotoTele = true;
                statePhotoFlashTele = true;
                Serial.println("New photo request with flash");
              }
          }
        }
        lastTimeBotRan = millis();
    }
}

bool isMoreDataAvailable();
byte *getNextBuffer();
int getNextBufferLen();
camera_fb_t *fb = NULL;
bool dataAvailable = false;

bool sendPhotoTelegramDirect(UniversalTelegramBot bot) {
  Serial.println("Enter sendPhotoTelegramDirect");

  if (statePhotoFlashTele) {
    Serial.println("On FLASH");
    digitalWrite(FLASH_LED_PIN, 1);
  }

  //Dispose first picture because of bad quality
  fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed TELEGRAM");
    delay(1000);
    //ESP.restart();
    return false;
  }else{
    Serial.println("Camera capture OK TELEGRAM");
  }

  dataAvailable = true;
  Serial.println("Sending Photo direct");
  String response = bot.sendPhotoByBinary(CHAT_ID, "image/jpeg", fb->len,
                        isMoreDataAvailable, nullptr,
                        getNextBuffer, getNextBufferLen);
  Serial.println(response);
  // Parse the response
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    esp_camera_fb_return(fb);
    return false;
  }

  bool success = doc["ok"];
  
  if (!success) {
    Serial.println("Sending Photo failed!");
    esp_camera_fb_return(fb);
    return false;
  }

  Serial.println("Sending Photo direct done!");
  esp_camera_fb_return(fb);

  if (statePhotoFlashTele){
    Serial.println("Off FLASH");
    digitalWrite(FLASH_LED_PIN, 0);
  }

  return true;
}

bool isMoreDataAvailable()
{
  if (dataAvailable)
  {
    dataAvailable = false;
    return true;
  }
  else
  {
    return false;
  }
}

byte *getNextBuffer()
{
  if (fb)
  {
    return fb->buf;
  }
  else
  {
    return nullptr;
  }
}

int getNextBufferLen()
{
  if (fb)
  {
    return fb->len;
  }
  else
  {
    return 0;
  }
}