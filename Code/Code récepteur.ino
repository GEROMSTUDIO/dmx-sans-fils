#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
#include <FastLED.h>
#include <DmxSimple.h>

#define NUM_LEDS 1 // Nombre de LEDs dans la bande
#define DATA_PIN 6  // Broche de données à laquelle la led est connectée

CRGB leds[NUM_LEDS]; // Déclaration d'un tableau de LED

#define pinCE   10             // Broche CE du NRF24L01
#define pinCSN  9              // Broche CSN du NRF24L01
#define tunnel  "PIPE1"        // Nom du tunnel de communication

RF24 radio(pinCE, pinCSN);     // Instanciation du NRF24L01

const byte adresse[6] = tunnel;       // Adresse du tunnel
char message[32];                     // Données reçues
int buttonPin = A1;                   // Broche du bouton
int channelIndex = 0;                 // Index du canal courant
int channelMap[] = {1, 4, 7, 10, 13, 16, 19, 22}; // Canaux disponibles
unsigned long timeout = 5000;         // Délai d'attente pour trouver un canal disponible en millisecondes
bool searching = false;               // Indique si la recherche est en cours
bool boutonEtatPrecedent = HIGH; // État précédent du bouton
unsigned long tempsDebutAppui = 0; // Temps au début de l'appui
int mode = 1;
byte valeursRecues[32]; // Pour recevoir les données
unsigned long lastReceiveTime = 0; // Temps de la dernière réception

void setup() {
  Serial.begin(9600);
  DmxSimple.usePin(3);
  DmxSimple.maxChannel(186); // Configuration pour un maximum de 186 canaux
  pinMode(buttonPin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  radio.begin();
  radio.openReadingPipe(0, adresse);
  loadModeFromEEPROM(); // Ajout pour charger le mode depuis l'EEPROM au démarrage
  //radio.setPALevel(RF24_PA_MIN);
  setModeConfiguration();
  radio.setChannel(channelMap[mode - 1]); // Changer le canal RF en fonction du mode
  radio.startListening(); // Commencer l'écoute initialement
}

void loop() {
  unsigned long currentMillis = millis();
  bool boutonEtat = digitalRead(buttonPin);

if (radio.available()) {
    setModeConfiguration();
    Serial.println("Données reçues !!");
    memset(valeursRecues, 0, sizeof(valeursRecues)); // Réinitialisation du buffer
    radio.read(&valeursRecues, sizeof(valeursRecues));
    lastReceiveTime = currentMillis; // Mettre à jour le temps de la dernière réception
    
    byte indicateur = valeursRecues[0];

    // Logique adaptée pour utiliser DmxSimple selon l'indicateur
    for (int b = 1; b <= 31; b++) {
      int channelOffset = (indicateur - 1) * 31;
      DmxSimple.write(b + channelOffset, valeursRecues[b]);
    }
  }
  
  // Incrémenter le mode si le bouton est enfoncé
  if (boutonEtat == LOW && boutonEtatPrecedent == HIGH) { // Correction du nom de la variable
    delay(150); // Debouncing
    if (digitalRead(buttonPin) == LOW) { // Vérification après debounce
      mode++;
      if (mode > 8) { mode = 1; }
      saveModeToEEPROM();
      setModeConfiguration();
      radio.stopListening(); // Arrêter l'écoute
      radio.setChannel(channelMap[mode - 1]); // Changer le canal RF en fonction du mode
      radio.startListening(); // Reprendre l'écoute
    }
  }

  // Vérification si le bouton est enfoncé depuis plus de 10 secondes
  if (boutonEtat == LOW && boutonEtatPrecedent == HIGH) {
    tempsDebutAppui = millis(); // Enregistrer le moment où le bouton est enfoncé
  }
  if (boutonEtat == HIGH && boutonEtatPrecedent == LOW && (currentMillis - tempsDebutAppui) >= 10000) {
    searchChannel(); // Activer la fonction searchChannel si le bouton est relâché après avoir été enfoncé pendant plus de 10 secondes
  }

  // Mise à jour de l'état précédent du bouton
  boutonEtatPrecedent = boutonEtat;

  // Si la recherche est en cours et que le délai d'attente est écoulé, arrêter la recherche
  if (searching && currentMillis - timeout >= timeout) {
    searching = false;
    Serial.println("Aucun canal disponible trouvé. Arrêt de la recherche.");
    setModeConfiguration();
  }
   
   // Vérification si aucun signal n'a été reçu depuis plus de 5000 millisecondes
  if (currentMillis - lastReceiveTime >= 3000) {
    // Clignoter la LED dans la couleur du mode actuel
    blinkLED();
  }
}

// Fonction pour la recherche de canal disponible
void searchChannel() {
  Serial.println("Début de la recherche de canal disponible...");
  searching = true;
  for (int i = 0; i < sizeof(channelMap) / sizeof(channelMap[0]); i++) { // Correction du nom du tableau
    blinkLEDDuringSearch();
    radio.stopListening(); // Arrêter l'écoute
    radio.setChannel(channelMap[i]); // Changer de canal
    radio.startListening(); // Reprendre l'écoute
    unsigned long startMillis = millis();
    
    while (millis() - startMillis < timeout) {
      if (radio.available()) {
        Serial.print("Canal RF trouvé : ");
        Serial.println(channelMap[i]);
        mode = i + 1;
        setModeConfiguration();
        saveModeToEEPROM();
        searching = false;
        return; // Sortir de la fonction après avoir trouvé un canal
      }
    }
  }
}

// Fonction pour faire clignoter la LED pendant la recherche de canal
void blinkLEDDuringSearch() {
  static unsigned long previousMillis = 0;
  static bool ledState = false;

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 200) { // Clignotement toutes les 200 millisecondes
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
  EEPROM.write(0, mode);
}

void loadModeFromEEPROM() {
  mode = EEPROM.read(0);
}
