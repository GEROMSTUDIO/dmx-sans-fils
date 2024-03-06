#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
#include <DmxSimple.h>
#include <FastLED.h>

#define NUM_LEDS 1 // Nombre de LEDs dans la bande
#define DATA_PIN 6  // Broche de données à laquelle la led est connectée

CRGB leds[NUM_LEDS]; // Déclaration d'un tableau de LED

#define pinCE   10
#define pinCSN  9

RF24 radio(pinCE, pinCSN);
const byte adresse[6] = "00001";
byte valeursRecues[32]; // Pour recevoir les données
const int boutonPin = A1;
int mode = 1;
uint8_t channelMap[8] = {1, 4, 7, 10, 13, 16, 19, 22}; // Écart de 3 entre chaque canal

void setup() {
  Serial.begin(9600);
  DmxSimple.usePin(3);
  DmxSimple.maxChannel(186); // Configuration pour un maximum de 186 canaux
  pinMode(boutonPin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  radio.begin();
  radio.openReadingPipe(1, adresse);
  loadModeFromEEPROM(); // Ajout pour charger le mode depuis l'EEPROM au démarrage
  setModeConfiguration();
  radio.setChannel(channelMap[mode - 1]); // Changer le canal RF en fonction du mode
  radio.startListening();
}

void loop() {
  // Lire l'état du bouton
  int etatBouton = digitalRead(boutonPin);

  // Incrémenter le mode si le bouton est enfoncé
  if (etatBouton == LOW) {
    delay(150); // Debouncing
    if (digitalRead(boutonPin) == LOW) { // Vérification après debounce
      mode++;
      if (mode > 8) mode = 1;
      saveModeToEEPROM();
      setModeConfiguration();
      radio.setChannel(channelMap[mode - 1]); // Changer le canal RF en fonction du mode
    }
  }

  if (radio.available()) {
    memset(valeursRecues, 0, sizeof(valeursRecues)); // Réinitialisation du buffer
    radio.read(&valeursRecues, sizeof(valeursRecues));

    byte indicateur = valeursRecues[0];

    // Logique adaptée pour utiliser DmxSimple selon l'indicateur
    for (int i = 1; i <= 31; i++) {
      int channelOffset = (indicateur - 1) * 31;
      DmxSimple.write(i + channelOffset, valeursRecues[i]);
    }
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
  EEPROM.write(0, mode);
}

void loadModeFromEEPROM() {
  mode = EEPROM.read(0);
}
