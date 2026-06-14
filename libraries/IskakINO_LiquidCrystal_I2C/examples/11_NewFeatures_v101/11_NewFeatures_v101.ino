/*
 * =========================================================
 * Project: IskakINO_LiquidCrystal_I2C
 * File: 11_NewFeatures_v101.ino
 * Deskripsi: Demonstrasi fitur baru v1.0.1 (printCenter, typewriter, isConnected)
 * Author: Iskak Fatoni
 * =========================================================
 */

#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

void setup() {
  lcd.begin();
  lcd.backlight();

  // 1. Fitur isConnected() - Cek apakah kabel LCD terpasang dengan benar
  if (lcd.isConnected()) {
    // 2. Fitur typewriter() - Teks muncul perlahan seperti mengetik
    // Format: (Teks, Baris, Kecepatan_Delay)
    lcd.typewriter("IskakINO v1.0.1", 0, 100);
    delay(1000);
    lcd.clear();

    // 3. Fitur printCenter() - Teks otomatis berada di tengah
    // Tidak perlu lagi menghitung spasi manual!
    lcd.printCenter("SISTEM READY", 0);
    lcd.printCenter("SKT-2026", 1);
  } else {
    // Jika LCD tidak terdeteksi (misal kabel lepas)
    Serial.begin(9600);
    Serial.println("Error: LCD tidak terdeteksi!");
  }
}

void loop() {
  // Demo loop: Berkedip di tengah
  delay(2000);
  lcd.clear();
  lcd.printCenter("TERIMA KASIH", 0);
  lcd.printCenter("iskakfatoni", 1);
  delay(2000);
}
