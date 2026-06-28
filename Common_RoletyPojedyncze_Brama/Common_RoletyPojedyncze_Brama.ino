#include <WiFi.h>
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/roller_shutter.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <SPI.h>
#include <Preferences.h>

#define CC1101_SCK    4
#define CC1101_MISO   5
#define CC1101_MOSI   6
#define CC1101_CSN    7
#define CC1101_GDO0   3

const int LED_PIN = 8;

const char* SSID_WIFI = "WypadZBaru";
const char* PASS_WIFI = "Multiwitamina33";

const char* SUPLA_SERVER = "svr160.supla.org";
const char* SUPLA_EMAIL  = "michalbujak1987@gmail.com";

const char GUID[16]    = {0x2D, 0xA1, 0xC5, 0x11, 0x06, 0x3D, 0x06, 0x44, 0xEE, 0x55, 0xFF, 0x66, 0x11, 0x22, 0x33, 0x47};
const char AUTHKEY[16] = {0x44, 0x33, 0x22, 0x11, 0x66, 0xFF, 0x55, 0xEE, 0x44, 0xDD, 0x33, 0xCC, 0x22, 0xBB, 0x11, 0xAA};

const uint8_t CMD_GORA = 0x11;
const uint8_t CMD_DOL  = 0x33;
const uint8_t CMD_STOP = 0x55;

// Nowe, czyste ID wirtualnego pilota Somfy
const uint32_t REMOTE_ID = 0xABC126; 
const byte CMD_MY   = 0x1; 
const byte CMD_UP   = 0x2; 
const byte CMD_DOWN = 0x4; 
const byte CMD_PROG = 0x8; 

Supla::ESPWifi wifi(SSID_WIFI, PASS_WIFI);
Preferences preferences;

uint16_t globalRollingCode = 1;

struct RollerPins {
  uint8_t pinUp;
  uint8_t pinDown;
  const char* name;
  bool lastUp;
  bool lastDown;
};

// Obsługa 10 rolet Aluprof
RollerPins rolety[10] = {
  {1, 2, "Wszystkie rolety", false, false},
  {9, 10, "Taras", false, false},
  {11, 12, "Kuchnia Zachod", false, false},
  {13, 14, "Kuchnia Polnoc", false, false},
  {21, 35, "Garaz", false, false},
  {36, 37, "Kotlownia", false, false},
  {38, 39, "Madzia", false, false},
  {40, 41, "Dodek", false, false},
  {42, 45, "Polcia", false, false},
  {46, 47, "Sypialnia", false, false}
};

// Zmienne do śledzenia stanu wirtualnych pinów bramy Somfy
const int BRAMA_PIN_UP = 17;
const int BRAMA_PIN_DOWN = 18;
bool lastBramaUp = false;
bool lastBramaDown = false;

bool bootReady = false;

inline void sendPulse(boolean state, int duration) {
    digitalWrite(CC1101_GDO0, state ? HIGH : LOW);
    ets_delay_us(duration); 
}

void nadajBitAluprof(bool bit) {
  if (bit) {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(720);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(360);
  } else {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(360);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(720);
  }
}

void nadajPaczkeAluprof(uint8_t kanal, uint8_t komenda) {
  ELECHOUSE_cc1101.Init(); 
  ELECHOUSE_cc1101.setMHZ(433.92);
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.SpiWriteReg(0x02, 0x0D); 
  ELECHOUSE_cc1101.SpiWriteReg(0x08, 0x30); 
  ELECHOUSE_cc1101.SpiWriteReg(0x1B, 0x03); 
  ELECHOUSE_cc1101.SpiWriteReg(0x21, 0x11); 
  
  uint8_t ramka[5] = {0x2A, 0x3D, 0x06, (uint8_t)(0x40 | (kanal & 0x0F)), komenda};
  ELECHOUSE_cc1101.SetTx();
  pinMode(CC1101_GDO0, OUTPUT);

  for (int powtorzenie = 0; powtorzenie < 6; powtorzenie++) {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(4800);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(1600);
    for (int bajtIdx = 0; bajtIdx < 5; bajtIdx++) {
      for (int bitIdx = 7; bitIdx >= 0; bitIdx--) {
        nadajBitAluprof((ramka[bajtIdx] >> bitIdx) & 0x01);
      }
    }
    digitalWrite(CC1101_GDO0, LOW);
    delay(10);
  }
  pinMode(CC1101_GDO0, INPUT);
  ELECHOUSE_cc1101.SetRx();
}

