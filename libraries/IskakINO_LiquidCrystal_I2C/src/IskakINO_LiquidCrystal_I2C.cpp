#include "IskakINO_LiquidCrystal_I2C.h"
#include <Arduino.h>
#include <Wire.h>

/* =========================================================
   Constructor
   ========================================================= */
LiquidCrystal_I2C::LiquidCrystal_I2C(uint8_t cols, uint8_t rows)
    : _cols(cols),
      _rows(rows),
      _addr(0x00),
      _backlight(true),
      _initialized(false), 
      _displayfunction(LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS),
      _displaycontrol(0),
      _displaymode(0)
{
}

/* =========================================================
   Public API
   ========================================================= */

void LiquidCrystal_I2C::setAddress(uint8_t addr) {
    _addr = addr;
#if LCD_ENABLE_SERIAL_DEBUG
    Serial.print(F("[LCD] Manual I2C address set: 0x"));
    Serial.println(_addr, HEX);
#endif
}

uint8_t LiquidCrystal_I2C::getAddress() const {
    return _addr;
}

void LiquidCrystal_I2C::createChar(uint8_t location, const uint8_t charmap[]) {
    location &= 0x7;
    _command(LCD_SETCGRAMADDR | (location << 3));
    for (uint8_t i = 0; i < 8; i++) {
        write(charmap[i]);
    }
}

void LiquidCrystal_I2C::begin() {
    if (_initialized) return;
    Wire.begin();
    Wire.setClock(100000);   // Stabil untuk LCD I2C

    _scanAddress();

    if (_addr == 0x00) return;   // Tidak ditemukan

    if (_rows > 1) {
        _displayfunction |= LCD_2LINE;
    }

    delay(50);

    _write4bits(0x03 << 4);
    delayMicroseconds(4500);
    _write4bits(0x03 << 4);
    delayMicroseconds(4500);
    _write4bits(0x03 << 4);
    delayMicroseconds(150);
    _write4bits(0x02 << 4);

    _command(LCD_FUNCTIONSET | _displayfunction);

    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    clear();

    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    _command(LCD_ENTRYMODESET | _displaymode);

    home();
    _initialized = true;
}

void LiquidCrystal_I2C::clear() {
    _command(LCD_CLEARDISPLAY);
    delayMicroseconds(2000);
}

void LiquidCrystal_I2C::home() {
    _command(LCD_RETURNHOME);
    delayMicroseconds(2000);
}

void LiquidCrystal_I2C::setCursor(uint8_t col, uint8_t row) {
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= _rows) row = _rows - 1;
    _command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LiquidCrystal_I2C::display() {
    _displaycontrol |= LCD_DISPLAYON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::cursor() {
    _displaycontrol |= LCD_CURSORON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::noCursor() {
    _displaycontrol &= ~LCD_CURSORON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::blink() {
    _displaycontrol |= LCD_BLINKON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::noBlink() {
    _displaycontrol &= ~LCD_BLINKON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::backlight() {
    _backlight = true;
    _expanderWrite(0);
}

void LiquidCrystal_I2C::noBacklight() {
    _backlight = false;
    _expanderWrite(0);
}

void LiquidCrystal_I2C::scrollDisplayLeft() {
    _command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal_I2C::scrollDisplayRight() {
    _command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LiquidCrystal_I2C::leftToRight() {
    _displaymode |= LCD_ENTRYLEFT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal_I2C::rightToLeft() {
    _displaymode &= ~LCD_ENTRYLEFT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal_I2C::autoscroll() {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal_I2C::noAutoscroll() {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

size_t LiquidCrystal_I2C::write(uint8_t value) {
    _send(value, Rs);
    return 1;
}

/* =========================================================
   Low-level command handling
   ========================================================= */

void LiquidCrystal_I2C::_command(uint8_t value) {
    _send(value, 0);
}

void LiquidCrystal_I2C::_send(uint8_t value, uint8_t mode) {
    if (_addr == 0x00) return;

    uint8_t highnib = value & 0xF0;
    uint8_t lownib  = (value << 4) & 0xF0;

    _write4bits(highnib | mode);
    _write4bits(lownib  | mode);
}

void LiquidCrystal_I2C::_write4bits(uint8_t value) {
    _expanderWrite(value);
    _pulseEnable(value);
}

void LiquidCrystal_I2C::_pulseEnable(uint8_t data) {
    _expanderWrite(data | En);
    delayMicroseconds(1);

    _expanderWrite(data & ~En);
    delayMicroseconds(50);
}

void LiquidCrystal_I2C::_expanderWrite(uint8_t data) {
    if (_addr == 0x00) return;

    Wire.beginTransmission(_addr);
    Wire.write(data | (_backlight ? LCD_BACKLIGHT : LCD_NOBACKLIGHT));
    Wire.endTransmission();
}

/* =========================================================
   I2C Address Auto Scan
   ========================================================= */

void LiquidCrystal_I2C::_scanAddress() {

    if (_addr != 0x00) return; // manual override

    // PCF8574
    for (uint8_t addr = 0x20; addr <= 0x27; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            _addr = addr;
#if LCD_ENABLE_SERIAL_DEBUG
            Serial.print(F("[LCD] Found at 0x"));
            Serial.println(_addr, HEX);
#endif
            return;
        }
    }

    // PCF8574A
    for (uint8_t addr = 0x38; addr <= 0x3F; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            _addr = addr;
#if LCD_ENABLE_SERIAL_DEBUG
            Serial.print(F("[LCD] Found at 0x"));
            Serial.println(_addr, HEX);
#endif
            return;
        }
    }

#if LCD_ENABLE_SERIAL_DEBUG
    Serial.println(F("[LCD] I2C device NOT found"));
#endif
}

/**
 * Menampilkan teks tepat di tengah layar secara otomatis
 */
void LiquidCrystal_I2C::printCenter(String text, int row) {
  int len = text.length();
  int pos = (_cols - len) / 2;
  if (pos < 0) pos = 0; // Jaga-jaga jika teks lebih panjang dari kolom
  setCursor(pos, row);
  print(text);
}

/**
 * Menampilkan teks dengan efek mengetik (per karakter)
 */
void LiquidCrystal_I2C::typewriter(String text, int row, int delayTime) {
  setCursor(0, row);
  for (int i = 0; i < text.length(); i++) {
    if (i < _cols) { // Batasi agar tidak meluap dari jumlah kolom
      print(text[i]);
      delay(delayTime);
    }
  }
}

/**
 * Mengecek apakah LCD masih terhubung di jalur I2C
 */
bool LiquidCrystal_I2C::isConnected() {
  Wire.beginTransmission(_addr); // Benar (a kecil)
  return (Wire.endTransmission() == 0);
}
