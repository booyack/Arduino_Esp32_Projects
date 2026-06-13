#include <WiFiS3.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <string>
// --- KONFIGURACJA SIECI WI-FI ---
const char* ssid     = "WypadZBaru"; // Wpisz nazwę swojej sieci Wi-Fi
const char* password = "Multiwitamina33"; // Wpisz hasło do sieci Wi-Fi

// --- KONFIGURACJA SERWERA POGODOWEGO (Open-Meteo dla Libertowa) ---
const char* server = "api.open-meteo.com";
// Zapytanie o temperaturę (2m) oraz wilgotność względną (2m)
const String resource = "/v1/forecast?latitude=49.9722&longitude=19.8909&current=temperature_2m,relative_humidity_2m";

// --- INICJALIZACJA URZĄDZEŃ ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 
WiFiClient client;

// Zmienne do kontroli czasu (pobieranie danych co 10 minut)
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60000; // 10 minut w milisekundach (10 * 60 * 1000)

void setup() {
  Serial.begin(115200);
  
  // Inicjalizacja ekranu LCD
  lcd.init();
  lcd.setBacklight(0x03);
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Laczenie z WiFi");
  
  // Próba połączenia z Wi-Fi
  Serial.print("Laczenie z siecia: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
    attempts++;
  }
  
  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nPolaczono z Wi-Fi!");
    lcd.setCursor(0, 0);
    lcd.print("Polaczono!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\nBlad polaczenia z Wi-Fi.");
    lcd.setCursor(0, 0);
    lcd.print("Blad Wi-Fi!");
    lcd.setCursor(0, 1);
    lcd.print("Sprobuj ponownie");
    while (true); // Zatrzymanie programu w przypadku błędu
  }

  // Pierwsze pobranie danych pogodowych natychmiast po uruchomieniu
  getWeatherData();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Sprawdzamy czy upłynął określony czas od ostatniej aktualizacji
  if (currentMillis - lastUpdate >= updateInterval || lastUpdate == 0) {
    lastUpdate = currentMillis;
    getWeatherData();
  }
}

// Funkcja łącząca się z API i pobierająca dane pogodowe
void getWeatherData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pobieranie...");
  
  Serial.println("\nNawiazywanie polaczenia z serwerem pogodowym...");
  
  if (client.connect(server, 80)) {
    Serial.println("Polaczono z serwerem. Wysylanie zapytania HTTP...");
    
    // Wysłanie nagłówka HTTP GET - Używamy HTTP/1.0, aby uniknąć kodowania fragmentami (chunked transfer)
    client.println("GET " + resource + " HTTP/1.0");
    client.println("Host: " + String(server));
    client.println("Accept: application/json");
    client.println("Connection: close");
    client.println(); // Pusta linia kończąca nagłówek

    // Oczekiwanie na odpowiedź serwera z timeoutem (5 sekund)
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Timeout serwera!");
        client.stop();
        wyswietlBlad("Blad serwera");
        return;
      }
    }

    // Pominięcie nagłówków HTTP (szukamy pustej linii \r\n\r\n wskazującej start body)
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break; // Nagłówki się skończyły, teraz przechodzimy do treści (JSON)
      }
    }

    // Odczytanie treści odpowiedzi (JSON)
    String response = client.readString();
    Serial.println("Otrzymana odpowiedz JSON:");
    Serial.println(response);

    // Parsowanie danych JSON
    JsonDocument doc; // Tworzenie dokumentu JSON (kompatybilne z ArduinoJson v7/v6)
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("Blad dekodowania JSON: ");
      Serial.println(error.c_str());
      wyswietlBlad("Blad danych JSON");
      return;
    }

    // Wyciąganie parametrów pogodowych z drzewa JSON
    float temperatura = doc["current"]["temperature_2m"];
   // String unit = doc["current_units"]["temperature_2m"];
    int wilgotnosc = doc["current"]["relative_humidity_2m"];

    Serial.print("Odczytana Temp: ");
    Serial.print(temperatura);
    Serial.print(" C, Wilgotnosc: ");
    Serial.print(wilgotnosc);
    Serial.println(" %");

    // Prezentacja danych na wyświetlaczu LCD
    lcd.clear();
    
    // Pierwsza linia: Lokalizacja i Temperatura
    lcd.setCursor(0, 0);
    lcd.print("Libertow: ");
    lcd.print(temperatura, 1); // Wyświetlanie z jednym miejscem po przecinku
 //   lcd.print(unit);
    lcd.write(223);            // Znak stopnia Celsjusza (°) w tablicy znaków LCD
    lcd.print("C");
    
    // Druga linia: Wilgotność
    lcd.setCursor(0, 1);
    lcd.print("Wilgotnosc: ");
    lcd.print(wilgotnosc);
    lcd.print("%");

  } else {
    Serial.println("Polaczenie z serwerem pogodowym nie powiodlo sie.");
    wyswietlBlad("Blad polaczenia");
  }
  
  client.stop(); // Zamknięcie połączenia
}

// Pomocnicza funkcja do obsługi wizualnych błędów
void wyswietlBlad(const char* komunikat) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ojej! Blad!");
  lcd.setCursor(0, 1);
  lcd.print(komunikat);
}