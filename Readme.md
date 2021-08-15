This code repository stores the implementation of the key codes involved in our thesis.

In order to show the results of our work more intuitively, please see this software demo animation below.

![动画](demo/%E5%8A%A8%E7%94%BB.gif)

If you want to see a clearer picture of the process, please refer to the demo folder.

- SegmentationNetwork:

  The basic implementation of U-Net based on https://github.com/milesial/Pytorch-UNet.

  - Test on our trained model

    One of our trained models has been pre-stored in the SegmentationNetwork directory, named MODEL.pth, you can also find more pre-trained models in the segmentationNetwork/BackupModules folder, just replace the original model file with them, pay attention to modify the file name.
    
    There are several Vickers indentation images stored in the SegmentationNetwork/data directory, and you can easily use them in Pycharm for experimentation. For example, if you want to test data/demo2.bmp, just execute the predict.py script in Pycharm with the following parameters:
    
    ```shell
    -i data\demo2.bmp --viz
    ```
    
    ![demo2](demo/demo2.png)

  - Train your own model

    You can use the Vickers hardness image data set collected by yourself. We recommend using labelme to label the image data. If you don't want to spend too much time and energy on the collection of Vickers image data, you can try to use the Vickers hardness image data set we shared. Although it is not perfect at the moment, I hope it can help you. You can download it by visiting the link below:
    
    https://drive.google.com/drive/folders/1lu8MJZz_jBU9nPpHsYtuajSAb_fV5KHi?usp=sharing

- RoughLoc:

  The basic realization of the coarse positioning algorithm, in the actual application, we use the Qt framework to build the entire software, and use Opencv to perform related image operations. You can easily run the code through the basic Qt installation and opencv call.

  > If you are having difficulty using VS2019 to compile and generate 32-bit Opencv4 libraries, please refer to my blog, which has detailed usage methods: https://blog.csdn.net/sements/article/details/108410470

  You can see that some Qt-related libraries are used in the algorithm, and the entire algorithm is called through signal slots. This is due to the fact that during actual deployment, we found that some computers with lower performance will take more time to run this part of the code. If deployed in the main thread, this will cause the user interface to freeze. Therefore, we recommend that you run it in a child thread.

  - how to use

    We can use coarse positioning algorithm by calling the following function through the signal slot:

    ```c++
    void IndentMesAlgorithm::slotImgIndentMes(double scale, cv::Mat inputMat, QRectF roi)
    ```

    Among them, scale is the zoom factor, inputMat is the image to be processed, roi is the region of interest (you can simply pass in an empty QRectF)

    It is worth noting that there are actually two algorithms in the RoughLocAlgorithm.h/cpp file, they are Max Internal Connect Filed (MICF) and Flood Fill (FF). The MICF method performs rough positioning by finding the largest connected domain. The FF method first searches for the approximate range of the indentation by searching for the largest connected domain, and uses the brightest and darkest points in this range as a reference to fill the area through the Flood Fill method to complete the rough positioning operation. You can choose it in this section:

    ```c++
    switch(gSystemProperty.imaAIMSelect) //This is an enumeration, please refer to the definition in the header file to choose which method to use
        {
            case RoughLocAlgorithm::eAIM_MaxInternalConnectFiled:
            {
                _AIM_MaxInternalConnectFiled(_optMat,_indentRect); //Max InternalConnect Domain Method
                break;
            }
            case RoughLocAlgorithm::eAIM_FloodFill:
            {
                _AIM_FloodFill(_optMat,_indentRect); //Diffuse water filling method
                break;
            }
            default:
            {
    
            }
        }
    ```

- tools:

  This folder stores some data annotations and gadgets that may be used during training

  - augmentor: This script will call Augmentor to augment the original pictures in the specified folder and the corresponding annotated pictures. For how to use Augmentor, please refer to: https://github.com/mdbloice/Augmentor
  - remove_name_space: A simple batch file. When you need to use the enhanced image data to train your network, you may need to batch rename all training images. Using this script can easily remove the space characters in the file name after batch renaming to avoid errors when reading the file.
  - trans.py: A script to batch convert labelme labeled json data into pictures. It uses the original labelme_json_to_dataset in the labelme library as the core, converts the json files in the folder in batches, and extracts the corresponding pictures to their respective folders. For more information about it, please refer to my blog: https://blog.csdn.net/sements/article/details/110135137

---

本代码仓库中存储的是我们论文中所涉及到的关键代码实现。

为了更直观的展示我们工作的成果，请看下面这个软件演示动画。

![动画](demo/%E5%8A%A8%E7%94%BB.gif)

