#include "ELECHOUSE_CC1101_SRC_DRV.h"

#define CC1101_SCK   4
#define CC1101_MISO  5
#define CC1101_MOSI  6
#define CC1101_CSN   7
#define CC1101_GDO0  3

const int LICZBA_PROBEK = 2000;
uint8_t buforSygnalu[LICZBA_PROBEK];

String binNaHex(String bin) {
  String hex = "";
  for (int i = 0; i < bin.length(); i += 4) {
    String czworka = bin.substring(i, i + 4);
    int wartosc = 0;
    for (int j = 0; j < czworka.length(); j++) {
      if (czworka[j] == '1') wartosc |= (1 << (czworka.length() - 1 - j));
    }
    if (wartosc < 10) hex += String(wartosc);
    else hex += String((char)('A' + wartosc - 10));
  }
  return hex;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n=== SUROWY ANALIZATOR HEX ALUPROF ===");

  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
  ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setModulation(2); // ASK/OOK
  ELECHOUSE_cc1101.setMHZ(433.92);

  ELECHOUSE_cc1101.SpiWriteReg(0x02, 0x0D); 
  ELECHOUSE_cc1101.SpiWriteReg(0x08, 0x30); 
  ELECHOUSE_cc1101.SpiWriteReg(0x1B, 0x03); 
  ELECHOUSE_cc1101.SpiWriteReg(0x21, 0x11); 

  ELECHOUSE_cc1101.SetRx();
  pinMode(CC1101_GDO0, INPUT);
  Serial.println("Skaner aktywny. Czekam na kliknięcia...");
}

void loop() {
  if (digitalRead(CC1101_GDO0) == HIGH) {
    unsigned long startH = micros();
    while (digitalRead(CC1101_GDO0) == HIGH) {
      if (micros() - startH > 3500) {
        
        unsigned long nastepnaProbka = micros();
        for (int i = 0; i < LICZBA_PROBEK; i++) {
          while (micros() < nastepnaProbka);
          buforSygnalu[i] = digitalRead(CC1101_GDO0);
          nastepnaProbka += 25;
        }

        int punktStartuDanych = -1;
        for (int i = 10; i < 400; i++) {
          if (buforSygnalu[i] == LOW) {
            int lZero = 0;
            while (i + lZero < LICZBA_PROBEK && buforSygnalu[i + lZero] == LOW) lZero++;
            if (lZero > 25 && (i + lZero) < LICZBA_PROBEK && buforSygnalu[i + lZero] == HIGH) {
              punktStartuDanych = i + lZero;
              break;
            }
            i += lZero;
          }
        }

        if (punktStartuDanych != -1) {
          String binarnaRamka = "";
          int indeksOkna = punktStartuDanych;
          bool status = true;

          for (int b = 0; b < 40; b++) {
            if (indeksOkna + 44 > LICZBA_PROBEK) { status = false; break; }
            int sHigh = 0;
            for (int s = 0; s < 44; s++) {
              if (buforSygnalu[indeksOkna + s] == HIGH) sHigh++;
            }
            if (sHigh > 21) binarnaRamka += "1";
            else binarnaRamka += "0";

            int nKrawedz = indeksOkna + 44;
            for (int j = indeksOkna + 32; j < indeksOkna + 56; j++) {
              if (j < LICZBA_PROBEK - 1 && buforSygnalu[j] == LOW && buforSygnalu[j+1] == HIGH) {
                nKrawedz = j + 1;
                break;
              }
            }
            indeksOkna = nKrawedz;
          }

          if (status && binarnaRamka.length() == 40) {
            String hexStr = binNaHex(binarnaRamka);
            
            // Formatowanie: rozbicie 40 bitów na 5 czystych bajtów HEX
            Serial.print("SUROWY LOG HEX: ");
            for (int i = 0; i < 10; i += 2) {
              Serial.print(hexStr.substring(i, i + 2));
              Serial.print(" ");
            }
            Serial.println();
          }
        }
        delay(400);
        return;
      }
    }
  }
}