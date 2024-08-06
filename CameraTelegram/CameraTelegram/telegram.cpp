#include "telegram.h"

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

extern bool sendPhoto = false;


extern bool flashState = LOW;



// Initialize Telegram BOT
extern String BOTtoken = "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
extern String CHAT_ID = "XXXXXXXXXX";


// Función para inicializar el bot
void initBotTelegram(UniversalTelegramBot &bot) {
    Serial.println("Bot initialized");
}


void handleNewMessages(UniversalTelegramBot &bot) {
    if (millis() > lastTimeBotRan + botRequestDelay){
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

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
            welcome += "/flash : toggles flash LED \n";
            bot.sendMessage(CHAT_ID, welcome, "");
            }
            if (text == "/flash") {
            flashState = !flashState;
            digitalWrite(LED_GPIO_NUM, flashState);
            Serial.println("Change flash LED state");
            }
            if (text == "/photo") {
            sendPhoto = true;
            Serial.println("New photo request");
            }
        }
        lastTimeBotRan = millis();
    }
}


String sendPhotoTelegram(WiFiClientSecure clientTCP) {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  //Dispose first picture because of bad quality
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb); // dispose the buffered image
  
  // Take a new photo
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    size_t imageLen = fb->len;
    size_t extraLen = head.length() + tail.length();
    size_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}