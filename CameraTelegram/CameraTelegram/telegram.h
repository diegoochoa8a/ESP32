#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

extern bool sendPhotoTele ;
extern bool statePhotoFlashTele ;
extern bool stateFlashTele ;
extern String BOTtoken ;
extern String CHAT_ID ;

void initBotTelegram(UniversalTelegramBot &bot);
void handleNewMessages(UniversalTelegramBot &bot);
String sendPhotoTelegram(WiFiClientSecure clientTCP);
bool sendPhotoTelegramDirect(UniversalTelegramBot bot);

#endif