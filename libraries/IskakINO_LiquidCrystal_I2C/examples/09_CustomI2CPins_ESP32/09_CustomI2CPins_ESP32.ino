/*
 * =========================================================
 * Project: IskakINO_LiquidCrystal_I2C
 * File: 09_CustomI2CPins_ESP32.ino
 * Deskripsi: Cara mengubah pin SDA & SCL secara manual pada ESP32/ESP8266.
 * Author: Iskak Fatoni
 * =========================================================
 */

#include <IskakINO_LiquidCrystal_I2C.h>
#include <Wire.h> // Dibutuhkan untuk mengatur pin Wire secara manual

// Inisialisasi LCD
LiquidCrystal_I2C lcd(16, 2);

// --- PENGATURAN PIN CUSTOM ---
// Sesuaikan angka di bawah ini dengan kabel yang Anda colok ke ESP32
#if defined(ESP32)
  const int PIN_SDA = 18; // Contoh: memindahkan SDA ke GPIO 18
  const int PIN_SCL = 19; // Contoh: memindahkan SCL ke GPIO 19
#elif defined(ESP8266)
  const int PIN_SDA = D1; 
  const int PIN_SCL = D2;
#endif

void setup() {
  // 1. MEMULAI WIRE DENGAN PIN CUSTOM
  // Kita harus memanggil Wire.begin(SDA, SCL) SEBELUM lcd.begin()
  #if defined(ESP32) || defined(ESP8266)
    Wire.begin(PIN_SDA, PIN_SCL);
  #endif

  // 2. MEMULAI LCD
  // Karena Wire sudah dimulai dengan pin custom, library IskakINO 
  // akan otomatis mengikuti jalur tersebut.
  lcd.begin();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("ESP32 Custom Pin");
  lcd.setCursor(0, 1);
  lcd.print("SDA:18  SCL:19");
}

void loop() {
  // Tetap bisa menjalankan fungsi lainnya
}
