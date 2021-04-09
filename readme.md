> - Summary: 在 RT-Thread 系统上执行 Mnist 推理，纯手搭版
> - Author: lebhoryi@gmail.com & qiwen20150309@163.com
> - Date: 2020/07/19

[TOC]


# 0x00 前言

在 H743 + RT - Thread 上实现MNIST推理

两种方法:

- [ ] STM32Cube AI + RTT

- [x] CMSIS + RTT

# 0x01 文件树

```shell
/home/lebhoryi/RT-Thread/Edge_AI [git::master *] [lebhoryi@RT-AI] [15:12]
> tree -L 1 ./Project2-Mnist 
./Project2-Mnist
├── data  # mnist 数据集
├── image  # 自定义测试集
├── Mnist_CMSIS   # CMSIS + RTT | 可直接scons \ MDK 编译
├── mnist.ipynb  # 模型训练及验证
├── model  # 模型储存
└── readme.md
```

# 0x02 PC 训练 Model

## 2.1 PC 端环境

- Tensorflow: 2.3.0-dev20200515
- Numpy: 1.16.4
- Keras: 2.2.4-tf

## 2.2 数据集

> File: ./data/mnist.npz

MNIST 数据集由 60000 (训练集) + 10000(测试集) 手写字符组成, 每张图片的大小为 $ 28 * 28 $, 数据集手动下载地址 [http://yann.lecun.com/exdb/mnist/](http://yann.lecun.com/exdb/mnist/) .

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719132029.png)

## 2.3 网络结构

- 两层卷积 + 一层全连接层

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719132529.png)

## 2.4 训练模型 & 验证模型

> File: ./model/mnist.h5

- 训练模型

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719133308.png)

- 验证训练模型的准确率

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719132812.png)

- 保存权重文件

```python
# save weights
model.save_weights(model_path / 'model_weights.h5')

# load weights
model.load_weights(model_path / 'model_weights.h5')

model.compile(optimizer='adam',
             loss='sparse_categorical_crossentropy',
             metrics=['accuracy',])
loss, acc = model.evaluate(x_test, y_test)
print("Restored model, accuracy: {:5.2f}%".format(100*acc))
```

```shell
313/313 [==============================] - 1s 2ms/step - loss: 0.1226 - accuracy: 0.9651
Restored model, accuracy: 96.51%
```

# 0x03 CMSIS + RTT

> CMSIS + RTT 推理成功界面

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719151207.png)

最终的目标是将训练好的 Model 在 RT-Thread 系统上能够推理(测试)成功, 需要借助 CMSIS-NN 库.

## 3.1 实验步骤

1. 保存权重文件
2. 调用 CMSIS 库, 复现神经网络, 文件为`.c` 类文件
3. 导入权重文件和测试样例
4. 推理成功

详细步骤请移步[./Mnist_CMSIS/README.md](./Mnist_CMSIS/README.md)

# 0x04 问题收集:

## 4.1 CMSIS + RTT 找不到 `arm_math.h`

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200716155857.png)

- windows: 

  1. 勾选DSP 开关

  ![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719180007.png)

  2. 增加宏定义 

     `USE_STDPERIPH_DRIVER,ARM_MATH_CM4,__CC_ARM,__FPU_PRESENT, ARM_MATH_DSP`

  ![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719153523.png)

- linux:

  1. 先解决找不到`math.h` 

     在`./Mnist_CMSIS/packages/CMSIS-latest/SConscript` 文件中,第15行, 手动添加`DSP`, 新增:

     ```python
     CPPPATH = CPPPATH + [cwd + '/CMSIS_5/CMSIS/DSP/Include']
     ```

     ![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200720094726.png)

  2. `scons` 之后会报这样一个错误:

     ![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200720092740.png)

     ![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200720094050.png)

     解决方式如下: 

     在`./Mnist_CMSIS/board/SConscript` 文件下, 第22行, 改为:

     ```python
     CPPDEFINES = ['STM32H743xx','ARM_MATH_CM7','__FPU_PRESENT']
     ```

     ![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200720095205.png)

## 4.2 CMSIS + RTT + Mnist scons 报错

> 但是文件均已经存在

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719155954.png)

解决:

在`./Mnist_CMSIS/SConscript` 下面, 改为如图所示

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200720101650.png)
