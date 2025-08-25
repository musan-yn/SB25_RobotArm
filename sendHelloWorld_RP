#include <Arduino.h>

// UART0 を使用（Earle Philhower コアでは Serial2 が UART0）
const int UART0_TX = 8;  // GP0 → UnitV RX
const int UART0_RX = 9;  // GP1 ← UnitV TX
const unsigned long BAUD = 115200;

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && (millis()-t0 < 3000)) {}  // 最大3秒待つ（ヘッドレスでも先に進む）

  Serial.println("Pico: UART0 monitor (Serial2, TX=GP0, RX=GP1, 115200)");

  Serial2.setTX(UART0_TX);
  Serial2.setRX(UART0_RX);
  Serial2.begin(BAUD);
}

void loop() {
  static char buf[128];
  static size_t idx = 0;

  while (Serial2.available()) {
    char c = (char)Serial2.read();
    if (c == '\r') continue;          // CRは無視
    if (c == '\n') {                   // 行確定
      buf[idx] = '\0';
      if (idx) {
        Serial.print("[RX] ");
        Serial.println(buf);           // 期待: [RX] Hello world
      }
      idx = 0;
    } else {
      if (idx < sizeof(buf)-1) buf[idx++] = c;
      else idx = 0;                    // 長すぎる行は捨てて復帰
    }
  }
}
