/*
 * Project: IskakINO_LiquidCrystal_I2C
 * Folder: examples/03_ScrollAndBacklight
 * Description: Demonstrates hardware text scrolling and backlight toggling.
 * Author: Iskak Fatoni
 */

#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

void setup() {
  lcd.begin();
  lcd.backlight();
  
  // Menampilkan pesan awal
  lcd.setCursor(0, 0);
  lcd.print("IskakINO Scrolling");
  delay(1000);
}

void loop() {
  // 1. Geser teks ke KIRI sebanyak 16 kali
  // Ini berguna untuk menampilkan pesan panjang yang tersembunyi
  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayLeft();
    delay(250);
  }

  // 2. Indikator lampu latar (Backlight Toggle)
  // Mematikan dan menyalakan lampu latar sebagai efek visual
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
  delay(500);

  // 3. Geser teks kembali ke KANAN sebanyak 16 kali
  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayRight();
    delay(250);
  }

  delay(1000);
}
