//
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Print.h>

/* =========================================================
   Compile-time configuration
   ========================================================= */
#ifndef LCD_ENABLE_SERIAL_DEBUG
#define LCD_ENABLE_SERIAL_DEBUG 0
#endif

/* =========================================================
   PCF8574 Pin Mapping
   ========================================================= */
#define En 0b00000100
#define Rw 0b00000010
#define Rs 0b00000001

#define LCD_BACKLIGHT     0x08
#define LCD_NOBACKLIGHT   0x00

/* =========================================================
   HD44780 Commands
   ========================================================= */
#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

/* =========================================================
   Entry Mode Flags
   ========================================================= */
#define LCD_ENTRYRIGHT           0x00
#define LCD_ENTRYLEFT            0x02
#define LCD_ENTRYSHIFTINCREMENT  0x01
#define LCD_ENTRYSHIFTDECREMENT  0x00

/* =========================================================
   Display Control Flags
   ========================================================= */
#define LCD_DISPLAYON   0x04
#define LCD_DISPLAYOFF  0x00
#define LCD_CURSORON    0x02
#define LCD_CURSOROFF   0x00
#define LCD_BLINKON     0x01
#define LCD_BLINKOFF    0x00

/* =========================================================
   Function Set Flags
   ========================================================= */
#define LCD_8BITMODE  0x10
#define LCD_4BITMODE  0x00
#define LCD_2LINE     0x08
#define LCD_1LINE     0x00
#define LCD_5x10DOTS  0x04
#define LCD_5x8DOTS   0x00

/* =========================================================
   Cursor / Display Shift Flags
   ========================================================= */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT   0x04
#define LCD_MOVELEFT    0x00

/* =========================================================
   Class Definition
   ========================================================= */
class LiquidCrystal_I2C : public Print {
public:
    LiquidCrystal_I2C(uint8_t cols, uint8_t rows);

    /* Basic control */
    void begin();
    void clear();
    void home();
    void setCursor(uint8_t col, uint8_t row);

    /* Display control */
    void display();
    void noDisplay();
    void cursor();
    void noCursor();
    void blink();
    void noBlink();

    /* Backlight */
    void backlight();
    void noBacklight();

    /* Scrolling */
    void scrollDisplayLeft();
    void scrollDisplayRight();
    void leftToRight();
    void rightToLeft();
    void autoscroll();
    void noAutoscroll();

    /* Address control */
    void setAddress(uint8_t addr);
    uint8_t getAddress() const;

    /* Custom character */
    void createChar(uint8_t location, const uint8_t charmap[]);

    /* Print override */
    virtual size_t write(uint8_t);

   // --- Tambahkan di bawah fungsi print standar ---
   void printCenter(String text, int row);
   void typewriter(String text, int row, int delayTime = 100);
   bool isConnected(); // Fitur cek koneksi hardware


private:
    void _command(uint8_t value);
    void _send(uint8_t value, uint8_t mode);
    void _write4bits(uint8_t value);
    void _pulseEnable(uint8_t data);
    void _expanderWrite(uint8_t data);
    void _scanAddress();

    uint8_t _cols;
    uint8_t _rows;
    uint8_t _addr;
    bool    _backlight;
    bool    _initialized;


    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;
};
