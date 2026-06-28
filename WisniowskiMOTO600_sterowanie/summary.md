# Dokumentacja Projektu: Wirtualny Pilot Somfy RTS (ESP32-S3 + CC1101)

Dokumentacja techniczna i sprzętowa implementacji wirtualnego pilota radiowego współpracującego z napędem bramowym Wiśniowski MOTO 600 RTS.

---

## 1. Specyfikacja Sprzętu i Systemu

* **Model napędu:** Wiśniowski MOTO 600 RTS (technologiczny bliźniak serii Somfy Dexxo).
* **Oryginalny pilot:** PULSAR RTS 2CH (Model: GX21 308 5143220A, producent: SOMFY F-74300 CLUSES).
* **Częstotliwość robocza:** **433,42 MHz** (pasmo dedykowane Somfy RTS; filtry tanich modułów CC1101 muszą być fizycznie dobrane pod pasmo 433 MHz, wersje 868 MHz wykazują zerowy zasięg).
* **Modulacja:** ASK/OOK (Kluczowanie amplitudy).

---

## 2. Parametry Transmisji i Kodowanie

* **Kodowanie linii:** Manchester (1 = LOW ➡️ HIGH, 0 = HIGH ➡️ LOW).
* **Timing bazowy:** Czas trwania pojedynczego półbitu wynosi dokładnie **605 µs** (weryfikowane sprzętowym licznikiem zegara procesora).
* **Struktura serii (Burst):**
  * **Ramka 1 (Pierwsza):** Impuls wybudzający (*Wake-up*: 9415 µs HIGH / 4707 µs LOW) + 4 impulsy synchronizacji sprzętowej (*HW Sync*: 2416 µs HIGH / 2416 µs LOW).
  * **Ramki kolejne (Powtórzenia):** Brak impulsów Wake-up + dokładnie 7 impulsów *HW Sync*.
  * **Dla każdej ramki w serii:** Synchronizacja programowa (*SW Sync*: 4750 µs HIGH / 1200 µs LOW) ➡️ Dane (Manchester) ➡️ Przerwa międzyramkowa (*Inter-frame gap*: ~30 ms stan niski).
  * **Liczba repetycji:** 7 dla zwykłych komend ruchowych, 10 dla procedury parowania (*PROG*).

---

## 3. Szczegółowa Struktura Ramki Danych (Somfy RTS)

Pojedyncza ramka danych składa się dokładnie z **7 bajtów (56 bitów)**. Poniższa struktura przedstawia postać ramki **przed procesem obfuskacji**:

| Bajt | Opis zawartości | Format bitowy | Uwagi |
| :---: | :--- | :---: | :--- |
| **0** | Klucz szyfrujący (Encryption Key) | `KKKK KKKK` | Zaczyna się od wartości bazowej (np. `0xA7`), inkrementowany przy każdym naciśnięciu. |
| **1** | Komenda (4 MSB) + Suma kontrolna (4 LSB) | `CCCC SSSS` | `C` - Kod operacji, `S` - 4-bitowy wynik operacji XOR (Suma kontrolna). |
| **2** | Licznik kodu zmiennego (Rolling Code) - MSB | `RRRR RRRR` | Górny bajt 16-bitowego licznika zapisanego w pamięci NVS. |
| **3** | Licznik kodu zmiennego (Rolling Code) - LSB | `RRRR RRRR` | Dolny bajt 16-bitowego licznika. |
| **4** | Unikalny ID pilota (Remote ID) - Bajt 1 | `IIII IIII` | Najstarszy bajt 24-bitowego adresu wirtualnego pilota (np. `0xAB`). |
| **5** | Unikalny ID pilota (Remote ID) - Bajt 2 | `IIII IIII` | Środkowy bajt adresu (np. `0xC1`). |
| **6** | Unikalny ID pilota (Remote ID) - Bajt 3 | `IIII IIII` | Najmłodszy bajt adresu (np. `0x23`). |

### Kody Komend (`CCCC`):
* `0x1` ➡️ **STOP / MY** (Zatrzymanie lub wywołanie pozycji ulubionej)
* `0x2` ➡️ **UP** (Otwieranie bramy)
* `0x4` ➡️ **DOWN** (Zamykanie bramy)
* `0x8` ➡️ **PROG** (Rejestracja/Parowanie pilota)

