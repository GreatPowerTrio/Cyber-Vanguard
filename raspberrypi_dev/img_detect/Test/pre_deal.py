import cv2
import os.path as osp
import numpy as np
# 获取路径
script_dir = osp.dirname(__file__)
img_dir = osp.join(script_dir, 'image')
captured_img_dir = osp.join(img_dir, 'captured_image_dir')


# 读取图像并转换为灰度图像
img = cv2.imread(osp.join(captured_img_dir, 'img_0.jpg'), cv2.IMREAD_GRAYSCALE)

print(img.shape)

# 高斯滤波
blur_kernel = (15,15) #卷积核
blur_img = cv2.GaussianBlur(img, blur_kernel, 0)

# 开运算
opening_kernel = np.ones((5,5),np.uint8) 
opening_img = cv2.morphologyEx(blur_img, cv2.MORPH_OPEN, opening_kernel)

# 应用二值化
ret, thresh_img = cv2.threshold(blur_img, 100, 255, cv2.THRESH_BINARY)
ret, thresh_img1 = cv2.threshold(opening_img, 100, 255, cv2.THRESH_BINARY)
cv2.imshow('Pre Image', img)
cv2.imshow('Blur Image', blur_img)
# 显示二值化后的图像
cv2.imshow('Thresholded Image', thresh_img)
cv2.imshow('Thresholded Image1', thresh_img1)


cv2.waitKey(0)
cv2.destroyAllWindows()
