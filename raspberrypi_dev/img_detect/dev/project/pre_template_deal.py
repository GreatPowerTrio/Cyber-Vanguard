import cv2
import os.path as osp
import numpy as np



def cv_show(img):
    cv2.imshow('image', img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

# 路径定义
script_dir = osp.dirname(__file__)
parent_dir = osp.dirname(script_dir)
img_dir = osp.join(parent_dir, 'image')
template_dir = osp.join(img_dir, 'template_img_dir')



#####################################################################
# 思路：    step 1 边缘检测
#           step 2 读取模板轮廓的矩形坐标
#           step 3 生成模板字典
#####################################################################




thresh_img = cv2.imread(osp.join(template_dir, 'num_template.jpg'),cv2.IMREAD_GRAYSCALE)
print('二值化模板图像已读取')

# 高斯滤波
blur_kernel = (5,5) #卷积核
blur_img = cv2.GaussianBlur(thresh_img, blur_kernel, 0)

# 二值化
ret, thresh_img = cv2.threshold(blur_img, 100, 255, cv2.THRESH_BINARY)

edged_img = cv2.Canny(blur_img, 75, 200) # 边缘低阈值75和高阈值200

# 轮廓检测
template_contours, hierarchy = cv2.findContours(edged_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)



contours_img = cv2.drawContours(thresh_img, template_contours, -1, (0, 0, 255), 3)
cv_show(contours_img)
print(template_contours)
# 读取模板轮廓的矩形坐标
x, y, w, h = cv2.boundingRect(template_contours[6])
marked_img = cv2.rectangle(contours_img, (x, y), (x+w, y+h), (255, 0, 255), 3)
cv_show(marked_img)

print(f'检测到{len(template_contours)}个轮廓')






