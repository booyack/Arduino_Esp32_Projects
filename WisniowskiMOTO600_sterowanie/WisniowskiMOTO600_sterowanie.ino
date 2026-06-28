#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <SPI.h>
#include <Preferences.h>

// Definicje pinów (Po zweryfikowanej korekcie sprzętowej)
const int PIN_GDO0 = 2;  // Czysty pin danych (GDO0)
const int PIN_CSN  = 7;
const int PIN_SCK  = 4;
const int PIN_MOSI = 6;
const int PIN_MISO = 5;

// Unikalny ID Twojego wirtualnego pilota
const uint32_t REMOTE_ID = 0xABC123; 

Preferences preferences;

// Definicje komend Somfy RTS
const byte CMD_MY    = 0x1; // Stop / Pozycja ulubiona (MY)
const byte CMD_UP    = 0x2; // Otwórz / W górę (UP)
const byte CMD_DOWN  = 0x4; // Zamknij / W dół (DOWN)
const byte CMD_PROG  = 0x8; // Programowanie / Parowanie (PROG)

// Funkcja generująca precyzyjne impulsy sprzętowe
inline void sendPulse(boolean state, int duration) {
    digitalWrite(PIN_GDO0, state ? HIGH : LOW);
    ets_delay_us(duration); 
}

void transmitSomfy(byte command, uint16_t rollingCode) {
    byte frame[7];
    byte key = 0xA7; // Klucz szyfrujący Somfy

    frame[0] = key;
    frame[1] = command << 4; 
    frame[2] = (rollingCode >> 8) & 0xFF;
    frame[3] = rollingCode & 0xFF;
    frame[4] = (REMOTE_ID >> 16) & 0xFF;
    frame[5] = (REMOTE_ID >> 8) & 0xFF;
    frame[6] = REMOTE_ID & 0xFF;

    // 1. Obliczanie sumy kontrolnej (4-bit XOR dla 14 półbajtów)
    byte checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum ^= (frame[i] >> 4) ^ (frame[i] & 0x0F);
    }
    frame[1] |= (checksum & 0x0F); 

    // 2. Obfuskacja (XORowanie kolejnych bajtów)
    for (int i = 1; i < 7; i++) {
        frame[i] ^= frame[i - 1];
    }

    // 3. Konfiguracja rejestrów CC1101 dla czystego asynchronicznego OOK
    ELECHOUSE_cc1101.SpiStrobe(0x36);         // SIDLE: Bezczynność
    ELECHOUSE_cc1101.SpiWriteReg(0x02, 0x0D); // IOCFG0: GDO0 jako asynchroniczne wejście szeregowe
    ELECHOUSE_cc1101.SpiWriteReg(0x08, 0x30); // PKTCTRL0: Wyłączenie FIFO i pakietowania
    ELECHOUSE_cc1101.SpiWriteReg(0x22, 0x11); // FREND0: Powiązanie logicznej '1' z mocą PATABLE[1]
    
    // Tabela mocy OOK (0x00 = brak fali, 0xC0 = pełna moc nadawania +10dBm)
    byte paTable[2] = {0x00, 0xC0};
    ELECHOUSE_cc1101.SpiWriteBurstReg(0x3E, paTable, 2);

    // Kalibracja i fizyczne uruchomienie nadajnika (STX)
    ELECHOUSE_cc1101.SpiStrobe(0x33); 
    delay(2); 
    ELECHOUSE_cc1101.SpiStrobe(0x35);         
    delay(2); 

    pinMode(PIN_GDO0, OUTPUT);

    // Nadajemy serię ramek: 7 dla zwykłych komend, 10 dla trybu programowania (PROG)
    int repetitions = (command == CMD_PROG) ? 10 : 7; 

    for (int repeat = 0; repeat < repetitions; repeat++) {
        if (repeat == 0) {
            // RAMKA 1: Wake-up + 4 impulsy HW Sync
            sendPulse(true, 9415);
            sendPulse(false, 4707);
            for (int i = 0; i < 4; i++) {
                sendPulse(true, 2416);
                sendPulse(false, 2416);
            }
        } else {
            // RAMKI KOLEJNE: Dokładnie 7 impulsów HW Sync
            for (int i = 0; i < 7; i++) {
                sendPulse(true, 2416);
                sendPulse(false, 2416);
            }
        }
        
        // Software Sync
        sendPulse(true, 4750);
        sendPulse(false, 1200);

        // Nadawanie danych (Polaryzacja Manchester Somfy: 1 = LOW->HIGH, 0 = HIGH->LOW)
        for (int byteIdx = 0; byteIdx < 7; byteIdx++) {
            for (int bitIdx = 7; bitIdx >= 0; bitIdx--) {
                boolean bit = bitRead(frame[byteIdx], bitIdx);
                if (bit) {
                    sendPulse(false, 605);
                    sendPulse(true, 605);
                } else {
                    sendPulse(true, 605);
                    sendPulse(false, 605);
                }
            }
        }

        // Wygaszenie fali przed przerwą międzyramkową
        digitalWrite(PIN_GDO0, LOW);
        delay(30); 
    }

    // 4. Powrót układu do bezpiecznego trybu bezczynności i nasłuchu RX
    pinMode(PIN_GDO0, INPUT);
    ELECHOUSE_cc1101.SpiStrobe(0x36); // SIDLE
    ELECHOUSE_cc1101.SetRx();         
}

