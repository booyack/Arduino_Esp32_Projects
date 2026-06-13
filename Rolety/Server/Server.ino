#include <WiFi.h>
#include <WebServer.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <iostream>

using namespace std;

// ==================== KONFIGURACJA UŻYTKOWNIKA ====================
const char* const SSID_WIFI     = "TWOJA_NAZWA_WIFI";
const char* const PASS_WIFI     = "TWOJE_HASLO_WIFI";

// Wpisz tutaj 24-bitowy ID Twojego pilota, który odczytasz snifferem (np. 0x2A4B12)
const uint32_t FIZYCZNY_PILOT_ID = 0x2A4B12; 
// ==================================================================

// Definicje pinów dla Waveshare ESP32-S3-Zero (połączenie sekwencyjne)
#define CC1101_GDO0  14
#define CC1101_SCK   13
#define CC1101_MISO  12
#define CC1101_MOSI  11
#define CC1101_CSN   10

// Wymyślone, unikalne ID dla Twojego ESP32 (musi być inne niż fizyczny pilot!)
const uint32_t WIRTUALNY_PILOT_ID = 0x123456;

// Komendy protokołu Dooya/Aluprof
enum RoletaAkcja : uint8_t {
    GORA = 0x1,
    STOP = 0x2,
    DOL  = 0x3,
    P2 = 0x4
};

WebServer server(80);

// Funkcje pomocnicze generatora fali radiowej
inline void sendSync() {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(4800);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(1500);
}

inline void sendBit(bool bit) {
    if (bit) {
        digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(700);
        digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(350);
    } else {
        digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(350);
        digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(700);
    }
}

// Główna funkcja nadawcza - buduje ramkę w locie
void transmitujRozkaz(uint8_t kanal, RoletaAkcja akcja) {
    // 1. Rozbicie ID i parametrów na pojedyncze bajty
    uint8_t b1 = (FIZYCZNY_PILOT_ID >> 16) & 0xFF;
    uint8_t b2 = (FIZYCZNY_PILOT_ID >> 8) & 0xFF;
    uint8_t b3 = FIZYCZNY_PILOT_ID & 0xFF;
    uint8_t b4 = (akcja << 4) | (kanal & 0x0F);
    uint8_t b5 = b1 ^ b2 ^ b3 ^ b4; // Suma kontrolna XOR

    // 2. Pakowanie do 64-bitowego rejestru (interesuje nas 40 bitów)
    uint64_t ramka = 0;
    ramka |= (uint64_t)b1 << 32;
    ramka |= (uint64_t)b2 << 24;
    ramka |= (uint64_t)b3 << 16;
    ramka |= (uint64_t)b4 << 8;
    ramka |= b5;

    // 3. Fizyczna transmisja radiowa
    ELECHOUSE_cc1101.SetTx();
    pinMode(CC1101_GDO0, OUTPUT);

    for (int r = 0; r < 5; r++) { // Silniki Dooya wymagają 5 powtórzeń ramki
        sendSync();
        for (int i = 39; i >= 0; i--) {
            sendBit((ramka >> i) & 1);
        }
        digitalWrite(CC1101_GDO0, LOW);
        delay(10); // Przerwa międzyramkowa
    }

    // Wyłączenie nadajnika (powrót do trybu RX chroni przed sianiem szumów)
    ELECHOUSE_cc1101.SetRx(); 
    pinMode(CC1101_GDO0, INPUT);
}

// Obsługa żądań HTTP API
void handleControl() {
    if (!server.hasArg("ch") || !server.hasArg("action")) {
        server.send(400, "text/plain", "Blad: Brak parametrow 'ch' lub 'action'.");
        return;
    }

    int kanal = server.arg("ch").toInt();
    String akcjaStr = server.arg("action");
    RoletaAkcja akcja;

    if (akcjaStr == "up")        akcja = GORA;
    else if (akcjaStr == "down") akcja = DOL;
    else if (akcjaStr == "stop") akcja = STOP;
    else if (akcjaStr == "pair" || akcjaStr == "p2") akcja = P2; // Obsługa parowania
    else {
        server.send(400, "text/plain", "Blad: Nieznana akcja. Uzyj 'up', 'down' lub 'stop'.");
        return;
    }

    if (kanal < 0 || kanal > 15) {
        server.send(400, "text/plain", "Blad: Kanal poza zakresem (0-15).");
        return;
    }

    transmitujRozkaz(kanal, akcja);
    
    String response = "Wykonano: Kanal " + String(kanal) + ", Akcja: " + akcjaStr;
    server.send(200, "text/plain", response);
}

void handleRoot() {
    String html = "<html><head><meta charset='UTF-8'><style>";
    html += "button {font-size:20px; margin:10px; width:120px; height:50px;}";
    html += "</style></head><body><h1>Sterowanie Roletami</h1>";
    html += "<h3>Wszystkie okna (Kanal 0)</h3>";
    html += "<button onclick=\"fetch('/control?ch=0&action=up')\">GÓRA</button>";
    html += "<button onclick=\"fetch('/control?ch=0&action=stop')\">STOP</button>";
    html += "<button onclick=\"fetch('/control?ch=0&action=down')\">DÓŁ</button>";
    html += "<button onclick=\"fetch('/control?ch=0&action=pair')\">P2</button>";
    
    for (int i = 1; i <= 15; ++i)
    {
        html += "<h3>Roleta 1 (Salon)</h3>";
        html += "<button onclick=\"fetch('/control?ch=" + String(i) + "&action=up')\">GÓRA</button>";
        html += "<button onclick=\"fetch('/control?ch=" + String(i) + "&action=stop')\">STOP</button>";
        html += "<button onclick=\"fetch('/control?ch=" + String(i) + "&action=down')\">DÓŁ</button>";
        html += "<button onclick=\"fetch('/control?ch=" + String(i) + "&action=pair')\">P2</button>";
    }
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);

    // Inicjalizacja sprzętowa CC1101
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
    ELECHOUSE_cc1101.Init();
    
    if (!ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("KRYTYCZNY BŁĄD: Nie znaleziono modułu CC1101!");
        while (1);
    }

    ELECHOUSE_cc1101.setMHZ(433.9205); // Precyzyjne nastrojenie pod Aluprof
    ELECHOUSE_cc1101.setModulation(2);  // Modulacja OOK/ASK
    ELECHOUSE_cc1101.SetRx();

    // Połączenie z siecią WiFi
    WiFi.begin(SSID_WIFI, PASS_WIFI);
    Serial.print("Laczenie z WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\nPolaczono! Adres IP: ");
    Serial.println(WiFi.localIP());

    // Konfiguracja routingu HTTP
    server.on("/", handleRoot);
    server.on("/control", handleControl);
    server.begin();
    Serial.println("Serwer HTTP uruchomiony.");
}

void loop() {
    server.handleClient();
}