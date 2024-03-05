#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
#include <DMXSerial.h>
#include <FastLED.h>

#define NUM_LEDS 1 // Nombre de LEDs dans la bande
#define DATA_PIN 6  // Broche de données à laquelle la led est connectée

CRGB leds[NUM_LEDS]; // Déclaration d'un tableau de LED

#define pinCE   10
#define pinCSN  9

RF24 radio(pinCE, pinCSN);
const byte adresse[6] = "00001";
byte valeursEnvoyees1[32]; // Canaux 1-31
byte valeursEnvoyees2[32]; // Canaux 32-62
byte valeursEnvoyees3[32]; // Canaux 63-93
byte valeursEnvoyees4[32]; // Canaux 94-124
byte valeursEnvoyees5[32]; // Canaux 125-155
byte valeursEnvoyees6[32]; // Canaux 156-186
const int boutonPin = A1;
int mode = 1;

void setup() {
  Serial.begin(9600);
  DMXSerial.init(DMXReceiver);
  pinMode(boutonPin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  radio.begin();
  radio.openWritingPipe(adresse);
  loadModeFromEEPROM(); // Charger le mode au démarrage
}

void loop() {
  
  // Lire l'état du bouton
  int etatBouton = digitalRead(boutonPin);

  // Incrémenter le mode si le bouton est enfoncé
  if (etatBouton == LOW) {
    delay(20); // Débouncing
    if (etatBouton == LOW) { // Vérifier de nouveau pour éviter les faux déclenchements
      mode++;
      if (mode > 8) {
        mode = 1;
      }
      setModeConfiguration();
      saveModeToEEPROM(); // Sauvegarder le mode dans l'EEPROM
    }
  }
  
  memset(valeursEnvoyees1, 0, sizeof(valeursEnvoyees1));
  memset(valeursEnvoyees2, 0, sizeof(valeursEnvoyees2));
  memset(valeursEnvoyees3, 0, sizeof(valeursEnvoyees3));
  memset(valeursEnvoyees4, 0, sizeof(valeursEnvoyees4));
  memset(valeursEnvoyees5, 0, sizeof(valeursEnvoyees5));
  memset(valeursEnvoyees6, 0, sizeof(valeursEnvoyees6));
  
  valeursEnvoyees1[0] = 1; // Indicateur pour les canaux 1-31
  valeursEnvoyees2[0] = 2; // Indicateur pour les canaux 32-62
  valeursEnvoyees3[0] = 3; // Indicateur pour les canaux 63-93
  valeursEnvoyees4[0] = 4; // Indicateur pour les canaux 94-124
  valeursEnvoyees5[0] = 5; // Indicateur pour les canaux 125-155
  valeursEnvoyees6[0] = 6; // Indicateur pour les canaux 156-186

  for (int i = 1; i <= 31; i++) {
    valeursEnvoyees1[i] = DMXSerial.read(i);
    valeursEnvoyees2[i] = DMXSerial.read(i + 31);
    valeursEnvoyees3[i] = DMXSerial.read(i + 62);
    valeursEnvoyees4[i] = DMXSerial.read(i + 93);
    valeursEnvoyees5[i] = DMXSerial.read(i + 124);
    valeursEnvoyees6[i] = DMXSerial.read(i + 155);
  }

  radio.write(&valeursEnvoyees1, sizeof(valeursEnvoyees1));
  delay(10); // Un petit délai pour éviter l'interférence
  radio.write(&valeursEnvoyees2, sizeof(valeursEnvoyees2));
  delay(10);
  radio.write(&valeursEnvoyees3, sizeof(valeursEnvoyees3));
  delay(10);
  radio.write(&valeursEnvoyees4, sizeof(valeursEnvoyees4));
  delay(10);
  radio.write(&valeursEnvoyees5, sizeof(valeursEnvoyees5));
  delay(10);
  radio.write(&valeursEnvoyees6, sizeof(valeursEnvoyees6));
}

void setModeConfiguration() {
  // Utiliser différentes adresses en fonction du mode
  if (mode == 1) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF01); // Adresse 1
    leds[0] = CRGB(255, 0, 0); // Rouge pour le mode 1
  } else if (mode == 2) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF02); // Adresse 2
    leds[0] = CRGB(0, 255, 0); // Vert pour le mode 2
  } else if (mode == 3) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF03); // Adresse 3
    leds[0] = CRGB(0, 0, 255); // Bleu pour le mode 3
  } else if (mode == 4) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF04); // Adresse 4
    leds[0] = CRGB(255, 255, 0); // Jaune pour le mode 4
  } else if (mode == 5) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF05); // Adresse 5
    leds[0] = CRGB(255, 0, 255); // Magenta pour le mode 5
  } else if (mode == 6) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF06); // Adresse 6
    leds[0] = CRGB(0, 255, 255); // Cyan pour le mode 6
  } else if (mode == 7) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF07); // Adresse 7
    leds[0] = CRGB(255, 255, 255); // Blanc pour le mode 7
  } else if (mode == 8) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF08); // Adresse 8
    leds[0] = CRGB(255, 165, 0); // Orange pour le mode 8
  }
  FastLED.show();
}

void saveModeToEEPROM() {
  EEPROM.write(0, mode); // Enregistrer le mode dans l'EEPROM à l'adresse 0
}

void loadModeFromEEPROM() {
  mode = EEPROM.read(0); // Charger le mode depuis l'EEPROM à l'adresse 0
}
