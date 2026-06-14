/*
 * Project: IskakINO_LiquidCrystal_I2C
 * Folder: examples/05_ProgressBar
 * Description: Dynamic loading progress bar using custom characters.
 * Author: Iskak Fatoni
 */

#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

// Definisi karakter custom untuk batang progress (full block)
uint8_t fullBlock[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

void setup() {
  lcd.begin();
  lcd.backlight();

  // Simpan blok penuh ke memori CGRAM index 0
  lcd.createChar(0, fullBlock);

  lcd.setCursor(0, 0);
  lcd.print("System Loading:");
}

void loop() {
  // Reset baris kedua
  lcd.setCursor(0, 1);
  lcd.print("                "); // 16 spasi untuk clear baris

  // Animasi Progress Bar
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.write(0); // Menampilkan karakter custom index 0
    
    // Tampilkan persentase di baris pertama
    int percent = (i + 1) * 100 / 16;
    lcd.setCursor(12, 0);
    if(percent < 100) lcd.print(" "); // Padding spasi
    lcd.print(percent);
    lcd.print("%");
    
    delay(300); // Kecepatan loading
  }

  delay(2000); // Berhenti sejenak sebelum mengulang
}
