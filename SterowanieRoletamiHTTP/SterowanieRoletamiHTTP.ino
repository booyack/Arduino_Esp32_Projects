#include <WiFi.h>
#include <WebServer.h>
#include "ELECHOUSE_CC1101_SRC_DRV.h"

// Fizyczne piny Twojego ESP32-S3 SuperMini
#define CC1101_SCK   4
#define CC1101_MISO  5
#define CC1101_MOSI  6
#define CC1101_CSN   7
#define CC1101_GDO0  3

// TUTAJ WPISZ DANE SWOJEJ DOMOWEJ SIECI WI-FI
const char* ssid = "WypadZBaru";
const char* password = "Multiwitamina33";

WebServer server(80);

// Czyste komendy Aluprof / Dooya
const uint8_t CMD_GORA = 0x11;
const uint8_t CMD_DOL  = 0x33;
const uint8_t CMD_STOP = 0x55;

// Responsywny interfejs UI
const char HTML_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Centrum Sterowania Roletami</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background: #121212; color: #e0e0e0; padding: 10px; margin: 0; }
        .container { max-width: 480px; margin: 20px auto; background: #1e1e1e; padding: 15px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.6); }
        h1 { color: #00adb5; font-size: 22px; margin-bottom: 20px; }
        .row { display: flex; justify-content: space-between; align-items: center; padding: 12px 5px; border-bottom: 1px solid #2d2d2d; }
        .row:last-child { border-none; }
        .btn { padding: 10px 18px; font-weight: bold; border: none; border-radius: 6px; cursor: pointer; color: white; transition: background 0.15s; margin-left: 4px; }
        .btn-up { background: #28a745; } .btn-up:hover { background: #218838; }
        .btn-stop { background: #dc3545; } .btn-stop:hover { background: #c82333; }
        .btn-down { background: #007bff; } .btn-down:hover { background: #0069d9; }
        .roll-name { font-size: 15px; font-weight: bold; color: #b3b3b3; }
        .row-global { background: #252525; border-radius: 6px; margin-bottom: 10px; padding: 12px 10px; }
        .row-global .roll-name { color: #00adb5; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Rolety Aluprof DC313</h1>
        <div id="lista-rolet"></div>
    </div>

    <script>
        const lista = document.getElementById('lista-rolet');
        
        function generujWiersz(kanal, etykieta, czyGlobalny = false) {
            const div = document.createElement('div');
            div.className = czyGlobalny ? 'row row-global' : 'row';
            div.innerHTML = `
                <span class="roll-name">${etykieta}</span>
                <div>
                    <button class="btn btn-up" onclick="wyslij(${kanal}, 'gora')">▲</button>
                    <button class="btn btn-stop" onclick="wyslij(${kanal}, 'stop')">■</button>
                    <button class="btn btn-down" onclick="wyslij(${kanal}, 'dol')">▼</button>
                </div>
            `;
            lista.appendChild(div);
        }

        generujWiersz(0, 'WSZYSTKIE ROZESŁANE (Ch 0)', true);
        for (let i = 1; i <= 15; i++) {
            generujWiersz(i, `Roleta Kanał ${i}`);
        }

        function wyslij(kanal, komenda) {
            fetch(`/control?ch=${kanal}&cmd=${komenda}`)
                .then(r => console.log(`Wysłano Ch: ${kanal} -> ${komenda}`))
                .catch(e => alert('Błąd połączenia z ESP32'));
        }
    </script>
</body>
</html>
)=====";

void nadajBit(bool bit) {
  if (bit) {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(720);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(360);
  } else {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(360);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(720);
  }
}

void nadajPaczkeAluprof(uint8_t kanal, uint8_t komenda) {
  uint8_t ramka[5] = {0x2A, 0x3D, 0x06, (uint8_t)(0x40 | (kanal & 0x0F)), komenda};

  ELECHOUSE_cc1101.SetTx();
  pinMode(CC1101_GDO0, OUTPUT);

  for (int powtorzenie = 0; powtorzenie < 6; powtorzenie++) {
    digitalWrite(CC1101_GDO0, HIGH); delayMicroseconds(4800);
    digitalWrite(CC1101_GDO0, LOW);  delayMicroseconds(1600);

    for (int bajtIdx = 0; bajtIdx < 5; bajtIdx++) {
      for (int bitIdx = 7; bitIdx >= 0; bitIdx--) {
        nadajBit((ramka[bajtIdx] >> bitIdx) & 0x01);
      }
    }
    digitalWrite(CC1101_GDO0, LOW); delay(10);
  }

  pinMode(CC1101_GDO0, INPUT);
  ELECHOUSE_cc1101.SetRx();
}

void handleControl() {
  if (server.hasArg("ch") && server.hasArg("cmd")) {
    int kanal = server.arg("ch").toInt();
    String cmdStr = server.arg("cmd");
    uint8_t cmdHex = CMD_STOP;

    if (cmdStr == "gora") cmdHex = CMD_GORA;
    else if (cmdStr == "dol") cmdHex = CMD_DOL;

    Serial.printf("Sieć lokalna -> Kanał: %d, Akcja: %s\n", kanal, cmdStr.c_str());
    
    nadajPaczkeAluprof(kanal, cmdHex);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Błąd parametrów");
  }
}

void handleRoot() {
  server.send_P(200, "text/html", HTML_INDEX);
}

void setup() {
  Serial.begin(115200);

  // CC1101 Init
  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CSN);
  ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setMHZ(433.92);
  
  ELECHOUSE_cc1101.SpiWriteReg(0x02, 0x0D); 
  ELECHOUSE_cc1101.SpiWriteReg(0x08, 0x30); 
  ELECHOUSE_cc1101.SpiWriteReg(0x1B, 0x03); 
  ELECHOUSE_cc1101.SpiWriteReg(0x21, 0x11); 
  ELECHOUSE_cc1101.SetRx();
  pinMode(CC1101_GDO0, INPUT);

  // Łączenie z domowym Wi-Fi
  Serial.print("Łączenie z siecią "); Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Czekamy na przydzielenie IP przez DHCP routera
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n=== POŁĄCZONO Z SIECIĄ DOMOWĄ ===");
  Serial.print("Adres sieciowy Twoich rolet: http://"); 
  Serial.println(WiFi.localIP()); // Ten adres wpisujesz w przeglądarce

  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(2);
}