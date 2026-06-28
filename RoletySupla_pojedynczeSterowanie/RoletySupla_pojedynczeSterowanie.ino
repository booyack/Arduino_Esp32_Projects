#include <WiFi.h>
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/roller_shutter.h>
#include "ELECHOUSE_CC1101_SRC_DRV.h"

// Fizyczne piny CC1101 na Twoim SuperMini
#define CC1101_SCK    4
#define CC1101_MISO   5
#define CC1101_MOSI   6
#define CC1101_CSN    7
#define CC1101_GDO0   3

// Dane sieci Wi-Fi i Supla Cloud
const char* SSID_WIFI = "WypadZBaru";
const char* PASS_WIFI = "Multiwitamina33";

const char* SUPLA_SERVER = "svr160.supla.org";
const char* SUPLA_EMAIL  = "michalbujak1987@gmail.com";

// UNIKALNE KLUCZE (Zmieniony ostatni bajt na 0x47, żeby wymusić czystą rejestrację nowych nazw)
const char GUID[16]    = {0x2D, 0xA1, 0xC5, 0x11, 0x06, 0x3D, 0x06, 0x44, 0xEE, 0x55, 0xFF, 0x66, 0x11, 0x22, 0x33, 0x47};
const char AUTHKEY[16] = {0x44, 0x33, 0x22, 0x11, 0x66, 0xFF, 0x55, 0xEE, 0x44, 0xDD, 0x33, 0xCC, 0x22, 0xBB, 0x11, 0xAA};

const uint8_t CMD_GORA = 0x11;
const uint8_t CMD_DOL  = 0x33;
const uint8_t CMD_STOP = 0x55;

Supla::ESPWifi wifi(SSID_WIFI, PASS_WIFI);

struct RollerPins {
  uint8_t pinUp;
  uint8_t pinDown;
  const char* name;
  bool lastUp;
  bool lastDown;
};

// Nowa, w 100% bezpieczna pula pinów (omijamy CC1101, LED[8] oraz USB[19,20])
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

void nadajBit(bool bit) {
  if (bit) {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(720);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(360);
  } else {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(360);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(720);
  }
}

void nadajPaczkeAluprof(uint8_t kanal, uint8_t komenda) {
  uint8_t ramka[5] = {0x2A, 0x3D, 0x06, (uint8_t)(0x40 | (kanal & 0x0F)), komenda};
  ELECHOUSE_cc1101.SetTx();
  pinMode(CC1101_GDO0, OUTPUT);

  for (int powtorzenie = 0; powtorzenie < 6; powtorzenie++) {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(4800);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(1600);
    for (int bajtIdx = 0; bajtIdx < 5; bajtIdx++) {
      for (int bitIdx = 7; bitIdx >= 0; bitIdx--) {
        nadajBit((ramka[bajtIdx] >> bitIdx) & 0x01);
      }
    }
    digitalWrite(CC1101_GDO0, LOW); delay(10);
  }
  pinMode(CC1101_GDO0, INPUT);
  ELECHOUSE_cc1101.SetRx();
}

void setup() {
  Serial.begin(115200);

  // CC1101 Init
  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
  ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setMHZ(433.92);
  ELECHOUSE_cc1101.SpiWriteReg(0x02, 0x0D); 
  ELECHOUSE_cc1101.SpiWriteReg(0x08, 0x30); 
  ELECHOUSE_cc1101.SpiWriteReg(0x1B, 0x03); 
  ELECHOUSE_cc1101.SpiWriteReg(0x21, 0x11); 
  ELECHOUSE_cc1101.SetRx();
  pinMode(CC1101_GDO0, INPUT);

  // Rejestracja rolet
  for (int i = 0; i < 10; i++) {
    auto shutter = new Supla::Control::RollerShutter(rolety[i].pinUp, rolety[i].pinDown);
    shutter->setInitialCaption(rolety[i].name); 
  }

  SuplaDevice.setName("Rolety Dom");
  SuplaDevice.begin(GUID, SUPLA_SERVER, SUPLA_EMAIL, AUTHKEY);
}

void loop() {
  SuplaDevice.iterate();

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
}