[[_TOC_]]

## 0x00 如何运行

- 拉取`Mnist_CMSIS` 或者 `Mnist_CMSIS.7z` 到本地, 同文件夹, `Mnist_CMSIS` 有520M, 建议下载压缩包, 仅66.4M

- 运行方法:

  - scons
  - MDK5 编译

- CMSIS + RTT 推理成功界面

  ![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719151207.png)

> 在github上的文件夹中,已经包含实验运行所需要的CMSIS packages, 下载即可运行

## 0x01 实验详细步骤

1. PC 端读取训练好的权重文件并且保存为`int` 型
2. 开启`CMSIS`
3. 通过`CMSIS` 重构神经网络, 文件为`.c` 类文件
4. 导入权重文件和测试样例
5. 推理成功

## 0x02 PC 端保存权重文件并读取

- 权重文件保存 & 验证

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

  ```shel
  313/313 [==============================] - 1s 2ms/step - loss: 0.1226 - accuracy: 0.9651
  Restored model, accuracy: 96.51%
  ```

- 权重文件读取

  ```python
  # 读取权重 h5 文件
  import h5py
  
  weights_file = model_path / 'model_weights.h5'
  weights_txt = model_path / 'model_weights_int.txt'
  
  def traverse_datasets(hdf_file):
      def h5py_dataset_iterator(g, prefix=''):
          for key in g.keys():
              item = g[key]
              path = f'{prefix}/{key}'
              if isinstance(item, h5py.Dataset): # test for dataset
                  yield (path, item)
              elif isinstance(item, h5py.Group): # test for group (go down)
                  yield from h5py_dataset_iterator(item, path)
  
      for path, _ in h5py_dataset_iterator(hdf_file):
          yield path
          
  h5_f = h5py.File(weights_file, 'r')
  with open(weights_txt, 'w+') as f:
      for dset in traverse_datasets(h5_f):
          weight = h5_f[dset].value * 2**7
          weight = weight.flatten()
          weight = weight.astype(np.int32)
          f.write(h5_f[dset].name + " : ")
          f.write(str(weight.tolist()))
          f.write('\n\n')
          
  h5_f.close()
  ```

## 0x03 开启CMSIS

> 前提: 已经安装好 RT-Thread 所需要的运行环境

```shell
# windows
> pkgs --upgrade
# 开启 CMSIS
> menuconfig
> pkgs --update
> scons --target=mdk5

# linux
# 如果开启不了, 请执行
# (base) Mnist_CMSIS[master*] % source ~/.env/env.sh
(base) Mnist_CMSIS[master] % pkgs --upgrade 
(base) Mnist_CMSIS[master] % scons --menuconfig
(base) Mnist_CMSIS[master*] % pkgs --update
```

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719135513.png)

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719135536.png)

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719135602.png)

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719135637.png)

## 0x03 重构神经网络

通过调用CMSIS API, 实现网络重构, 此步骤需要一定的深度学习基础,

另外, 在重构的过程中, 均用`int`, 而非`float` 型

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200720160610.png)

感兴趣的可以阅读源文件

- ./Mnist_CMSIS/applications/main.c
- ./Mnist_CMSIS/applications/mnist_parameters.h

其他文件并无做任何改动

## 0x04 编译 & 烧录

- Windows 

  `MDK` or `scons` 一键编译一键烧录, 时间略久, 通过 Putty 观察输出情况

- Linux

  `scons` 编译, 通过`STM32 Cube Programmer` 烧录, `minicom` 观察输出情况

## 0x05 问题记录

### 5.1找不到 `arm_math.h`

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

### 5.2 scons 报错

> 但是文件均已经存在

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200719155954.png)

解决:

在`./Mnist_CMSIS/SConscript` 下面, 改为如图所示

![](https://gitee.com/lebhoryi/PicGoPictureBed/raw/master/img/20200720101650.png)