#include <Arduino.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>

// Nowy układ pinów SPI dla ESP32-S3 SuperMini
#define CC1101_SCK   4
#define CC1101_MISO  5
#define CC1101_MOSI  6
#define CC1101_CS    7

// Nowy pin danych GDO0 (GD0) podłączony do GPIO 3
#define CC1101_GDO0  3

#define MAX_CHANGES 200
volatile unsigned long timings[MAX_CHANGES];
volatile unsigned int changeCount = 0;
volatile unsigned long lastTime = 0;
volatile bool packetReady = false;

void IRAM_ATTR handleRFChange() {
    const unsigned long currentTime = micros();
    const unsigned long duration = currentTime - lastTime;
    lastTime = currentTime;

    if (packetReady) return;

    if (duration > 12000) {
        if (changeCount > 40) {
            packetReady = true;
        } else {
            changeCount = 0;
        }
        return;
    }

    if (changeCount < MAX_CHANGES) {
        timings[changeCount++] = duration;
    }
}

void setup() {
    Serial.begin(115200);
    while(!Serial) delay(10);

    Serial.println("Inicjalizacja CC1101 z nowym układem pinów...");

    // Inicjalizacja magistrali SPI na nowych pinach
    SPI.begin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);

    // Przekazanie nowych pinów do biblioteki SmartRC-CC1101
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    
    if (ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("CC1101 znaleziony poprawnie na nowych pinach!");
    } else {
        Serial.println("BŁĄD: Nie znaleziono CC1101. Sprawdź połączenia (GPIO 4, 5, 6, 7).");
        while(1);
    }

    // Konfiguracja parametrów pod pilota Aluprof
    ELECHOUSE_cc1101.Init();                 
    ELECHOUSE_cc1101.setModulation(2);       // ASK/OOK
    ELECHOUSE_cc1101.setMHZ(433.92);         // Częstotliwość
    ELECHOUSE_cc1101.setRxBW(270);           // Pasmo ~270 kHz
    ELECHOUSE_cc1101.setDRate(4.8);          // Data Rate
    ELECHOUSE_cc1101.setPA(10);              
    
    // Przełączenie w tryb odbiornika asynchronicznego
    ELECHOUSE_cc1101.SetRx();                
    
    // Konfiguracja przerwania na nowym pinie GPIO 3
    pinMode(CC1101_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), handleRFChange, CHANGE);
    
    Serial.println("CC1101 nasłuchuje na GPIO 3...");
}

void loop() {
    if (packetReady) {
        detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));

        int syncIndex = -1;
        for (int i = 0; i < changeCount; i++) {
            if (timings[i] > 4000 && timings[i] < 6000) {
                syncIndex = i;
                break;
            }
        }

        if (syncIndex != -1) {
            String binaryString = "";
            for (int i = syncIndex + 1; i < changeCount - 1; i += 2) {
                unsigned long highTime = timings[i];
                unsigned long lowTime = timings[i+1];
                
                if (highTime + lowTime > 2000 || highTime + lowTime < 800) continue; 

                if (highTime > lowTime) {
                    binaryString += "1";
                } else {
                    binaryString += "0";
                }
            }

            if (binaryString.length() >= 40) {
                Serial.print("Odebrano Aluprof DC313 HEX: ");
                for (int i = 0; i < 40; i += 8) {
                    String byteStr = binaryString.substring(i, i + 8);
                    long byteVal = strtol(byteStr.c_str(), NULL, 2);
                    if (byteVal < 16) Serial.print("0");
                    Serial.print(byteVal, HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }
        }

        changeCount = 0;
        packetReady = false;
        lastTime = micros();
        attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), handleRFChange, CHANGE);
    }
}