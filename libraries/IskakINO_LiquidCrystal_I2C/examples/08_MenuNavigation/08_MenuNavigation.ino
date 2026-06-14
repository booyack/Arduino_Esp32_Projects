/*
 * =========================================================
 * Project: IskakINO_LiquidCrystal_I2C
 * File: 08_MenuNavigation.ino
 * Deskripsi: Belajar membuat sistem menu halaman pada LCD.
 * Author: Iskak Fatoni
 * =========================================================
 */

#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

// Variabel untuk menyimpan halaman mana yang sedang aktif
int halaman = 1; 
int totalHalaman = 3;

// Variabel waktu agar perpindahan halaman tidak terlalu cepat (Debounce sederhana)
unsigned long waktuTerakhirKlik = 0;
const int jedaKlik = 2000; // Berpindah halaman setiap 2 detik secara otomatis

void setup() {
  lcd.begin();
  lcd.backlight();
}

void loop() {
  unsigned long waktuSekarang = millis();

  // Logika perpindahan halaman otomatis setiap 2 detik
  // (Dalam proyek nyata, ini biasanya diganti dengan menekan tombol)
  if (waktuSekarang - waktuTerakhirKlik >= jedaKlik) {
    waktuTerakhirKlik = waktuSekarang;
    
    halaman++;
    if (halaman > totalHalaman) {
      halaman = 1; // Kembali ke halaman pertama jika sudah di akhir
    }
    
    // Setiap ganti halaman, bersihkan layar agar tidak ada tulisan sisa
    lcd.clear();
  }

  // --- LOGIKA TAMPILAN MENU ---
  // Kita cek halaman berapa yang sedang aktif menggunakan switch-case
  switch (halaman) {
    case 1:
      // Tampilan Halaman Utama
      lcd.setCursor(0, 0);
      lcd.print("> MENU UTAMA   1");
      lcd.setCursor(0, 1);
      lcd.print("Sistem Aktif... ");
      break;

    case 2:
      // Tampilan Halaman Sensor (Contoh)
      lcd.setCursor(0, 0);
      lcd.print("> DATA SENSOR  2");
      lcd.setCursor(0, 1);
      lcd.print("Suhu: 27.5 C    ");
      break;

    case 3:
      // Tampilan Halaman Pengaturan
      lcd.setCursor(0, 0);
      lcd.print("> PENGATURAN   3");
      lcd.setCursor(0, 1);
      lcd.print("Cerah: 100%     ");
      break;
  }
}
