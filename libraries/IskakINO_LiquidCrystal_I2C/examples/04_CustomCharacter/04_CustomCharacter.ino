/*
 * Project: IskakINO_LiquidCrystal_I2C
 * Folder: examples/04_CustomCharacter
 * Description: Creating and displaying custom 5x8 pixel characters.
 * Author: Iskak Fatoni
 */

#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

// Definisi pola pixel (5 kolom x 8 baris)
// 1 = Pixel Nyala, 0 = Pixel Mati
uint8_t Heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

uint8_t Bell[8] = {
  0b00100,
  0b01110,
  0b01110,
  0b01110,
  0b11111,
  0b00000,
  0b00100,
  0b00000
};

void setup() {
  lcd.begin();
  lcd.backlight();

  // Mendaftarkan karakter ke memori LCD (Tersedia index 0-7)
  lcd.createChar(0, Heart);
  lcd.createChar(1, Bell);

  lcd.setCursor(0, 0);
  lcd.print("Custom Char:");
  
  // Menampilkan karakter menggunakan fungsi write()
  lcd.setCursor(0, 1);
  lcd.print("Love ");
  lcd.write(0); // Menampilkan Heart
  
  lcd.setCursor(8, 1);
  lcd.print("Bell ");
  lcd.write(1); // Menampilkan Bell
}

void loop() {
  // Efek kedip pada icon heart
  lcd.setCursor(5, 1);
  lcd.print(" ");
  delay(500);
  lcd.setCursor(5, 1);
  lcd.write(0);
  delay(500);
}
