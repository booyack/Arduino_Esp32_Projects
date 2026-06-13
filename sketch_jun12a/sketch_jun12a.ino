#include <ELECHOUSE_CC1101_SRC_Arduino.h>

// Nasza nowa, przetestowana konfiguracja pinów
#define CC1101_SCK   4
#define CC1101_MISO  5
#define CC1101_MOSI  6
#define CC1101_CSN   7
#define CC1101_GDO0  3

void setup() {
  Serial.begin(115200);
  
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000));

  Serial.println("\n=== INICJALIZACJA SNIFFERA CC1101 ===");

  // Ręczne przypisanie sprawdzonych pinów SPI do biblioteki
  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
  
  // Przypisanie pinu odbiorczego GDO0
  ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);

  if (ELECHOUSE_cc1101.getCC1101()) {
    Serial.println("SUKCES: Połączenie z CC1101 ustanowione!");
  } else {
    Serial.println("BŁĄD: Brak kontaktu z radiem. Sprawdź konfigurację.");
    while (1);
  }

  // Konfiguracja radia pod standard Aluprof (433.92 MHz, modulacja ASK/OOK)
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(433.92);
  ELECHOUSE_cc1101.SetRx(); // Przełączenie w tryb odbioru
  
  pinMode(CC1101_GDO0, INPUT);
  Serial.println("Rozpoczęto nasłuchiwanie na częstotliwości 433.92 MHz...");
}

void loop() {
  // Prosty, niskopoziomowy podgląd stanu pinu GDO0
  // Jeśli pilot nadaje, pin zacznie gwałtownie zmieniać stan (0 i 1)
  if (digitalRead(CC1101_GDO0) == HIGH) {
    Serial.print("1");
  } else {
    // Żeby nie zalewać konsoli, drukujemy kropki dla stanu niskiego
    // (Możesz usunąć/zmienić tę logikę po przejściu na pełny dekoder sygnału)
    delayMicroseconds(500); 
  }
}