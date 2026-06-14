/*
 * =========================================================
 * Project: IskakINO_LiquidCrystal_I2C
 * File: 07_DigitalClock.ino
 * Deskripsi: Belajar membuat jam digital tanpa membuat Arduino "macet" (Non-Blocking).
 * Author: Iskak Fatoni
 * =========================================================
 */

// Memanggil library IskakINO agar fungsi LCD bisa digunakan
#include <IskakINO_LiquidCrystal_I2C.h>

// Membuat objek LCD dengan ukuran 16 kolom dan 2 baris
// Alamat I2C (seperti 0x27) akan dicari otomatis oleh library Iskak
LiquidCrystal_I2C lcd(16, 2);

// --- VARIABLE UNTUK WAKTU ---
// unsigned long digunakan karena angka milidetik bisa menjadi sangat besar
unsigned long waktuLalu = 0;      
const long intervalSatuDetik = 1000; // 1000 milidetik = 1 detik

// Variabel penyimpan data waktu (bisa diubah sesuai keinginan)
int detik = 0;
int menit = 0;
int jam   = 12; 

void setup() {
  // Memulai koneksi I2C dan menghidupkan layar
  lcd.begin();
  lcd.backlight();
  
  // Tampilkan judul di baris pertama sekali saja
  lcd.setCursor(0, 0);
  lcd.print("JAM DIGITAL:");
}

void loop() {
  // millis() adalah fungsi bawaan Arduino yang menghitung 
  // sudah berapa lama Arduino nyala sejak pertama kali diberi power.
  unsigned long waktuSekarang = millis();

  // LOGIKA NON-BLOCKING:
  // Kita cek: "Apakah selisih waktu sekarang dengan waktu terakhir kita update sudah 1 detik?"
  if (waktuSekarang - waktuLalu >= intervalSatuDetik) {
    
    // Update waktuLalu agar perhitungan berikutnya mulai dari detik ini
    waktuLalu = waktuSekarang;
    
    // 1. Tambah detik
    detik++;
    
    // 2. Jika detik sudah 60, reset ke 0 dan tambah menit
    if (detik >= 60) {
      detik = 0;
      menit++;
    }
    
    // 3. Jika menit sudah 60, reset ke 0 dan tambah jam
    if (menit >= 60) {
      menit = 0;
      jam++;
    }
    
    // 4. Jika jam sudah 24, reset ke 0 (tengah malam)
    if (jam >= 24) {
      jam = 0;
    }

    // --- PROSES MENAMPILKAN KE LCD ---
    // Kita arahkan kursor ke baris kedua, kolom ke-4 agar jam ada di tengah
    lcd.setCursor(4, 1);
    
    // TIPS PEMULA: Padding Nol
    // Jika angka jam kurang dari 10 (misal jam 9), tampilkan "0" dulu baru "9" -> "09"
    if (jam < 10) lcd.print("0");
    lcd.print(jam);
    lcd.print(":"); // Pemisah jam dan menit
    
    if (menit < 10) lcd.print("0");
    lcd.print(menit);
    lcd.print(":"); // Pemisah menit dan detik
    
    if (detik < 10) lcd.print("0");
    lcd.print(detik);
    
    // Karena kita tidak pakai delay(), di dalam loop() ini Anda bisa 
    // menambahkan kode lain (misal baca sensor suhu) tanpa mengganggu jalannya jam!
  }
}
