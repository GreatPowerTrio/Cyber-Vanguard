import cv2
import os.path as osp

script_dir = osp.dirname(__file__)
img_dir = osp.join(script_dir, 'image')

img = cv2.imread(img_dir+'/2.jpg',cv2.COLOR_BGR2GRAY)

cv2.imshow('image', img)
cv2.waitKey(0)
cv2.destroyAllWindows()
height, width = img.shape[0:2]

thresh = 60



cv2.imshow('image', img)
cv2.waitKey(0)
cv2.destroyAllWindows()