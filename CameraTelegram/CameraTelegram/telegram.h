#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

extern bool sendPhoto ;
extern String BOTtoken ;
extern String CHAT_ID ;
extern bool flashState ;

void initBotTelegram(UniversalTelegramBot &bot);
void handleNewMessages(UniversalTelegramBot &bot);
String sendPhotoTelegram(WiFiClientSecure clientTCP);

#endif