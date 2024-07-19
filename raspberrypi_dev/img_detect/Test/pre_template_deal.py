import cv2
import os.path as osp

def cv_show(img):
    cv2.imshow('image', img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

# 路径定义
script_dir = osp.dirname(__file__)
img_dir = osp.join(script_dir, 'image')
template_dir = osp.join(img_dir, 'template_img_dir')

img1 = cv2.imread(osp.join(template_dir, 'T1.jpg'), cv2.IMREAD_GRAYSCALE)
img2 = cv2.imread(osp.join(template_dir, 'T2.jpg'), cv2.IMREAD_GRAYSCALE)

# 合并图像
img = cv2.hconcat([img1, img2])
img = cv2.resize(img, (600,400))
print(img.shape)
cv_show(img)

cv2.imwrite(osp.join(template_dir, 'num_template.jpg'), img)
print('模板图像已保存')

# 高斯滤波
blur_kernel = (5,5) #卷积核
blur_img = cv2.GaussianBlur(img, blur_kernel, 0)

# 二值化
ret, thresh_img = cv2.threshold(-blur_img, 100, 255, cv2.THRESH_BINARY)
cv_show(thresh_img)

cv2.imwrite(osp.join(template_dir, 'num_template.jpg'), img)
print('二值化模板图像已保存')

# 轮廓检测
contours, hierarchy = cv2.findContours(thresh_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

thresh_img_BGR = cv2.cvtColor(thresh_img, cv2.COLOR_GRAY2BGR)

contours_img = cv2.drawContours(thresh_img_BGR, contours, -1, (0, 0, 255), 3)
cv_show(contours_img)


x, y, w, h = cv2.boundingRect(contours[6])
marked_img = cv2.rectangle(contours_img, (x, y), (x+w, y+h), (255, 0, 255), 3)
cv_show(marked_img)

print(f'检测到{len(contours)}个轮廓')





