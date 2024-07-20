import cv2 
import os
import os.path as osp
import numpy as np

script_dir = osp.dirname(__file__)
img_dir = osp.join(script_dir, 'image')
save_dir = osp.join(img_dir, 'captured_image_dir')

# 创建保存目录,如果不存在
if not osp.exists(save_dir):
    os.makedirs(save_dir)
    print(f"保存目录{save_dir}已创建")
else:
    print(f"保存目录{save_dir}已存在")
# 创建视频捕获对象
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("无法打开摄像头")
    exit()
times = int(input("请输入需要截取几个图片\n"))
i = 0
while True:
    # 读取一帧
    ret, frame = cap.read()
    if ret:
        # 显示图像
        cv2.imshow('frame', frame)
        
        # 保存图像
        if cv2.waitKey(1) & 0xFF == ord('s'):
            cv2.imwrite(osp.join(save_dir , f"img_{i}.jpg"), frame)
            print(f"图像已保存为 img_{i}.jpg")
            times -= 1
            i += 1
            
        
    else:
        print("无法读取摄像头图像")
    if cv2.waitKey(1) & 0xFF == ord('q') or times == 0:  
        break
# 释放摄像头
cap.release()
# 关闭所有OpenCV窗口
cv2.destroyAllWindows()