void transmitSomfy(byte command, uint16_t rollingCode) {
    byte frame[7];
    byte key = 0xA7; 

    frame[0] = key;
    frame[1] = command << 4; 
    frame[2] = (rollingCode >> 8) & 0xFF;
    frame[3] = rollingCode & 0xFF;
    frame[4] = (REMOTE_ID >> 16) & 0xFF;
    frame[5] = (REMOTE_ID >> 8) & 0xFF;
    frame[6] = REMOTE_ID & 0xFF;

    byte checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum ^= (frame[i] >> 4) ^ (frame[i] & 0x0F);
    }
    frame[1] |= (checksum & 0x0F); 

    for (int i = 1; i < 7; i++) {
        frame[i] ^= frame[i - 1];
    }

    ELECHOUSE_cc1101.Init(); 
    ELECHOUSE_cc1101.setMHZ(433.42);   
    ELECHOUSE_cc1101.setModulation(2); 
    
    ELECHOUSE_cc1101.SpiStrobe(0x36);         
    ELECHOUSE_cc1101.SpiWriteReg(0x02, 0x0D); 
    ELECHOUSE_cc1101.SpiWriteReg(0x08, 0x30); 
    ELECHOUSE_cc1101.SpiWriteReg(0x22, 0x11); 
    
    byte paTable[2] = {0x00, 0xC0};
    ELECHOUSE_cc1101.SpiWriteBurstReg(0x3E, paTable, 2);

    ELECHOUSE_cc1101.SpiStrobe(0x33); 
    delay(2); 
    ELECHOUSE_cc1101.SpiStrobe(0x35);          
    delay(2); 

    pinMode(CC1101_GDO0, OUTPUT);

    int repetitions = (command == CMD_PROG) ? 10 : 7; 

    for (int repeat = 0; repeat < repetitions; repeat++) {
        if (repeat == 0) {
            sendPulse(true, 9415);
            sendPulse(false, 4707);
            for (int i = 0; i < 4; i++) {
                sendPulse(true, 2416);
                sendPulse(false, 2416);
            }
        } else {
            for (int i = 0; i < 7; i++) {
                sendPulse(true, 2416);
                sendPulse(false, 2416);
            }
        }
        
        sendPulse(true, 4750);
        sendPulse(false, 1200);

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
        digitalWrite(CC1101_GDO0, LOW);
        delay(30); 
    }

    pinMode(CC1101_GDO0, INPUT);
    ELECHOUSE_cc1101.SpiStrobe(0x36); 
    ELECHOUSE_cc1101.SetRx();         
}

uint16_t getAndIncrementRollingCode() {
    globalRollingCode++; 
    preferences.begin("somfy_rts", false);
    preferences.putUShort("counter", globalRollingCode);
    preferences.end();
    return globalRollingCode;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 

  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
  ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
  ELECHOUSE_cc1101.Init();
  
  if (!ELECHOUSE_cc1101.getCC1101()) {
      Serial.println("CC1101: Blad komunikacji SPI!");
      while (1);
  }

  ELECHOUSE_cc1101.SetRx();
  pinMode(CC1101_GDO0, INPUT);

  preferences.begin("somfy_rts", true);
  globalRollingCode = preferences.getUShort("counter", 0);
  preferences.end();
  
  if(globalRollingCode == 0) {
    globalRollingCode = 1;
  }
  
  Serial.printf("START. Licznik w RAM: %d\n", globalRollingCode);

  // Rejestracja 10 tradycyjnych rolet (Aluprof)
  for (int i = 0; i < 10; i++) {
    auto shutter = new Supla::Control::RollerShutter(rolety[i].pinUp, rolety[i].pinDown);
    shutter->setInitialCaption(rolety[i].name); 
  }

  // Rejestracja kanału bramy Somfy jako standardowy obiekt na pinach 17 i 18
  auto somfyGarage = new Supla::Control::RollerShutter(BRAMA_PIN_UP, BRAMA_PIN_DOWN);
  somfyGarage->setInitialCaption("Brama Garaz (Somfy)");

  SuplaDevice.setName("Rolety i Brama");
  SuplaDevice.begin(GUID, SUPLA_SERVER, SUPLA_EMAIL, AUTHKEY);
}