### Algorytm przetwarzania ramki (Matematyka protokołu):
1. **Suma kontrolna:** 4-bitowy XOR wszystkich 14 półbajtów (nibbles) z całej ramki (bity sumy kontrolnej w bajcie 1 na czas obliczeń są wyzerowane). Wynik wpisywany jest w dolne 4 bity bajtu 1.
2. **Obfuskacja (Zatarcie kodu):** Począwszy od bajtu 1 aż do bajtu 6, każdy kolejny bajt jest XORowany z bajtem go poprzedzającym:
   $$\text{frame}[i] = \text{frame}[i] \oplus \text{frame}[i-1] \quad (\text{dla } i \in [1, 6])$$
3. **Manchester:** Tak przetworzona ramka jest wysyłana bit po bicie z zachowaniem właściwej polaryzacji fazowej.

---

## 4. Połączenie Sprzętowe (Pinout)

Konfiguracja fizyczna dla mikrokontrolera **ESP32-S3 Super Mini** oraz modułu radiowego **CC1101**:

| Pin CC1101 | Nazwa pinu | Pin ESP32-S3 | Rola w układzie / Uwagi inżynierskie |
| :---: | :--- | :---: | :--- |
| **1** | GND | **GND** | Wspólna masa układu. |
| **2** | 3V3 | **3V3** | Zasilanie układu (zalecany stabilny kondensator filtrujący przy skokach prądowych TX). |
| **3** | **GDO0** | **2** | **KLUCZOWA POPRAWKA:** Przeniesiony z pinu 3. Pin GPIO 3 na płytce ESP32-S3 Super Mini posiada fabryczny dzielnik napięcia do linii USB (VBUS), który zniekształca zbocza sygnału mikrosekundowego i blokuje poprawną modulację OOK. Pin 2 jest elektrycznie czysty. |
| **4** | CSN | **7** | Cyfrowa linia wyboru układu (SPI CS). |
| **5** | SCK | **4** | Linia zegarowa magistrali SPI. |
| **6** | MOSI | **6** | Master Output Slave Input (Dane wyjściowe SPI). |
| **7** | MISO | **5** | Master Input Slave Output (Dane wejściowe SPI). |

---

## 5. Procedura Programowania Napędu MOTO 600

1. Podejdź fizycznie do napędu bramy i zdejmij plastikową osłonę.
2. **Przytrzymaj przycisk B** na płycie głównej napędu MOTO 600, aż wbudowana dioda zacznie miarowo migać (uruchomienie trybu nasłuchu i programowania odbiornika).
3. Wyślij komendę **`PROG`** z mikrokontrolera ESP32 (w programie wywoływane klawiszem `'p'`).
4. Dioda na płycie napędu gwałtownie zmieni rytm migania, po czym całkowicie zgaśnie. Oznacza to sukces – adres wirtualnego pilota został pomyślnie dopisany do pamięci nieulotnej napędu.
5. Przetestuj działanie wysyłając komendy ruchu (`u` - otwórz, `d` - zamknij).

---

## 6. Kluczowe Wnioski i Pułapki Implementacji

* **Tryb Asynchroniczny CC1101:** Układ radiowy musi omijać wewnętrzne bufory pakietowe FIFO biblioteki `SmartRC`. Wymusza się to poprzez bezpośredni zapis niskopoziomowy do rejestrów (`IOCFG0 = 0x0D` oraz `PKTCTRL0 = 0x30`), co mapuje fizyczny stan pinu GPIO 2 bezpośrednio na kluczowanie nadajnika radiowego.
* **Sterowanie Tabelą Mocy (PATABLE):** W modulacji OOK stan niski logiczny na pinie danych musi wygaszać falę (`0x00`), a stan wysoki wyzwalać maksymalną moc (`0xC0` ➡️ +10 dBm). Rejestr wyboru mocy `FREND0` musi być jawnie ustawiony na wartość `0x11`.
* **Eliminacja Jittera Systemowego:** Standardowa funkcja `delayMicroseconds()` w środowisku Arduino dla ESP32 bywa zakłócana przez mechanizm przełączania zadań systemu FreeRTOS. Wykorzystanie bezpośredniej funkcji z pamięci ROM procesora – `ets_delay_us()` – zagwarantowało idealną powtarzalność timingu falowego.