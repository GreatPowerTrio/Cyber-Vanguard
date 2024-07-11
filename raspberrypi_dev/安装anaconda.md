# 引言

安装anaconda，为的是保证python环境不冲突，在树莓派5B上搭建需要考虑版本。树莓派5使用的是基于64位 ARM架构的处理器，因此在安装软件时应选择aarch64版本（适用所有软件安装）。


# anaconda 官网

官网首页:
https://www.anaconda.com/

**要在电脑上下载完在传到树莓派上会好一点，直接在树莓派上下载速度很慢**

# 安装

下载完成后执行脚本安装

```bash
bash ~/Anaconda3-2024.02-1-Linux-aarch64.sh
```

跟随指引一路yes即可安装成功

# 配置环境变量
进入.bashrc
```bash
nano ~/.bashrc
```
在最后一行添加环境变量
```bash
export PATH="/home/pi/anaconda3/bin:$PATH"
#这个PATH的地址应该与你安装anaconda3的地址一致在+/bin
```

最后source一下
```bash
source ~/.bashrc

```

# 测试是否安装成功

终端输入
```bash
conda --version
```

# 参考blog

```bash
https://blog.csdn.net/m0_64301651/article/details/137422983
```