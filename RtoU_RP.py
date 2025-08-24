#include <Arduino.h>  // ← これが無いと Serial, String, millis などが未定義になります

// --- UART1 ピン（必要なら変更） ---
const int PICO_UART1_RX = 9;  // GP9 ← UnitV TX
const int PICO_UART1_TX = 8;  // GP8 → UnitV RX
const unsigned long BAUD = 115200;

unsigned long lastPing = 0;

void setup() {
  // USB シリアル（デバッグ）
  Serial.begin(115200);
  // USBが開かなくても数秒で先に進むように（ヘッドレスでも動く）
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 3000)) { /* wait up to 3s */ }

  // UART1 のピン割り当て（Earle Philhower core）
  Serial1.setRX(PICO_UART1_RX);
  Serial1.setTX(PICO_UART1_TX);

  // UART1 開始
  Serial1.begin(BAUD);

  Serial.println("Pico 2W: UART1 started at 115200 (RX=GP9, TX=GP8)");
}

void loop() {
  // --- 受信: UnitV からの 1 行（CSV + 改行）を読む ---
  if (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');  // 改行まで
    line.trim();
    if (line.length() > 0) {
      Serial.print("RX line: ");
      Serial.println(line);

      // 簡単な CSV パース例（3 要素想定: str,int,int）
      int c1 = line.indexOf(',');
      int c2 = (c1 >= 0) ? line.indexOf(',', c1 + 1) : -1;

      if (c1 > 0 && c2 > c1) {
        String s0 = line.substring(0, c1);
        long v1 = line.substring(c1 + 1, c2).toInt();
        long v2 = line.substring(c2 + 1).toInt();

        Serial.print("  field0: "); Serial.println(s0);
        Serial.print("  field1: "); Serial.println(v1);
        Serial.print("  field2: "); Serial.println(v2);
      }
    }
  }

  // --- 送信: 1 秒ごとに PING を送る（確認用） ---
  if (millis() - lastPing >= 1000) {
    lastPing = millis();
    Serial1.print("PING\n");  // UnitV 側が読むなら改行つき推奨
  }
}
