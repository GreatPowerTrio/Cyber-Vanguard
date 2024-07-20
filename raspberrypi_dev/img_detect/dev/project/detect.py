import cv2
import numpy as np
import time
import os.path as osp
import sys


# 路径定义
script_dir = osp.dirname(__file__)
parent_dir = osp.dirname(script_dir)
img_dir = osp.join(parent_dir, 'image')
template_dir = osp.join(img_dir, 'template_img_dir')

# 暂时添加父目录到系统变量，方便引用 hardware库
sys.path.append(parent_dir)
import utils.hardware as hardware
import utils.num_detect as num_detect
## 参数设置

# 边缘检测参数
low_threshold = 75
high_threshold = 200


def hardware_init():
    hardware.GPIO_init()
    ser = hardware.UART_init()
    return ser





#####################################################################
# 思路：    step 1 边缘检测
#           step 2 读取模板轮廓的矩形坐标
#           step 3 生成模板字典
#####################################################################
if __name__ == '__main__':

    # 定义需识别数字
    to_detect_num = None

    # 硬件初始化
    ser = hardware_init()

        
    template = cv2.imread(osp.join(template_dir, 'num_template.jpg'))
    gray_template = cv2.cvtColor(template, cv2.COLOR_BGR2GRAY)
    print('二值化模板图像已读取')

    # 边缘检测
    edged_template = cv2.Canny(template, 80, 200) # 边缘低阈值75和高阈值20

    # 轮廓检测
    template_contours, hierarchy = cv2.findContours(edged_template, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # 读取模板轮廓的矩形坐标
    x, y, w, h = cv2.boundingRect(template_contours[1])

    print(f'检测到{len(template_contours)}个轮廓')

    # 模板数字排序 1-8
    sorted_template_contours = [template_contours[5], template_contours[4], template_contours[1],template_contours[0], template_contours[7], template_contours[6], template_contours[3], template_contours[2]]

    # 构造模板字典
    digit_dict = {}

    # 分割模板图像
    for (i, c) in enumerate(sorted_template_contours):
        (x, y, w, h) = cv2.boundingRect(c)
        roi = gray_template[y+10:y+h-10, x+10:x+w-10]
        
        roi = cv2.resize(roi, (100, 150))
        digit_dict[i] = roi

    # 初始化视频捕获对象
    cap = cv2.VideoCapture(0)
    
    while True:
        # 逐帧读取视频
        start_time = time.time()
        ret, frame = cap.read()
        if not ret:
            break
        # 图像resize
        ratio = frame.shape[0] / 500.0
        orig_frame = frame.copy()
        # 转换为灰度图像
        gray_img = cv2.cvtColor(orig_frame, cv2.COLOR_BGR2GRAY)
        # 应用高斯模糊
        blur_img = cv2.GaussianBlur(gray_img, (5, 5), 0)
        
        # Canny 边缘检测
        edged_img = cv2.Canny(blur_img, low_threshold, high_threshold) # 边缘低阈值75和高阈值200
        cv2.imshow("edged_img",edged_img)
        # 定义膨胀操作的核
        kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5))
        # 对边缘检测后的图像进行膨胀操作
        dilated_img = cv2.dilate(edged_img, kernel, iterations=1)
        cv2.imshow("Dilated Image", dilated_img)

        
        # 轮廓检测
        contours, _ = cv2.findContours(edged_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        sorted_contours = sorted(contours, key=cv2.contourArea, reverse=True)[:5]
        
        
        i = 0
        screenCnt = []  # 在循环开始前初始化screenCnt
        output = []
        for c in sorted_contours:
            # 计算轮廓周长
            peri = cv2.arcLength(c, True)
            # 近似轮廓  0.02*peri为近似精度
            approx = cv2.approxPolyDP(c, 0.02*peri, True)
            
            # 如果近似轮廓有四个顶点，则认为找到了数字边缘
            if len(approx)==4 and cv2.contourArea(approx) > 10000 and cv2.contourArea(approx) < 45000:
                #num_detect.shrink_approx(approx, 3)
                
                screenCnt.append(approx)
                i += 1
                if i == 2:
                    break
        # 根据检测到的两个数字的x左坐标来判断左右并排序
        
        if len(screenCnt) == 2:
            num_detect.left_right_sort(screenCnt)

            # print(f"左:{screenCnt[0]}, 右:{screenCnt[1]}")     


        # 在尝试展示轮廓之前检查screenCnt是否已定义
        if screenCnt != []:
            cv2.drawContours(frame, screenCnt, -1, (255, 255, 0), 2)
            postion_count = 0
            warped_list = []
            thresh_list = []
            for c in screenCnt:
                
                # 检查轮廓c是否有点（即是否有效）
                if c.size == 0:
                    print("找到一个空的轮廓，跳过。")
                    continue  # 跳过当前轮廓，继续下一个轮廓
                # 透视变换
                warped_list.append(num_detect.four_point_transform(orig_frame, c.reshape(4, 2) * ratio))
                warped_list[postion_count] = cv2.cvtColor(warped_list[postion_count], cv2.COLOR_BGR2GRAY)
                thresh_list.append(cv2.threshold(warped_list[postion_count], 0, 255, cv2.THRESH_BINARY | cv2.THRESH_OTSU)[1])
                
                # 显示透视变换结果
                cv2.imshow(f"Thresh_{postion_count}", thresh_list[postion_count])
                

                if roi.size > 0:  # 检查roi是否为空
                    roi = cv2.resize(thresh_list[postion_count], (100, 150))
                    # 定义结构元素
                    kernel = np.ones((5,5), np.uint8)

                    roi_blur = cv2.GaussianBlur(roi, (7, 7), 0)
                    
                    roi_edge = cv2.Canny(roi_blur, 80, 200)
                    roi_eroded = cv2.erode(roi_edge, kernel, iterations=2)
                    # 找到边缘的轮廓
                    roi_contours, _ = cv2.findContours(roi_edge, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
                    
                    for cnt in roi_contours:
                        (x,y,w,h) = cv2.boundingRect(roi_edge)
                    
                    cv2.rectangle(roi, (x, y), (x+w, y+h), (0, 255, 0), 2)
                    cv2.imshow("roi_edge",roi_edge)
                    
                    
                    roi = cv2.resize(roi[y:y+h, x:x+w],(100, 150))
                    roi = cv2.resize(roi[0+15:150-15, 0+12:100-12],(100, 150))
                    cv2.imshow(f"ROI{postion_count}", roi)
                else:
                    print("ROI为空，跳过调整大小。")

                postion_count += 1
                # print(f"坐标位置{postion_count}:{c}")

                # 初始化模板匹配
                scores = []

                for (digit, digitROI) in digit_dict.items():
                    result = cv2.matchTemplate(roi, digitROI, cv2.TM_CCOEFF) # 模板匹配
                    (_, score, _, _) = cv2.minMaxLoc(result)
                    scores.append(score)
                trust = False
                # 找到最适合的数字，从scores中找到最大值的索引 + 1     1~8
                for score in scores:
                    if score > 0.8:
                        trust = True      
                if trust:
                    output.append(str(scores.index(max(scores)) + 1))
                    
            
            if output != []:# 如果识别结果不为空
                
                if to_detect_num is not None: # 如果这不是第一次检测
                    for i in range(len(output)):
                        cv2.putText(frame, output[i], (screenCnt[i][0][0][0], screenCnt[i][0][0][1] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
                    if len(output) == 2:
                        #time.sleep(0.5)
                        print(f"左：{output[0]}，右：{output[1]}")
                        if(hardware.UART_read(ser)==b'O'):
                            print(f"发送{output[0]}、{output[1]}成功")
                            hardware.UART_write(ser, f'({output[0]}{output[1]})\r\n'.encode())
                            
                        
                else:
                    while(hardware.UART_read(ser)==b'O'):
                        hardware.UART_write(ser, f'({output[0]}{output[0]})\r\n'.encode())
                        print(f"发送{output[0]}成功")
                        to_detect_num = int(output[0])
                        print(f"需要识别的数字为 {output[0]}")
                        print("等待下位机OK")
                        start_wait_OK = time.time()
                    
                        
                        if(((start_wait_OK - time.time())*10)%10 > 1):
                            start_wait_OK = time.time()
                            print("等待超时")
                            hardware.UART_write(ser, f'({output[0]}{output[0]})\r\n'.encode())
                            print(f"发送{output[0]}成功")
                
        else:
            1
            #print("没有找到满足条件的轮廓")

        # 展示轮廓检测结果
        cv2.imshow("Contours", frame)
            
        end_time = time.time()
        #print(f"Time: {1000*(end_time - start_time):.2f} ms")
        # 按 'q' 键退出循环
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        
