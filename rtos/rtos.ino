#include <Arduino_FreeRTOS.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

// Globalny obiekt do obsługi wbudowanej matrycy LED 12x8
ArduinoLEDMatrix matrix;

// Uchwyty zadań (Task Handles)
TaskHandle_t TaskBlinkHandle = NULL;
TaskHandle_t TaskMatrixHandle = NULL;

// Wątek 1: Sterowanie wbudowaną diodą LED (Blink)
// Dioda miga niezależnie od tekstu wyświetlanego na matrycy.
void TaskBlink(void *pvParameters) {
  (void) pvParameters;
  
  // Konfiguracja pinu diody wbudowanej (Pin 13) jako wyjście
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Oddaj procesor innym zadaniom na 500 ms
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Oddaj procesor innym zadaniom na 500 ms
  }
}

// Wątek 2: Sterowanie matrycą LED Matrix (przewijany tekst)
void TaskMatrix(void *pvParameters) {
  (void) pvParameters;

  // Tekst, który będzie się przewijał (spacje na początku i końcu poprawiają czytelność)
  const char* message = "   Arduino UNO R4 WiFi   ";
  
  for (;;) {
    // 1. Rozpocznij rysowanie
    matrix.beginDraw();
    
    // 2. Ustaw prędkość przewijania (w milisekundach na klatkę)
    matrix.textScrollSpeed(80);
    
    // 3. Ustaw czcionkę systemową
    matrix.textFont(Font_5x7);
    
    // 4. Rozpocznij pisanie tekstu w określonej pozycji (x=0, y=1) z pełną jasnością (0xFFFFFF)
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.print(message);
    
    // 5. Zakończ tekst z parametrem kierunku przewijania w lewo
    matrix.endText(SCROLL_LEFT);
    
    // 6. Zakończ sesję rysowania (uruchamia animację)
    matrix.endDraw();

    // Obliczenie czasu potrzebnego na pełne przewinięcie tekstu:
    // Długość wiadomości: 25 znaków.
    // Każdy znak ma szerokość 5 pikseli + 1 piksel odstępu = 6 pikseli na znak.
    // Łączna szerokość tekstu = 25 * 6 = 150 pikseli.
    // Szerokość wyświetlacza to 12 pikseli. Całkowity dystans przewijania = 150 + 12 = 162 kroki.
    // Czas trwania kroku to 80 ms (ustawiony w textScrollSpeed).
    // Całkowity czas jednego przewinięcia = 162 * 80 ms = 12960 ms (około 13 sekund).
    // Ponieważ matrix.endDraw() jest nieblokujące i od razu wraca do pętli,
    // usypiamy ten wątek na czas trwania animacji, by nie restartować jej bez przerwy.
    vTaskDelay(13000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  // Inicjalizacja portu szeregowego USB do celów diagnostycznych
  Serial.begin(115200);
  
  // Opcjonalnie: czekaj na podłączenie terminala szeregowego (tylko do debugowania)
  // while (!Serial) { ; }
  
  Serial.println(F("Uruchamianie systemu wielowątkowego FreeRTOS na Arduino UNO R4..."));

  // Inicjalizacja matrycy LED
  matrix.begin();

  // Utworzenie wątku do migania diodą LED
  // Parametry: (funkcja zadania, nazwa, rozmiar stosu w słowach, parametry, priorytet, uchwyt)
  xTaskCreate(
    TaskBlink,
    "BlinkTask",
    128,             // 128 słów (512 bajtów) - wystarczające dla prostego przełączania pinu
    NULL,
    1,               // Niski priorytet
    &TaskBlinkHandle
  );

  // Utworzenie wątku do obsługi matrycy LED Matrix
  xTaskCreate(
    TaskMatrix,
    "MatrixTask",
    512,             // 512 słów (2048 bajtów) - większy stos ze względu na użycie biblioteki graficznej
    NULL,
    1,               // Taki sam priorytet
    &TaskMatrixHandle
  );

  // Uwaga: W rdzeniu Arduino UNO R4 (Renesas), planista FreeRTOS
  // uruchamia się automatycznie po zakończeniu funkcji setup().
  Serial.println(F("System gotowy. Start planisty FreeRTOS."));
}

void loop() {
  // Pętla loop pozostaje pusta, ponieważ FreeRTOS automatycznie
  // rozdziela czas procesora pomiędzy zdefiniowane wyżej wątki (TaskBlink i TaskMatrix).
}
