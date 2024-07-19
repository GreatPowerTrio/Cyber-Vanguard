####
# 该文件用于初始化树莓派硬件设备
# 包括GPIO口和串口



import RPi.GPIO as GPIO
import time
import serial 


COM1 = "/dev/ttyAMA0" # 串口号

# GPIO初始化
def GPIO_init():
    # 设置GPIO口编码方式为BOARD
    GPIO.setmode(GPIO.BOARD)
    # 设置GPIO口3为输出
    GPIO.setup(3, GPIO.OUT)


# 串口初始化
def UART_init():
    ser = serial.Serial(port=COM1, baudrate=115200, stopbits=serial.STOPBITS_ONE, timeout=1) # 设置串口波特率为9600
    return ser


# 串口写入
def UART_write(ser,message):
    message = message
    time.sleep(0.01)
    if not ser.in_waiting: # 如果串口有数据
        ser.write(message) # 读取串口数据并写入串口
        print(message) # 打印串口数据

# 串口读
def UART_read(ser):
    if ser.in_waiting: # 如果串口有数据
        message = ser.read(ser.in_waiting) # 打印串口数据
        print(message)
    else:
        message = None
    return message


if __name__ == 'main':
    print("This is hardware module")