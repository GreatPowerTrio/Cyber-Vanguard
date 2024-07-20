import serial
import time
COM1 = "/dev/ttyAMA0" # 串口号

ser = serial.Serial(port=COM1, baudrate=115200, stopbits=serial.STOPBITS_ONE, timeout=1) # 设置串口波特率为9600

while True:
    start_time = time.time()
    ser.write(b"(12)\r\n")
    time.sleep(0.01)
    print("(12)\r\n")
    end_time = time.time()
    print("time: ", 1000*(end_time - start_time))
    #if ser.in_waiting: # 如果串口有数据
        #ser.write(ser.read(ser.in_waiting)) # 读取串口数据并写入串口
        #print(ser.read(ser.in_waiting)) # 打印串口数据
        