void loop() {
  SuplaDevice.iterate(); // Chmura w tle ustawia stany pinów 17 i 18

  if (!bootReady) {
    for (int i = 0; i < 10; i++) {
      rolety[i].lastUp = digitalRead(rolety[i].pinUp);
      rolety[i].lastDown = digitalRead(rolety[i].pinDown);
    }
    // Synchronizacja wirtualnych pinów bramy na starcie
    lastBramaUp = digitalRead(BRAMA_PIN_UP);
    lastBramaDown = digitalRead(BRAMA_PIN_DOWN);

    bootReady = true;
    Serial.println("-> Piny zsynchronizowane.");
    return;
  }

  // 1. OBSŁUGA BRAMY GARAŻOWEJ SOMFY Z SUPLA CLOUD (Przechwytywanie zmian na pinach 17 i 18)
  bool currentBramaUp = digitalRead(BRAMA_PIN_UP);
  bool currentBramaDown = digitalRead(BRAMA_PIN_DOWN);

  if (currentBramaUp && !lastBramaUp) {
      uint16_t rCode = getAndIncrementRollingCode();
      Serial.printf("[SUPLA BRAMA] Zdarzenie -> GÓRA | Code: %d\n", rCode);
      transmitSomfy(CMD_UP, rCode);
  }
  else if (currentBramaDown && !lastBramaDown) {
      uint16_t rCode = getAndIncrementRollingCode();
      Serial.printf("[SUPLA BRAMA] Zdarzenie -> DÓŁ | Code: %d\n", rCode);
      transmitSomfy(CMD_DOWN, rCode);
  }
  else if ((!currentBramaUp && lastBramaUp) || (!currentBramaDown && lastBramaDown)) {
      // Wyślij STOP (CMD_MY) tylko wtedy, gdy OBA wirtualne piny spadły do zera.
      // Zapobiega to wysłaniu STOP przy bezpośredniej zmianie kierunku GÓRA <-> DÓŁ.
      if (!currentBramaUp && !currentBramaDown) {
          uint16_t rCode = getAndIncrementRollingCode();
          Serial.printf("[SUPLA BRAMA] Zdarzenie -> STOP | Code: %d\n", rCode);
          transmitSomfy(CMD_MY, rCode);
      }
  }
  lastBramaUp = currentBramaUp;
  lastBramaDown = currentBramaDown;


  // 2. OBSŁUGA 10 TRADYCYJNYCH ROLET ALUPROF
  for (int i = 0; i < 10; i++) {
    bool currentUp = digitalRead(rolety[i].pinUp);
    bool currentDown = digitalRead(rolety[i].pinDown);

    if (currentUp && !rolety[i].lastUp) {
        Serial.printf("Supla -> %s w GORE\n", rolety[i].name);
        nadajPaczkeAluprof(i, CMD_GORA);
    }
    else if (currentDown && !rolety[i].lastDown) {
        Serial.printf("Supla -> %s w DOL\n", rolety[i].name);
        nadajPaczkeAluprof(i, CMD_DOL);
    }
    else if ((!currentUp && rolety[i].lastUp) || (!currentDown && rolety[i].lastDown)) {
        Serial.printf("Supla -> %s STOP\n", rolety[i].name);
        nadajPaczkeAluprof(i, CMD_STOP);
    }

    rolety[i].lastUp = currentUp;
    rolety[i].lastDown = currentDown;
  }

  // 3. OBSŁUGA PORTU SZEREGOWEGO
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input != '\n' && input != '\r') {
      uint16_t currentCounter = 0;
      switch (input) {
        case 'p': case 'P':
          currentCounter = getAndIncrementRollingCode();
          Serial.printf("[SERIAL] Wysyłam PROG | Code: %d\n", currentCounter);
          transmitSomfy(CMD_PROG, currentCounter);
          break;
        case 'u': case 'U':
          currentCounter = getAndIncrementRollingCode();
          Serial.printf("[SERIAL] Wysyłam UP | Code: %d\n", currentCounter);
          transmitSomfy(CMD_UP, currentCounter);
          break;
        case 'd': case 'D':
          currentCounter = getAndIncrementRollingCode();
          Serial.printf("[SERIAL] Wysyłam DOWN | Code: %d\n", currentCounter);
          transmitSomfy(CMD_DOWN, currentCounter);
          break;
        case 's': case 'S':
          currentCounter = getAndIncrementRollingCode();
          Serial.printf("[SERIAL] Wysyłam STOP | Code: %d\n", currentCounter);
          transmitSomfy(CMD_MY, currentCounter);
          break;
      }
    }
  }
}