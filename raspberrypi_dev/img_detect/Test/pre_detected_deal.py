import cv2
import os.path as osp
import numpy as np

# 获取路径
script_dir = osp.dirname(__file__)
img_dir = osp.join(script_dir, 'image')
captured_img_dir = osp.join(img_dir, 'captured_image_dir')


# resize function 
# function: 按照指定的宽度或者高度进行等比例缩放 
def resize(img, width=None, height=None, inter=cv2.INTER_AREA):
    # 初始化缩放比例，并获取图像尺寸
    dim = None
    (h, w) = img.shape[:2]

    # 如果宽度和高度均为0，则返回原图像
    if width is None and height is None:
        return img

    # 计算宽高比
    if width is None:
        r = height / float(h)
        dim = (int(w*r), height)
    else:
        r = width / float(w)
        dim = (width, int(h*r))

    # 缩放图像
    resized = cv2.resize(img, dim, interpolation=inter)

    # 返回缩放后的图像
    return resized

def order_points(pts):
    # 一共4个坐标点
    rect = np.zeros((4, 2), dtype="float32")

    # 按顺序找到对应坐标0123分别是左上，右上，右下，左下
    # 计算左上，右下
    s = pts.sum(axis=1)
    rect[0] = pts[np.argmin(s)]
    rect[2] = pts[np.argmax(s)]

    # 计算右上和左下
    diff = np.diff(pts, axis=1)
    rect[1] = pts[np.argmin(diff)]
    rect[3] = pts[np.argmax(diff)]

    return rect

####
# 透视变换思路
# 1. 通过四个坐标点计算变换矩阵 几何上的 点到点的线段长度公式
# 2. 使用变换矩阵对图像进行透视变换

####

def four_point_transform(image, pts):
    # 获取输入坐标点
    rect = order_points(pts)
    (tl, tr, br, bl) = rect

    # 计算输入的w和h值
    widthA = np.sqrt(((br[0] - bl[0]) ** 2) + ((br[1] - bl[1]) ** 2))
    widthB = np.sqrt(((tr[0] - tl[0]) ** 2) + ((tr[1] - tl[1]) ** 2))
    maxWidth = max(int(widthA), int(widthB))

    heightA = np.sqrt(((tr[0] - br[0]) ** 2) + ((tr[1] - br[1]) ** 2))
    heightB = np.sqrt(((tl[0] - bl[0]) ** 2) + ((tl[1] - bl[1]) ** 2))
    maxHeight = max(int(heightA), int(heightB))

    # 变换后对应坐标位置
    dst = np.array([
        [0, 0],
        [maxWidth - 1, 0],
        [maxWidth - 1, maxHeight - 1],
        [0, maxHeight - 1]], dtype="float32")

    # 计算透视变换矩阵
    M = cv2.getPerspectiveTransform(rect, dst) # 变换前坐标和变换后坐标 -> 透视变换矩阵
    warped = cv2.warpPerspective(image, M, (maxWidth, maxHeight))

    # 返回变换后的图像
    return warped


def cv_show(img):
    cv2.imshow('image', img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
#############################################################
# 思路：    step 1 读取图像并转换为灰度图像
#           step 2 高斯滤波
#           step 3 开运算
#           step 4 应用二值化
#           step 5 边缘检测
#           step 6 轮廓检测
#           step 7 仿射变换
#           step 8 OCR识别    
#############################################################


# 读取输入数据
img = cv2.imread(osp.join(captured_img_dir, 'img_2.jpg'))

# 图像resize
ratio = img.shape[0] / 500.0
orig_img = img.copy()

#! 图像预处理

# 图像缩放
img = resize(orig_img, height=500)

# 图像转换成灰度图 
gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

# 高斯滤波
blur_img = cv2.GaussianBlur(gray_img, (5,5), 0)

# 边缘检测
edged_img = cv2.Canny(blur_img, 75, 200)  # 边缘低阈值75和高阈值200

# 展示边缘检测结果
cv_show(edged_img)

# 轮廓检测
contours, _ = cv2.findContours(edged_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
sorted_contours = sorted(contours, key=cv2.contourArea, reverse=True)[:5]

for c in sorted_contours:
    screenCnt = []
    # 计算轮廓周长
    peri = cv2.arcLength(c, True)
    # 近似轮廓  0.02*peri为近似精度
    approx = cv2.approxPolyDP(c, 0.02*peri, True)

    # 如果近似轮廓有四个顶点，则认为找到了数字边缘
    if len(approx) == 4:
        
        screenCnt.append(approx)
        
# 在尝试展示轮廓之前检查screenCnt是否已定义
if not screenCnt:
    # 展示轮廓检测结果
    cv2.drawContours(blur_img, screenCnt, -1, (255, 255, 0), 2)
    cv_show(blur_img)
    if len(screenCnt) == 2:
        print("坐标位置1", screenCnt[0])
        print("坐标位置2", screenCnt[1])
else:
    print("没有找到满足条件的轮廓")


# 透视变换
warped_img = four_point_transform(orig_img, screenCnt.reshape(4, 2) * ratio)

# 展示透视变换结果

warped_img = cv2.cvtColor(warped_img, cv2.COLOR_BGR2GRAY)
thresh_img = cv2.threshold(warped_img, 0, 255, cv2.THRESH_BINARY | cv2.THRESH_OTSU)[1]
cv_show(thresh_img)









