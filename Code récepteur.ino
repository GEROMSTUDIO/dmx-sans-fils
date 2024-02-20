#include <EEPROM.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DmxSimple.h>
#include <Adafruit_NeoPixel.h>

RF24 radio(10, 9); // Broches CE, CSN
const int boutonPin = A1;
const int ledPin = 6;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, ledPin, NEO_GRB + NEO_KHZ800);
int mode = 1;

void setup() {
  radio.begin();
  pinMode(boutonPin, INPUT_PULLUP);
  strip.begin();
  loadModeFromEEPROM(); // Charger le mode depuis l'EEPROM
  setModeConfiguration();
  DmxSimple.usePin(3); // Utiliser la broche 3 pour la communication DMX
}

void loop() {
  byte buffer[32];

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

  // Écrire les données DMX
  if (radio.available()) {
    radio.read(&buffer, sizeof(buffer));

    for (int i = 1; i <= 512; i++) {
      DmxSimple.write(i, buffer[(i - 1) % 32]);
    }
  }
}

void setModeConfiguration() {
  // Utiliser différentes adresses en fonction du mode
  if (mode == 1) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF01); // Adresse 1
    strip.setPixelColor(0, strip.Color(255, 0, 0)); // Rouge pour le mode 1
  } else if (mode == 2) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF02); // Adresse 2
    strip.setPixelColor(0, strip.Color(0, 255, 0)); // Vert pour le mode 2
  } else if (mode == 3) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF03); // Adresse 3
    strip.setPixelColor(0, strip.Color(0, 0, 255)); // Bleu pour le mode 3
  } else if (mode == 4) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF04); // Adresse 4
    strip.setPixelColor(0, strip.Color(255, 255, 0)); // Jaune pour le mode 4
  } else if (mode == 5) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF05); // Adresse 5
    strip.setPixelColor(0, strip.Color(255, 0, 255)); // Magenta pour le mode 5
  } else if (mode == 6) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF06); // Adresse 6
    strip.setPixelColor(0, strip.Color(0, 255, 255)); // Cyan pour le mode 6
  } else if (mode == 7) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF07); // Adresse 7
    strip.setPixelColor(0, strip.Color(255, 255, 255)); // Blanc pour le mode 7
  } else if (mode == 8) {
    radio.openReadingPipe(1, (uint64_t)0xABCDEF08); // Adresse 8
    strip.setPixelColor(0, strip.Color(255, 165, 0)); // Orange pour le mode 8
  }
  strip.show();
}

void saveModeToEEPROM() {
  EEPROM.write(0, mode); // Enregistrer le mode dans l'EEPROM à l'adresse 0
}

void loadModeFromEEPROM() {
  mode = EEPROM.read(0); // Charger le mode depuis l'EEPROM à l'adresse 0
}
