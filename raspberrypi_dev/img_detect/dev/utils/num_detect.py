## 该文件用于给树莓派做数字识别



import cv2
import numpy as np


def cv_show(img):
    cv2.imshow('image', img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

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

# 数字左右排序
def left_right_sort(screenCnt):
    x_ords = [] 
    for c in screenCnt:
        (x, y, w, h) = cv2.boundingRect(c)
        x_ords.append(x)
    if x_ords[0] > x_ords[1]:
        screenCnt[0], screenCnt[1] = screenCnt[1], screenCnt[0]

    

# 扩展四点坐标
def expand_approx(approx,num):
    for point in approx:
            # 对x坐标进行扩展
            if point[0][0] > np.mean(approx[:, 0, 0]):
                point[0][0] += num
            else:
                point[0][0] -= num
            # 对y坐标进行扩展
            if point[0][1] > np.mean(approx[:, 0, 1]):
                point[0][1] += num
            else:
                point[0][1] -= num

# 压缩四点坐标
def shrink_approx(approx,num):
    for point in approx:
            # 对x坐标进行扩展
            if point[0][0] > np.mean(approx[:, 0, 0]):
                point[0][0] -= num
            else:
                point[0][0] += num
            # 对y坐标进行扩展
            if point[0][1] > np.mean(approx[:, 0, 1]):
                point[0][1] -= num
            else:
                point[0][1] += num

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
    
    
    warped = cv2.warpPerspective(image, M, (maxWidth+30, maxHeight+20))

    # 返回变换后的图像
    return warped


if __name__ == 'main':
    print("This is num module")