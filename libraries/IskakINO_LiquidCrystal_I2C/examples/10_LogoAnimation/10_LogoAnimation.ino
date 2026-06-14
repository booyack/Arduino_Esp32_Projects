/*
 * =========================================================
 * Project: IskakINO_LiquidCrystal_I2C
 * File: 10_LogoAnimation.ino
 * Deskripsi: Membuat animasi Splash Screen (Logo) saat awal dinyalakan.
 * Author: Iskak Fatoni
 * =========================================================
 */

#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

// Membuat blok khusus untuk logo tebal
uint8_t bar[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
uint8_t top[8] = {0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};
uint8_t bot[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};

void setup() {
  lcd.begin();
  lcd.backlight();

  // Daftarkan karakter custom
  lcd.createChar(0, bar);
  lcd.createChar(1, top);
  lcd.createChar(2, bot);

  // --- ANIMASI MULAI ---
  
  // 1. Munculkan teks satu per satu (Typewriter effect)
  String msg = "BOOTING ISKAK...";
  for (int i = 0; i < msg.length(); i++) {
    lcd.setCursor(i, 0);
    lcd.print(msg[i]);
    delay(100);
  }

  delay(500);
  lcd.clear();

  // 2. Tampilkan Logo "I" Besar (Menggunakan Custom Char)
  // Baris Atas
  lcd.setCursor(7, 0); lcd.write(1); // Top bar
  lcd.setCursor(7, 1); lcd.write(2); // Bot bar
  
  // Efek kedip logo
  for(int i=0; i<3; i++) {
    lcd.backlight();
    delay(200);
    lcd.noBacklight();
    delay(200);
  }
  lcd.backlight();

  // 3. Tampilkan tulisan utama dengan animasi geser
  lcd.clear();
  lcd.setCursor(16, 0);
  lcd.print(" ISKAK INO LIB ");
  lcd.setCursor(16, 1);
  lcd.print("  V1.0.0 READY ");

  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayLeft();
    delay(50);
  }

  delay(2000);
  lcd.clear();
}

void loop() {
  // Masuk ke menu utama setelah animasi selesai
  lcd.setCursor(0, 0);
  lcd.print("System Running..");
  lcd.setCursor(0, 1);
  lcd.print("Mode: Standby   ");
}
