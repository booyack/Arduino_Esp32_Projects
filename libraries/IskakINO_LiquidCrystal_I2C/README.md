# IskakINO_LiquidCrystal_I2C

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/)
[![Platform](https://img.shields.io/badge/platform-Arduino%20%7C%20ESP32-green)](#)
[![License](https://img.shields.io/badge/license-MIT-lightgrey)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0.0-orange)](#)
[![CI](https://github.com/iskakfatoni/IskakINO_LiquidCrystal_I2C/actions/workflows/arduino-ci.yml/badge.svg)](https://github.com/iskakfatoni/IskakINO_LiquidCrystal_I2C/actions)

LiquidCrystal I2C library for **HD44780 LCD** using **PCF8574 I2C expander**  
with **automatic I2C address detection**.

Designed to be:
- ✅ Backward compatible with `LiquidCrystal_I2C`
- ⚡ Lightweight & optimized for flash/RAM
- 🔌 Arduino & ESP32 friendly
- 🧼 Clean API & maintainer-grade structure
---
## 📑 Table of Contents
- [Features](#-features)
- [Supported Hardware](#-supported-hardware)
- [Installation](#-installation)
- [Wiring](#-wiring)
- [Basic Usage](#-basic-usage)
- [Showcase Examples](#-showcase-examples)
- [API Compatibility](#-api-compatibility)
- [Versioning](#-versioning)
- [License](#-license)

---
## ✨ Features
- Automatic I2C address scanning (0x20–0x27)
- Full **HD44780 command support**
- Standard `LiquidCrystal` compatible API
- Custom character (CGRAM) support
- Scroll, autoscroll, cursor & blink control
- Non-blocking friendly (millis-based design)
- Optimized for ESP32 & Arduino boards

---
## 🧩 Supported Hardware
This library supports HD44780 LCDs with I2C backpack (PCF8574 / PCF8574A) and popular microcontrollers:
| Board / MCU | Notes |
|-------------|-------|
| Arduino UNO / Nano | 5V logic, SDA/A4, SCL/A5 |
| Arduino Mega | 5V logic, SDA/20, SCL/21 |
| ESP32 | 3.3V logic, SDA = GPIO21, SCL = GPIO22 |
| ESP32-C3 | 3.3V logic, SDA = GPIO20, SCL = GPIO21 |
| ESP8266 (NodeMCU / Wemos D1) | 3.3V logic, SDA = D2 (GPIO4), SCL = D1 (GPIO5) |

---
## 📦 Installation
### Arduino IDE (Library Manager)
1. Open **Arduino IDE**
2. Sketch → Include Library → **Manage Libraries**
3. Search for **IskakINO_LiquidCrystal_I2C**
4. Click **Install**
### Manual Installation (ZIP)
1. Download this repository as ZIP
2. Arduino IDE → Sketch → Include Library → **Add .ZIP Library**

---
## 🔌 Wiring
Connect your HD44780 LCD with I2C backpack to your board:
| LCD I2C Pin | Arduino | ESP32 | ESP32-C3 | ESP8266 |
|------------|----------|--------|-----------|----------|
| SDA | A4 | GPIO21 | GPIO20 | D2 (GPIO4) |
| SCL | A5 | GPIO22 | GPIO21 | D1 (GPIO5) |
| VCC | 5V | 3.3V | 3.3V | 3.3V |
| GND | GND | GND | GND | GND |
> ⚠ Ensure voltage compatibility between your LCD backpack and board.

---
## 🚀 Basic Usage
Cukup gunakan kode sederhana ini untuk memulai. Library akan secara otomatis mencari alamat I2C LCD Anda (0x27 atau 0x3F), sehingga Anda tidak perlu mengaturnya secara manual.

```cpp
#include <IskakINO_LiquidCrystal_I2C.h>

// Inisialisasi: (Jumlah Kolom, Jumlah Baris)
// Alamat I2C akan dideteksi secara otomatis!
LiquidCrystal_I2C lcd(16, 2);

void setup() {
  // Memulai komunikasi I2C dan LCD
  lcd.begin();
  
  // Menyalakan lampu latar
  lcd.backlight();

  // Menampilkan pesan di baris pertama
  lcd.setCursor(0, 0);
  lcd.print("Hello IskakINO");
  
  // Menampilkan pesan di baris kedua
  lcd.setCursor(0, 1);
  lcd.print("Auto-Scan Ready");
}

void loop() {
  // Kode utama Anda di sini
}
```

---
## 📂 Showcase Examples
Library ini menyediakan pembelajaran bertahap dari tingkat dasar hingga mahir. Anda dapat menemukan file-file ini di folder `examples/`.
### Daftar Contoh Berdasarkan Level

| No | Folder Contoh | Deskripsi | Level |
|:---:|:---|:---|:---:|
| 01 | **[01_HelloWorld](./examples/01_HelloWorld)** | Inisialisasi LCD dan fitur Auto-Scan alamat I2C secara otomatis. | ⭐ Basic |
| 02 | **[02_BlinkAndCursor](./examples/02_BlinkAndCursor)** | Mengontrol visual kursor (garis bawah) dan efek kedipan (blink). | ⭐ Basic |
| 03 | **[03_ScrollAndBacklight](./examples/03_ScrollAndBacklight)** | Animasi teks berjalan ke samping dan kontrol daya lampu latar. | ⭐ Basic |
| 04 | **[04_CustomCharacter](./examples/04_CustomCharacter)** | Cara menggambar simbol buatan sendiri (ikon hati, bel, dll) di memori LCD. | ⭐⭐ Interm. |
| 05 | **[05_ProgressBar](./examples/05_ProgressBar)** | Membuat animasi loading bar yang dinamis dan perhitungan persentase. | ⭐⭐ Interm. |
| 06 | **[06_I2CScannerMode](./examples/06_I2CScannerMode)** | Menggunakan fungsi internal library untuk mendeteksi perangkat I2C lain. | ⭐⭐ Interm. |
| 07 | **[07_DigitalClock](./examples/07_DigitalClock)** | Membuat jam digital akurat tanpa `delay()` menggunakan logika `millis()`. | ⭐⭐⭐ Advance |
| 08 | **[08_MenuNavigation](./examples/08_MenuNavigation)** | Struktur menu multi-halaman untuk navigasi antarmuka pengguna (UI). | ⭐⭐⭐ Advance |
| 09 | **[09_CustomI2CPins_ESP32](./examples/09_CustomI2CPins_ESP32)** | Konfigurasi pin SDA & SCL kustom untuk fleksibilitas pada ESP32/ESP8266. | ⭐⭐⭐ Advance |
| 10 | **[10_LogoAnimation](./examples/10_LogoAnimation)** | Membuat animasi Splash Screen (Booting) menggunakan gabungan teknik kustom. | ⭐⭐⭐ Advance |

---
### 🚀 Cara Menjalankan Contoh

1. **Instalasi**: Pastikan folder library `IskakINO_LiquidCrystal_I2C` sudah berada di dalam folder `libraries` Arduino Anda.
2. **Buka IDE**: Jalankan Arduino IDE.
3. **Pilih Contoh**: Klik menu **File** > **Examples** > **IskakINO_LiquidCrystal_I2C**.
4. **Upload**: Pilih contoh (mulai dari `01_HelloWorld`), lalu klik tombol **Upload**.

> **Catatan Hardware:**
> - Untuk **Arduino Uno**, sambungkan SDA ke `A4` dan SCL ke `A5`.
> - Untuk **ESP32**, pin default adalah `GPIO 21 (SDA)` dan `GPIO 22 (SCL)`.
> - Pastikan Jumper I2C pada modul belakang LCD terpasang agar lampu latar (backlight) menyala.

---
## 🔁 API Compatibility

Library ini dirancang agar **100% kompatibel** dengan pola penggunaan `LiquidCrystal_I2C` yang umum digunakan di komunitas Arduino. Anda tidak perlu mengubah logika kode lama Anda.

### 1. Auto-Detect Mode (Rekomendasi)
Cukup masukkan ukuran layar. Library akan melakukan scanning otomatis pada jalur I2C untuk menemukan alamat LCD Anda.
```cpp
// Format: (Kolom, Baris)
LiquidCrystal_I2C lcd(16, 2);
```
2. Standard Mode (Manual Address)
Jika Anda ingin menentukan alamat I2C secara spesifik (misalnya jika Anda memiliki lebih dari satu LCD), library tetap mendukung cara lama.
```cpp
// Format: (Alamat_I2C, Kolom, Baris)
LiquidCrystal_I2C lcd(0x27, 16, 2);
```
3. Kompatibilitas Fungsi Utama
Semua fungsi standar tetap tersedia dan berjalan normal:
```cpp
lcd.init() atau lcd.begin()
lcd.backlight() / lcd.noBacklight()
lcd.createChar()
lcd.scrollDisplayLeft() / lcd.scrollDisplayRight()
```
Catatan: Panggilan API lama (Legacy) tetap berfungsi penuh tanpa memerlukan modifikasi pada sketsa Anda.

---
## 🏷 Versi (Versioning)

Library ini mengikuti standar **Semantic Versioning**:
`MAJOR.MINOR.PATCH`

📌 **Versi Saat Ini: v1.0.0**

* **Rilis Stabil Awal**: Versi pertama yang siap digunakan untuk produksi.
* **Kompatibilitas Penuh**: Mendukung API standar `LiquidCrystal_I2C` (Backward Compatible).
* **Deteksi Otomatis**: Fitur pemindaian alamat I2C tanpa input manual.
* **Dukungan Karakter Kustom**: Membuat simbol dan ikon buatan sendiri.
* **Kontrol Penuh**: Pengaturan teks berjalan (*scroll*), kursor, dan kedipan (*blink*).
* **Contoh Lengkap**: Dilengkapi dengan contoh *progress bar* dan jam digital.
* **Komitmen Stabilitas**: Tidak akan ada perubahan drastis (*breaking changes*) pada seri versi 1.x.

---
### 📌 Current Version
&nbsp;&nbsp;&nbsp;&nbsp;**v1.0.0**
&nbsp;&nbsp;&nbsp;&nbsp;- Initial stable release  
&nbsp;&nbsp;&nbsp;&nbsp;- Backward compatible API  
&nbsp;&nbsp;&nbsp;&nbsp;- Automatic I2C address detection  
&nbsp;&nbsp;&nbsp;&nbsp;- Custom character support  
&nbsp;&nbsp;&nbsp;&nbsp;- Scroll, cursor & blink control  
&nbsp;&nbsp;&nbsp;&nbsp;- Advanced dashboard & progress bar examples  
&nbsp;&nbsp;&nbsp;&nbsp;- No breaking changes will be introduced within the 1.x series.

---
## 🤝 Kontribusi (Contributing)
Kontribusi selalu terbuka bagi siapa saja! Jika Anda menemukan bug atau memiliki ide fitur baru, silakan:
1. **Fork** repository ini.
2. Buat branch baru (`git checkout -b fitur-baru`).
3. Lakukan **Commit** perubahan Anda (`git commit -m 'Menambah fitur X'`).
4. **Push** ke branch tersebut (`git push origin fitur-baru`).
5. Buat **Pull Request**.
Kami sangat menghargai masukan untuk pengembangan library **IskakINO** agar lebih baik lagi bagi komunitas Maker di Indonesia.

---
## 📄 Lisensi (License)
Library ini didistribusikan di bawah **Lisensi MIT**. Anda bebas menggunakan, memodifikasi, dan membagikan library ini baik untuk proyek hobi maupun komersial selama tetap menyertakan atribusi penulis asli.
Copyright (c) 2026 **Iskak Fatoni**

---
## ☕ Dukungan
Jika library ini membantu proyek Anda, jangan ragu untuk memberikan **Star ⭐** pada repository ini atau bagikan ke teman-teman sesama pengembang. Dukungan Anda sangat berarti untuk pengembangan library gratis lainnya di masa depan!
**Happy Coding! 🚀**
