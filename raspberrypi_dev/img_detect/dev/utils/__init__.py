from PIL import Image, ImageFont , ImageOps
import numpy as np
import os.path as osp
import warnings

package_dir = osp.dirname(osp.abspath(__file__))
warnings.filterwarnings("ignore", category=DeprecationWarning)

def main():
    print("\n*******************************************")
    print("Initializing package")
    print("*******************************************\n")
    try:
        print("\n*******************************************")
        print("Check the num_detect.py")
        print("*******************************************\n")
        import num_detect 
        num_detect.main()
        print("\n*******************************************")
        print("Check the hardware.py")
        print("*******************************************\n")
        import hardware
        hardware.main()
    except Exception as e:
        print(f"Error:{e}\n")
        ascill_art(osp.join(package_dir,"img/96763214_p7.png"))
        return
    ascill_art(osp.join(package_dir,"img/my_avater_small.jpg"))
    



def ascill_art(file):
    # 打开图片文件
    im = Image.open(file)

    # 通过灰度覆盖这张图片 降低图片的亮度
    im = im.convert("L")

    #im = ImageOps.invert(im)

    # 对图片进行降采样
    sample_rate = 0.35

    # 这段代码可以处理图片拉伸的情况
    font = ImageFont.truetype(osp.join(package_dir,"font/SourceCodePro-Black-1.ttf"))
    aspect_ratdio = font.getsize("x")[0] / font.getsize("x")[1]

    new_im_size = np.array(
        [im.size[0] * sample_rate, im.size[1] * sample_rate * aspect_ratdio]
    ).astype(int)

    # 用新的的图片大小resize之前的
    im = im.resize(new_im_size)

    # 将图片转换成一个numpy字符
    im = np.array(im)

    # symbols中定义了 我们字符画中的所有字符
    # 按照字符亮度升序排列
    # 文件转换的时候会不断的查询这个symbols字符集
    symbols = np.array(list(" .-vM@#*"))

    # 设置这个字符集合的取值范围的最大和最小值[0,max_symbol_index)
    im = (im - im.min()) / (im.max() - im.min()) * (symbols.size - 1)

    # 完成字符的转换
    ascii = symbols[im.astype(int)]
    lines = "\n".join(("".join(r) for r in ascii))
    print(lines)

if __name__!="__main__":
    main()

if __name__=="__main__":
    print("Testing the __init__.py")
    main()
