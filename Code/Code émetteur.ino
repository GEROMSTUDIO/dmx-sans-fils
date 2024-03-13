#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
#include <DMXSerial.h>
#include <FastLED.h>

#define NUM_LEDS 1 // Nombre de LEDs dans la bande
#define DATA_PIN 6  // Broche de données à laquelle la LED est connectée

CRGB leds[NUM_LEDS]; // Déclaration d'un tableau de LED

#define pinCE   10
#define pinCSN  9
#define tunnel  "PIPE1"        // Nom du tunnel de communication

RF24 radio(pinCE, pinCSN);
const byte adresse[6] = tunnel;                // Adresse du tunnel
byte valeursEnvoyees1[32]; // Canaux 1-31
byte valeursEnvoyees2[32]; // Canaux 32-62
byte valeursEnvoyees3[32]; // Canaux 63-93
byte valeursEnvoyees4[32]; // Canaux 94-124
byte valeursEnvoyees5[32]; // Canaux 125-155
byte valeursEnvoyees6[32]; // Canaux 156-186
const int boutonPin = A1;
int mode = 1;
uint8_t channelMap[8] = {1, 4, 7, 10, 13, 16, 19, 22}; // Écart de 3 entre chaque canal

void setup() {
  DMXSerial.init(DMXReceiver);
  pinMode(boutonPin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  radio.begin();
  radio.openWritingPipe(adresse); // Ouvrir un canal d'écriture
  loadModeFromEEPROM(); // Ajout pour charger le mode depuis l'EEPROM au démarrage
  setModeConfiguration();
  radio.setChannel(channelMap[mode - 1]); // Définir le canal initial
}

void loop() {
  
  // Lire l'état du bouton
  int etatBouton = digitalRead(boutonPin);

  // Incrémenter le mode si le bouton est enfoncé
  if (etatBouton == LOW) {
    delay(150); // Débouncing
    if (etatBouton == LOW) { // Vérifier de nouveau pour éviter les faux déclenchements
      mode++;
      if (mode > 8) {
        mode = 1;
      }
      setModeConfiguration();
      saveModeToEEPROM(); // Sauvegarder le mode dans l'EEPROM
      radio.setChannel(channelMap[mode - 1]); // Changer le canal RF en fonction du mode
    }
  }

    // Vérifier si aucun signal DMX n'a été reçu depuis plus de 3000 ms
  if (DMXSerial.noDataSince() > 3000) {
    // Clignoter la LED dans la couleur du mode actuel
    blinkLED();
   } else {
   setModeConfiguration();
  // Sinon, continuer à recevoir et à envoyer des données DMX
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
   }

  radio.write(&valeursEnvoyees1, sizeof(valeursEnvoyees1));
  delay(1); // Un petit délai pour éviter l'interférence
  radio.write(&valeursEnvoyees2, sizeof(valeursEnvoyees2));
  delay(1);
  radio.write(&valeursEnvoyees3, sizeof(valeursEnvoyees3));
   delay(1);
  radio.write(&valeursEnvoyees4, sizeof(valeursEnvoyees4));
  delay(1);
  radio.write(&valeursEnvoyees5, sizeof(valeursEnvoyees5));
  delay(1);
  radio.write(&valeursEnvoyees6, sizeof(valeursEnvoyees6));
}

// Fonction pour faire clignoter la LED avec la couleur du mode actuel
void blinkLED() {
  static unsigned long previousMillis = 0;
  static bool ledState = false;

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 500) { // Clignotement toutes les 500 millisecondes
    previousMillis = currentMillis;
    if (ledState) {
      leds[0] = CRGB::Black; // Éteindre la LED
    } else {
      // Utiliser la couleur du mode actuel pour le clignotement
      if (mode == 1) {
        leds[0] = CRGB::Red;
      } else if (mode == 2) {
        leds[0] = CRGB::Green;
      } else if (mode == 3) {
        leds[0] = CRGB::Blue;
      } else if (mode == 4) {
        leds[0] = CRGB::Yellow;
      } else if (mode == 5) {
        leds[0] = CRGB::Purple;
      } else if (mode == 6) {
        leds[0] = CRGB::Cyan;
      } else if (mode == 7) {
        leds[0] = CRGB::White;
      } else if (mode == 8) {
        leds[0] = CRGB::OrangeRed; // Utiliser une nuance plus rouge pour le mode 8
      }
    }
    FastLED.show();
    ledState = !ledState;
  }
}

void setModeConfiguration() {
  // Configuration du mode et de la couleur des LEDs selon le mode
  if (mode == 1) {
    leds[0] = CRGB::Red;
  } else if (mode == 2) {
    leds[0] = CRGB::Green;
  } else if (mode == 3) {
    leds[0] = CRGB::Blue;
  } else if (mode == 4) {
    leds[0] = CRGB::Yellow;
  } else if (mode == 5) {
    leds[0] = CRGB::Purple;
  } else if (mode == 6) {
    leds[0] = CRGB::Cyan;
  } else if (mode == 7) {
    leds[0] = CRGB::White;
  } else if (mode == 8) {
    leds[0] = CRGB::OrangeRed; // Utiliser une nuance plus rouge pour le mode 8
  }
  FastLED.show();
}

void saveModeToEEPROM() {
  EEPROM.write(0, mode); // Enregistrer le mode dans l'EEPROM à l'adresse 0
}

void loadModeFromEEPROM() {
  mode = EEPROM.read(0); // Charger le mode depuis l'EEPROM à l'adresse 0
}
