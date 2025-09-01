#include "main.h"
#include "usart.h"     // ← 必須（huart1/huart2, MX_USARTx_UART_Init は usart.c で定義）
#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ===== Pico←→STM32 (USART2) 行受信用ワーク ===== */
static uint8_t rx_ch;
static char rx_line[128];
static volatile uint16_t rx_len = 0;
static volatile uint8_t line_ready = 0;

/* ===== サーボ制御フラグ ===== */
static volatile uint8_t servo_run = 0;

/* ===== サイン波パラメータ（元コード準拠） ===== */
static float rad1 = 0.0f;
static float rad2 = (float)M_PI;     // 逆位相
static const float amp = 500.0f;     // 振幅
static const int   center = 2048;    // 中立（12bit範囲: 0..4095）
static const float step = 0.05f;     // 1サイクルの刻み

/* ===== 前方宣言 ===== */
static void start_uart2_rx_it(void);
static void process_line(void);

/* ===== SCSCL チェックサム（ID～最後のパラメータの総和のビット反転） ===== */
static uint8_t scs_checksum(const uint8_t *p, uint8_t n) {
  uint32_t s = 0;
  for (uint8_t i = 0; i < n; i++) s += p[i];
  return (uint8_t)(~s);
}

/* ===== SCSCL WritePos：ID, 位置, 時間, 速度 を書く =====
   形式: FF FF | ID | LEN(0x09) | 0x03 | 0x2A | POSL POSH TIMEL TIMEH SPEEDL SPEEDH | CHK */
static HAL_StatusTypeDef scscl_write_pos(uint8_t id, uint16_t pos, uint16_t time, uint16_t speed) {
  uint8_t pkt[2 + 1 + 1 + 1 + 1 + 6 + 1];
  uint8_t i = 0;
  pkt[i++] = 0xFF; pkt[i++] = 0xFF;
  pkt[i++] = id;
  pkt[i++] = 0x09;          // data length
  pkt[i++] = 0x03;          // WRITE
  pkt[i++] = 0x2A;          // target position address
  pkt[i++] = (uint8_t)(pos & 0xFF);
  pkt[i++] = (uint8_t)(pos >> 8);
  pkt[i++] = (uint8_t)(time & 0xFF);
  pkt[i++] = (uint8_t)(time >> 8);
  pkt[i++] = (uint8_t)(speed & 0xFF);
  pkt[i++] = (uint8_t)(speed >> 8);
  pkt[i++] = scs_checksum(&pkt[2], i - 2);
  // USART1 は CubeMX で Half-Duplex 1Mbps に設定済みとする
  return HAL_UART_Transmit(&huart1, pkt, i, 10);
}

/* ===== UART2 受信割り込みコールバック（1行モード） ===== */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
    char c = (char)rx_ch;
    if (c == '\n') { rx_line[rx_len] = '\0'; line_ready = 1; rx_len = 0; }
    else if (c != '\r') { if (rx_len < sizeof(rx_line)-1) rx_line[rx_len++] = c; }
    start_uart2_rx_it();
  }
}

/* ===== 受信行の処理（RUN / STOP） ===== */
static void process_line(void) {
  if (!strcmp(rx_line, "RUN") || !strcmp(rx_line, "START")) {
    servo_run = 1;
    const char *ack = "ACK RUN\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)ack, strlen(ack), 10);
  } else if (!strcmp(rx_line, "STOP")) {
    servo_run = 0;
    const char *ack = "ACK STOP\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)ack, strlen(ack), 10);
  } else {
    const char *nak = "NAK Unknown cmd\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)nak, strlen(nak), 10);
  }
}

/* ===== 1バイト割り込み受信の開始 ===== */
static void start_uart2_rx_it(void) {
  HAL_UART_Receive_IT(&huart2, &rx_ch, 1);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();   // Pico通信用 115200
  MX_USART1_UART_Init();   // サーボ用   1Mbps Half-Duplex（CubeMXで設定）

  start_uart2_rx_it();

  const char *hello = "STM32 servo controller ready (send RUN/STOP)\\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)hello, strlen(hello), 50);

  uint32_t t_prev = HAL_GetTick();

  while (1) {
    if (line_ready) { process_line(); line_ready = 0; }

    if (servo_run && (HAL_GetTick() - t_prev) >= 20) {   // 50Hz
      t_prev += 20;

      int pos1 = center + (int)(amp * sinf(rad1));
      int pos2 = center + (int)(amp * sinf(rad2));

      // 範囲クリップ（0..4095）
      if (pos1 < 0) pos1 = 0; else if (pos1 > 4095) pos1 = 4095;
      if (pos2 < 0) pos2 = 0; else if (pos2 > 4095) pos2 = 4095;

      // サーボID 1,2 へ書き込み（時間=0, 速度=0 → 元コード準拠）
      scscl_write_pos(1, (uint16_t)pos1, 0, 0);
      scscl_write_pos(2, (uint16_t)pos2, 0, 0);

      rad1 += step;
      rad2 += step;
    }
  }
}
