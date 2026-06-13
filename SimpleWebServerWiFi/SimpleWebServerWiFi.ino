#include <SPI.h>

// Definicje pinów według Twojego układu
#define CC1101_CSN   8
#define CC1101_MOSI  9
#define CC1101_MISO  10
#define CC1101_SCK   11

void setup() {
  Serial.begin(115200);
  
  // Czekaj na Serial Monitor
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)); 

  Serial.println("\n=== NISKOPOZIOMOWY TEST SPRZĘTOWY SPI ===");

  // Ręczna konfiguracja pinu Chip Select
  pinMode(CC1101_CSN, OUTPUT);
  digitalWrite(CC1101_CSN, HIGH);

  // Inicjalizacja sprzętowej magistrali SPI w ESP32-S3
  // Kolejność: SCK, MISO, MOSI, SS
  SPI.begin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
}

void loop() {
  // CC1101 komunikuje się stabilnie przy 1 MHz, w trybie SPI_MODE0
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  
  // 1. Odczyt rejestru VERSION (Adres rejestru statusu 0x31 | bit burst 0x40 = 0x71)
  digitalWrite(CC1101_CSN, LOW);
  SPI.transfer(0x71); 
  byte version = SPI.transfer(0x00);
  digitalWrite(CC1101_CSN, HIGH);
  
  delayMicroseconds(10);

  // 2. Odczyt rejestru PARTNUM (Adres 0x30 | bit burst 0x40 = 0x70)
  digitalWrite(CC1101_CSN, LOW);
  SPI.transfer(0x70);
  byte partnum = SPI.transfer(0x00);
  digitalWrite(CC1101_CSN, HIGH);
  
  SPI.endTransaction();

  // Wyświetlenie surowych danych z krzemu
  Serial.print("Odpowiedź układu -> PARTNUM: ");
  Serial.print(partnum, HEX);
  Serial.print(" | VERSION: ");
  Serial.println(version, HEX);

  delay(1000);
}