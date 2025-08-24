# UnitV / M5StickV / MaixPy (K210) 用
# UART1: IO34=TX, IO35=RX（必要に応じて変更）
import time
from machine import UART
from fpioa_manager import fm

# --- ピン割り当て（必要なら変更） ---
UART_TX_PIN = 34  # UnitV TX
UART_RX_PIN = 35  # UnitV RX

fm.register(UART_TX_PIN, fm.fpioa.UART1_TX)
fm.register(UART_RX_PIN, fm.fpioa.UART1_RX)

# --- UART 初期化 ---
uart = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

# 送信用のダミーデータ作成例（CSV + 改行）
def make_payload():
    # 好きなデータに置き換え可。例: "center_x,center_y,area"
    return "hello,123,456\n"

print("UART1 started at 115200")

while True:
    # 送信
    uart.write(make_payload())

    # 受信（あればコンソール表示）
    if uart.any():
        try:
            data = uart.read()
            if data:
                print("RX from Pico:", data)
        except Exception as e:
            print("UART read error:", e)

    time.sleep_ms(500)
