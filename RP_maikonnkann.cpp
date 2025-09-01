#include <Arduino.h>  // ← これが必須（.cpp なら）

// Raspberry Pi Pico 2W (Arduino core / Earle Philhower)
// UART0: TX=GP0, RX=GP1  @115200 8N1

const int PICO_UART_TX = 0;  // GP0
const int PICO_UART_RX = 1;  // GP1
const unsigned long BAUD = 115200;

String lineBuf;
unsigned long lastPing = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial1.setTX(PICO_UART_TX);
  Serial1.setRX(PICO_UART_RX);
  Serial1.begin(BAUD);

  Serial.println("Pico 2W: UART0 started at 115200 (TX=GP0, RX=GP1)");
  delay(50);

  Serial1.println("HELLO from Pico");
}

void loop() {
  while (Serial1.available() > 0) {
    char c = (char)Serial1.read();
    if (c == '\r') continue;
    if (c == '\n') {
      Serial.print("[Pico] RX: ");
      Serial.println(lineBuf);
      lineBuf = "";
    } else {
      lineBuf += c;
      if (lineBuf.length() > 256) lineBuf.remove(0);
    }
  }

  unsigned long now = millis();
  if (now - lastPing >= 1000) {
    lastPing = now;
    Serial1.print("PING t=");
    Serial1.println(now);
  }
}
