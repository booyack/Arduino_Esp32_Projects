/*
 * Project: IskakINO_LiquidCrystal_I2C
 * Folder: examples/02_BlinkAndCursor
 * Description: Demonstrates how to control the cursor and blinking effect.
 * Author: Iskak Fatoni
 */

#include <IskakINO_LiquidCrystal_I2C.h>

// Inisialisasi LCD 16x2
LiquidCrystal_I2C lcd(16, 2);

void setup() {
  lcd.begin();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("IskakINO Cursor");
}

void loop() {
  // 1. Tampilkan Kursor (Garis Bawah)
  lcd.setCursor(0, 1);
  lcd.print("Cursor: ON   ");
  lcd.cursor();         // Mengaktifkan garis bawah
  delay(3000);

  // 2. Tambahkan Efek Kedip (Blink)
  lcd.setCursor(0, 1);
  lcd.print("Blink: ON    ");
  lcd.blink();          // Mengaktifkan kedipan kotak
  delay(3000);

  // 3. Matikan Kursor dan Kedip
  lcd.noBlink();        // Mematikan kedipan
  lcd.noCursor();       // Mematikan garis bawah
  lcd.setCursor(0, 1);
  lcd.print("Cursor: OFF  ");
  delay(3000);
}
