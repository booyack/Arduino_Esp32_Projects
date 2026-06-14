/*
 * Project: IskakINO_LiquidCrystal_I2C
 * Folder: examples/06_I2CScannerMode
 * Description: Using the built-in scanner to identify I2C devices on the bus.
 * Author: Iskak Fatoni
 */

#include <IskakINO_LiquidCrystal_I2C.h>

// Inisialisasi LCD
LiquidCrystal_I2C lcd(16, 2);

void setup() {
  // 1. Jalankan begin(). Library akan otomatis scan alamat LCD
  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("I2C Scanner");
  lcd.setCursor(0, 1);
  lcd.print("Scanning...");
  delay(2000);
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Address:");

  // 2. Ambil alamat yang berhasil ditemukan oleh library
  uint8_t addr = lcd.getAddress();

  lcd.setCursor(0, 1);
  if (addr == 0) {
    lcd.print("Not Found!");
  } else {
    lcd.print("0x");
    lcd.print(addr, HEX);
    lcd.print(" (Active)");
  }

  // Berhenti di sini, tidak perlu scan berulang kali
  while(true) {
    delay(1000);
  }
}