uint16_t getAndIncrementRollingCode() {
    preferences.begin("somfy_rts", false);
    uint16_t code = preferences.getUShort("counter", 0);
    code++;
    preferences.putUShort("counter", code);
    preferences.end();
    return code;
}

void setup() {
    Serial.begin(115200);
    while(!Serial); 

    // Inicjalizacja sprzętu SPI
    ELECHOUSE_cc1101.setSpiPin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CSN);
    ELECHOUSE_cc1101.setGDO0(PIN_GDO0);
    ELECHOUSE_cc1101.Init();

    if (!ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("CC1101: Blad komunikacji SPI!");
        while (1);
    }

    // Konfiguracja częstotliwości i bazowej modulacji
    ELECHOUSE_cc1101.setMHZ(433.42);   
    ELECHOUSE_cc1101.setModulation(2); // ASK/OOK
    ELECHOUSE_cc1101.SetRx();

    // Odczyt licznika z NVS
    preferences.begin("somfy_rts", true);
    uint16_t currentInNVS = preferences.getUShort("counter", 0);
    preferences.end();

    Serial.println("\n--- STEROWNIK MOTO 600 RTS GOTOWY (PIN 2) ---");
    Serial.print("Aktualny Rolling Code w pamięci: ");
    Serial.println(currentInNVS);
    Serial.println("Komendy: 'p' (PROG), 'u' (UP/Otwórz), 'd' (DOWN/Zamknij), 's' (STOP/MY)");
}

void loop() {
    if (Serial.available() > 0) {
        char input = Serial.read();
        if (input == '\n' || input == '\r') return;

        uint16_t currentCounter = 0;

        switch (input) {
            case 'p':
            case 'P':
                currentCounter = getAndIncrementRollingCode();
                Serial.print("\nWysyłam komendę parowania PROG | Rolling Code: ");
                Serial.println(currentCounter);
                transmitSomfy(CMD_PROG, currentCounter);
                break;
                
            case 'u':
            case 'U':
                currentCounter = getAndIncrementRollingCode();
                Serial.print("\nWysyłam komendę otwierania UP | Rolling Code: ");
                Serial.println(currentCounter);
                transmitSomfy(CMD_UP, currentCounter);
                break;
                
            case 'd':
            case 'D':
                currentCounter = getAndIncrementRollingCode();
                Serial.print("\nWysyłam komendę zamykania DOWN | Rolling Code: ");
                Serial.println(currentCounter);
                transmitSomfy(CMD_DOWN, currentCounter);
                break;
                
            case 's':
            case 'S':
                currentCounter = getAndIncrementRollingCode();
                Serial.print("\nWysyłam komendę STOP / MY | Rolling Code: ");
                Serial.println(currentCounter);
                transmitSomfy(CMD_MY, currentCounter);
                break;

            default:
                Serial.println("Nieznana komenda. Użyj: p, u, d, s");
                break;
        }
    }
}