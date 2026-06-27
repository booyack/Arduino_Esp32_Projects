#include <WiFi.h>
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/roller_shutter.h>
#include "ELECHOUSE_CC1101_SRC_DRV.h"

constexpr uint8_t CC1101_SCK = 4;
#define CC1101_MISO  5
#define CC1101_MOSI  6
#define CC1101_CSN   7
#define CC1101_GDO0  3

const char* SSID_WIFI = "WypadZBaru";
const char* PASS_WIFI = "Multiwitamina33";
const char* SUPLA_SERVER = "svr160.supla.org";
const char* SUPLA_EMAIL  = "michalbujak1987@gmail.com";

const char GUID[16]    = {0x2D, 0xA1, 0xC5, 0x11, 0x06, 0x3D, 0x06, 0x44, 0xEE, 0x55, 0xFF, 0x66, 0x11, 0x22, 0x33, 0x44};
const char AUTHKEY[16] = {0x44, 0x33, 0x22, 0x11, 0x66, 0xFF, 0x55, 0xEE, 0x44, 0xDD, 0x33, 0xCC, 0x22, 0xBB, 0x11, 0xAA};

const uint8_t CMD_GORA = 0x11;
const uint8_t CMD_DOL  = 0x33;
const uint8_t CMD_STOP = 0x55;

Supla::ESPWifi wifi(SSID_WIFI, PASS_WIFI);

// --- Funkcje radiowe Aluprof ---
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

// --- Dedykowana klasa wirtualnej rolety dla Supli ---
class AluprofRollerShutter : public Supla::Control::RollerShutter {
  public:
    AluprofRollerShutter(uint8_t aluprofChannel) 
      : Supla::Control::RollerShutter(-1, -1), channel(aluprofChannel) {}

    void handleAction(int event, int action) override {
      // Wywołujemy oryginalną logikę z biblioteki
      Supla::Control::RollerShutter::handleAction(event, action);

      // Mapowanie surowych ID akcji (zamiast niedostępnych Supla::OPEN / Supla::START_OPENING)
      // W bibliotece Supla: 1 = GÓRA/OTWÓRZ, 2 = DÓŁ/ZAMKNIJ, 3 = STOP
      if (action == 1) {
        Serial.printf("Kanal %d -> Ruch w GORE\n", channel);
        nadajPaczkeAluprof(channel, CMD_GORA);
      } 
      else if (action == 2) {
        Serial.printf("Kanal %d -> Ruch w DOL\n", channel);
        nadajPaczkeAluprof(channel, CMD_DOL);
      } 
      else if (action == 3) {
        Serial.printf("Kanal %d -> STOP\n", channel);
        nadajPaczkeAluprof(channel, CMD_STOP);
      }
    }

  private:
    uint8_t channel;
};

void setup() {
  Serial.begin(115200);

  // CC1101 Init
  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
  ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setMHZ(433.92);

  // preambula
  ELECHOUSE_cc1101.SpiWriteReg(0x02, 0x0D); 
  ELECHOUSE_cc1101.SpiWriteReg(0x08, 0x30); 
  ELECHOUSE_cc1101.SpiWriteReg(0x1B, 0x03); 
  ELECHOUSE_cc1101.SpiWriteReg(0x21, 0x11); 
  ELECHOUSE_cc1101.SetRx();
  pinMode(CC1101_GDO0, INPUT);

  // --- REJESTRACJA KANAŁÓW W SUPLI ---
  auto r0 = new AluprofRollerShutter(0); // Kanał 0 (Wszystkie rolety)
  auto r1 = new AluprofRollerShutter(1); // Kanał 1 (Taras)
  auto r2 = new AluprofRollerShutter(2); // Kanał 2 (KuchniaZachod)
  auto r3 = new AluprofRollerShutter(3); // Kanał 3 (KuchniaPolnoc)
  auto r4 = new AluprofRollerShutter(4); // Kanał 4 (Garaz)
  auto r5 = new AluprofRollerShutter(5); // Kanał 5 (Kotlownia)
  auto r6 = new AluprofRollerShutter(6); // Kanał 6 (Madzia)
  auto r7 = new AluprofRollerShutter(7); // Kanał 7 (Dodek)
  auto r8 = new AluprofRollerShutter(8); // Kanał 8 (Polcia)
  auto r9 = new AluprofRollerShutter(9); // Kanał 9 (Sypialnia)
  
  SuplaDevice.setName("Rolety Dom");
  SuplaDevice.begin(GUID, SUPLA_SERVER, SUPLA_EMAIL, AUTHKEY);
}

void loop() {
  SuplaDevice.iterate();
}