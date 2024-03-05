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

void setup() {
  Serial.begin(9600);
  DmxSimple.usePin(3);
  DmxSimple.maxChannel(186); // Configuration pour un maximum de 186 canaux
  pinMode(boutonPin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  radio.begin();
  radio.openReadingPipe(0, adresse);
  radio.startListening();
  loadModeFromEEPROM(); // Ajout pour charger le mode depuis l'EEPROM au démarrage
}

void loop() {
  // Lire l'état du bouton
  int etatBouton = digitalRead(boutonPin);

  // Incrémenter le mode si le bouton est enfoncé
  if (etatBouton == LOW) {
    delay(20); // Debouncing
    if (digitalRead(boutonPin) == LOW) { // Vérification après debounce
      mode++;
      if (mode > 8) mode = 1;
      setModeConfiguration();
      saveModeToEEPROM();
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
  switch (mode) {
    case 1: leds[0] = CRGB::Red; break;
    case 2: leds[0] = CRGB::Green; break;
    case 3: leds[0] = CRGB::Blue; break;
    case 4: leds[0] = CRGB::Yellow; break;
    case 5: leds[0] = CRGB::Magenta; break;
    case 6: leds[0] = CRGB::Cyan; break;
    case 7: leds[0] = CRGB::White; break;
    case 8: leds[0] = CRGB(255, 165, 0); break; // Orange
    default: leds[0] = CRGB::Black; // Éteindre si mode inconnu
  }
  FastLED.show();
  // Note: Pas de modification nécessaire pour l'adresse NRF24L01 dans cet extrait
}

void saveModeToEEPROM() {
  EEPROM.write(0, mode);
}

void loadModeFromEEPROM() {
  mode = EEPROM.read(0);
  if (mode < 1 || mode > 8) mode = 1; // Assurer que le mode est dans une plage valide
}
