# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

---

## [1.0.0] - 2026-02-10

### Added
- Initial stable release of IskakINO_LiquidCrystal_I2C
- Automatic I2C address detection for PCF8574 (0x20–0x27)
- Backward compatible API with common LiquidCrystal_I2C libraries
- ESP32 compatibility
- Compile-time configuration options
- Optional serial debug support
- Basic example sketch with wiring reference
- Added full LiquidCrystal compatible APIs
- Added cursor, blink, scroll, autoscroll support
- Added missing HD44780 command macros


### Changed
- Clean internal refactor for lower RAM and flash usage
- Simplified initialization flow while keeping legacy behavior

### Fixed
- Improved I2C initialization stability
- Reliable 4-bit LCD initialization sequence
- Initial stable release of IskakINO_LiquidCrystal_I2C
---

