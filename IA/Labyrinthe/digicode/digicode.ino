#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Keypad.h>
#include <ctype.h>
#include <WiFi.h>
#include <MQTT.h>



TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

const char ssid[] = "RobotiqueCPE";
const char pass[] = "AppareilLunaire:DauphinRadio";

WiFiClient net;
MQTTClient client;

const byte ROWS = 4; 
const byte COLS = 3; 

char numberKeys[ROWS][COLS] = {
    { '1','2','3' },
    { '4','5','6' },
    { '7','8','9' },
    { '*','0','#' }
};

byte rowPins[ROWS] = {13, 12, 32, 33}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {25, 26, 27}; //connect to the column pinouts of the keypad

Keypad numpad( makeKeymap(numberKeys), rowPins, colPins, sizeof(rowPins), sizeof(colPins) );

static char code[4];
static byte buildCount;
static byte kpadState;



void keypadEvent_num(KeypadEvent key) {
    kpadState = numpad.getState();
    swOnState(key);
}

int step = 1;
int result[] = {5, 25, 13, 36};
bool isComplete = false;

void ecrireCode(uint16_t color, char* codeAfficher){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(30,40);
    tft.setTextSize(5);
    tft.setTextColor(color);
    tft.printf(codeAfficher);
}

void validation(char code[4]) {
  // Convertir 'code' en entier
  int enteredCode = 0;
  for (int i = 0; i < 4; i++) {
    if (isdigit(code[i])) {  // Vérifier que le caractère est un chiffre
      enteredCode = enteredCode * 10 + (code[i] - '0');  // Convertir le caractère en entier
    } else {
      break;  // Si un caractère n'est pas numérique, on sort de la fonction
    }
  }

  if (enteredCode == result[step-1]){
    ecrireCode(TFT_GREEN, code);
    delay(500);
    ecrireCode(TFT_WHITE, code);
    delay(500);
    ecrireCode(TFT_GREEN, code);
    delay(500);

    step ++;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10,10);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.printf("Etape numero %d", step);
    if (step > 4) {  // Si on atteint la fin du tableau 'result', on recommence
      ecrireCode(TFT_GREEN, "BRAVO");
      isComplete = true;
      client.publish("/ia/labyrinthe/status", "finish");
      delay(5000);
    }
  }
  else{
    ecrireCode(TFT_RED, code);
    delay(500);
    ecrireCode(TFT_WHITE, code);
    delay(500);
    ecrireCode(TFT_RED, code);
    delay(500);

  }
}

void swOnState(char key) {
    if (kpadState == PRESSED && !isComplete) {
      Serial.println(key); 
        if (isdigit(key)) { 
            if (buildCount < 4) { 
                code[buildCount++] = key; 
                code[buildCount] = '\0'; 
            }
            else { 
                code[buildCount - 1] = key; 
            }
        }
        if (key == '*') { 
            if (buildCount > 0) {
                buildCount--;
                code[buildCount] = '\0';
            }
        }       
        if (key == '#') { 
            Serial.print("Code entré : ");
            Serial.println(code);
            validation(code);
            buildCount = 0;         // Réinitialise le compteur
            code[buildCount] = '\0';
        }
        if (!isComplete){
          ecrireCode(TFT_WHITE, code);
          Serial.print("Code actuel : ");
          Serial.println(code); 
          tft.setCursor(10,10);
          tft.setTextSize(2);
          tft.setTextColor(TFT_WHITE);
          tft.printf("Etape numero %d", step);
        }
        
    }
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  client.begin("134.214.51.148", net);
  client.onMessage(messageReceived);
  connect(); 

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK); // Efface l'écran
  tft.setTextColor(TFT_WHITE);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10,10);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.printf("Etape numero %d", step);

  numpad.begin( makeKeymap(numberKeys) );
  numpad.addEventListener(keypadEvent_num);  // Add an event listener.
  numpad.setHoldTime(500);    
}

char key;

void loop() {
  key = numpad.getKey( );
  client.loop();
  delay(10); 
  if (!client.connected()) {
    connect();
  }
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduinoClient", "public", "public")) {
    Serial.print("-");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/ia/labyrinthe/cmd");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming message:");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + payload);

  if(payload == "reset"){
    step = 1;
    isComplete = false;
    code[0] = '\0';
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10,10);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.printf("Etape numero %d", step);
    client.publish("/ia/labyrinthe/status", "waiting");
  }
}


