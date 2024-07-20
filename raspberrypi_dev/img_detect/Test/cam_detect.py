import cv2
import numpy as np
import time


# 初始化视频捕获对象
cap = cv2.VideoCapture(0)

while True:
    # 逐帧读取视频
    start_time = time.time()
    ret, frame = cap.read()
    if not ret:
        break
    
    # 转换为灰度图像
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    # 应用高斯模糊
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    # Canny 边缘检测
    edged = cv2.Canny(blurred, 30, 150)

    # 轮廓检测
    contours, _ = cv2.findContours(edged, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    sorted_contours = sorted(contours, key=cv2.contourArea, reverse=True)[:5]

    i = 0
    screenCnt = []  # 在循环开始前初始化screenCnt
    for c in sorted_contours:
        # 计算轮廓周长
        peri = cv2.arcLength(c, True)
        # 近似轮廓  0.02*peri为近似精度
        approx = cv2.approxPolyDP(c, 0.02*peri, True)

        # 如果近似轮廓有四个顶点，则认为找到了数字边缘
        if len(approx) == 4:
            screenCnt.append(approx)
            i += 1
            if i == 2:
                break

    # 在尝试展示轮廓之前检查screenCnt是否已定义
    if screenCnt != []:
        cv2.drawContours(frame, screenCnt, -1, (255, 255, 0), 2)
        if len(screenCnt) == 2:
            print(f"坐标位置1:{screenCnt[0]}")
            print(f"坐标位置2:{screenCnt[1]}")
    else:
        print("没有找到满足条件的轮廓")

    # 展示轮廓检测结果
    cv2.imshow("Contours", frame)
    end_time = time.time()
    print(f"Time: {1000*(end_time - start_time)} ms")
    # 按 'q' 键退出循环
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
        
