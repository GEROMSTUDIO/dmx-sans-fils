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
  DmxSimple.maxChannel(186); // Configuration pour un maximum de 248 canaux
  pinMode(boutonPin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  radio.begin();
  radio.openReadingPipe(0, adresse);
  radio.startListening();
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
  
  if (radio.available()) {
    memset(valeursRecues, 0, sizeof(valeursRecues)); // Réinitialisation du buffer
    radio.read(&valeursRecues, sizeof(valeursRecues));

    byte indicateur = valeursRecues[0];

    if (indicateur == 1) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i, valeursRecues[i]);
      }
    } else if (indicateur == 2) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i + 31, valeursRecues[i]);
      }
    } else if (indicateur == 3) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i + 62, valeursRecues[i]);
      }
    } else if (indicateur == 4) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i + 93, valeursRecues[i]);
      }
    } else if (indicateur == 5) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i + 124, valeursRecues[i]);
      }
    } else if (indicateur == 6) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i + 155, valeursRecues[i]);
      }
    } else if (indicateur == 7) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i + 186, valeursRecues[i]);
      }
    } else if (indicateur == 8) {
      for (int i = 1; i <= 31; i++) {
        DmxSimple.write(i + 217, valeursRecues[i]);
      }
    }
  }
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
