# UnitV / MaixPy (K210)
# 1秒ごとに "Hello world\n" をUARTで送信
import time
from machine import UART
from fpioa_manager import fm

UART_TX_PIN = 34  # UnitV TX → Pico RX(GP1)
UART_RX_PIN = 35  # UnitV RX ← Pico TX(GP0)

fm.register(UART_TX_PIN, fm.fpioa.UART1_TX, force=True)
fm.register(UART_RX_PIN, fm.fpioa.UART1_RX, force=True)

uart = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=256)

print("UnitV: UART1 115200 on TX={}, RX={}".format(UART_TX_PIN, UART_RX_PIN))
while True:
    uart.write("Hello world\n")   # ★ 改行つき
    time.sleep(1)
