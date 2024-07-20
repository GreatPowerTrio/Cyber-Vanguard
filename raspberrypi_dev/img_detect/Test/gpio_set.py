import RPi.GPIO as GPIO
import time
import serial 
# 设置GPIO口编码方式为BOARD
GPIO.setmode(GPIO.BOARD)
# 设置GPIO口3为输出
GPIO.setup(3, GPIO.OUT)

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)


while True:
    if GPIO.input(3): # GPIO3输入为1
        GPIO.output(3, GPIO.LOW) # GPIO3输出为低电平
        ser.write(b"Hello")
    else:
        GPIO.output(3, GPIO.HIGH) # GPIO3输出为高电平
    time.sleep(0.1)
    