如果你想要查看更加清晰的过程图片，请查阅 demo 文件夹。

- SegmentationNetwork：

  基于 https://github.com/milesial/Pytorch-UNet 的 U-Net 基本实现，关于。

  - 在我们训练好的模型上进行测试

    在 SegmentationNetwork 目录下已经预先存放好了一个我们训练好的模型，名为 MODEL.pth，你还可以在 segmentationNetwork/BackupModules 文件夹下找到更多预训练好的模型，只需用他们替换原本的 MODEL.pth 即可，注意修改文件名。

    在 SegmentationNetwork/data 目录下存放了几张维氏压痕图像，你可以在 Pycharm 中很方便的使用他们进行试验。例如你想要测试 data/demo2.bmp，只需在 Pycharm 中执行 predict.py 脚本附带以下参数即可：

    ```shell
    -i data\demo2.bmp --viz
    ```

    ![demo2](demo/demo2.png)

  - 训练自己的模型

    你可以使用自己采集的维氏硬度图像数据集，我们推荐使用 labelme 对图像数据进行标注。如果你不想花太多的时间和精力在维氏图像数据的采集工作上，你可以尝试使用我们分享的维氏硬度图像数据集。尽管它目前并不完善，但希望它能助你一臂之力，你可以访问下面的链接进行下载：

    https://drive.google.com/drive/folders/1lu8MJZz_jBU9nPpHsYtuajSAb_fV5KHi?usp=sharing

- RoughLoc：

  粗定位算法的基本实现，在实际的应用程序中，我们使用Qt框架对整个软件进行了搭建，并使用Opencv进行了相关的图像操作。通过基本的Qt安装以及opencv调用你可以很轻松的运行这其中的代码。

  > 如果你对于使用VS2019编译生成32位Opencv4库感到困难，请参考我的博客，里面有详细的使用方法：https://blog.csdn.net/sements/article/details/108410470

  你可以看到算法中用到了部分Qt相关的库，并且整个算法是通过信号槽的方式进行调用的。这是由于在实际部署时我们发现，部分性能较低的计算机运行这部分代码将比较耗时，若部署在主线程中，这将造成用户界面的卡顿，因此我们建议你在使用过程中将其部署在子线程中来运行。

  - 如何使用

    通过信号槽的方式调用下面的函数便可以使用我们的粗定位算法：

    ```c++
    void IndentMesAlgorithm::slotImgIndentMes(double scale, cv::Mat inputMat, QRectF roi)
    ```

    其中，scale 是缩放因子，inputMat 是待处理的图像，roi 是感兴趣区域（你可以简单的传入一个空 QRectF）

    值得注意的是，在 RoughLocAlgorithm.h/cpp 文件中实际包含两种算法，他们分别是 Max Internal Connect Filed（MICF）以及 Flood Fill（FF）。MICF方法通过寻找最大连通域的方式进行粗定位。FF方法首先通过寻找最大连通域的方式搜寻到压痕的大致范围，以这个范围内的最亮与最暗点为基准，通过 Flood Fill 方式对区域进行填充，完成粗定位操作。你可以在这一部分对其进行选择：

    ```c++
    switch(gSystemProperty.imaAIMSelect)                                    //此处为枚举量，请参考头文件中的定义以选择使用哪种方法
        {
            case RoughLocAlgorithm::eAIM_MaxInternalConnectFiled:
            {
                _AIM_MaxInternalConnectFiled(_optMat,_indentRect);              //最大内联通域方法
                break;
            }
            case RoughLocAlgorithm::eAIM_FloodFill:
            {
                _AIM_FloodFill(_optMat,_indentRect);                            //漫水填充法
                break;
            }
            default:
            {
    
            }
        }
    ```

- tools：

  这个文件夹下存放了一些数据标注以及训练时可能用到的小工具

  - augmentor：这个脚本将调用 Augmentor 对指定文件夹下的原始图片以及对应的标注图片进行数据增强，关于 Augmentor 的使用方法请参考：https://github.com/mdbloice/Augmentor
  - remove_name_space：一个简单的批处理文件，当你需要使用增强后的图片数据对你的网络进行训练时，你可能会需要对所有训练图片进行批量重命名。使用这个脚本可以方便的去除批量重命名后文件名称中的空格字符，避免文件读取时出错。
  - trans.py：批量转换labelme标注json数据为图片的脚本。它以调用labelme库中原有的 labelme_json_to_dataset 为核心，批量将文件夹中的json文件转换，并抽取对应图片至各自文件夹 。关于它的更多介绍可以参考我的博客：https://blog.csdn.net/sements/article/details/110135137
