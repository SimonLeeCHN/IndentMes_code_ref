This repository stores the implementation of the codes involved in our thesis.

In order to show the results more intuitively, please see this software demo animation below. The process images are stored in the *demo* folder.

![animation](demo/animation.gif)

- SegmentationNetwork:

  The basic implementation of U-Net based on https://github.com/milesial/Pytorch-UNet.

  - Testing with trained models

    One of our trained models has been pre-stored in the *SegmentationNetwork* directory, named *MODEL.pth*, you can also find more pre-trained models in the *SegmentationNetwork/BackupModules* folder. When replacing the original model file, please take care to change the file name.
    
    Several Vickers indentation figures are stored in the *SegmentationNetwork/data* directory and you can easily experiment with them in Pycharm. For example, if you want to test *data/demo2.bmp*, execute the *predict.py* script in Pycharm with the following parameters:
    
    ```shell
    -i data\demo2.bmp --viz
    ```
    
    ![demo2](demo/demo2.png)

  - Train your own model

    You can train the model using the Vickers indentation image dataset your own.  We recommend using [labelme](https://github.com/wkentaro/labelme) to annotate the image data. Some useful scripting tools we made can be found in the *tools* folder.
    
    If you don't want to spend too much time and efforts on collecting data, you can try using the Vickers indentation image dataset we shared. Although it is not perfect at the moment, we hope it will help you. You can download it by visiting the link below:
    
    https://drive.google.com/drive/folders/1lu8MJZz_jBU9nPpHsYtuajSAb_fV5KHi?usp=sharing

- RoughLoc:

  The implementation of the rough location algorithm. Qt framework was used to build the entire software and Opencv was used to perform image processing operations. 

  The rough location algorithm is called via the signal slot. On some low-performance computers, it may take longer to run this part of the code. If deployed in the main thread, this will freezing the interface. Therefore, we recommend you deploy this part in a sub-thread and use Qt's signal slot mechanism to make the calls.

  - how to use

    We can use rough location algorithm by calling the following function through the signal slot:

    ```c++
    void IndentMesAlgorithm::slotImgIndentMes(double scale, cv::Mat inputMat, QRectF roi)
    ```
  
    Among them, *scale* is the zoom factor, *inputMat* is the image to be processed, *roi* is the region of interest (you can simply pass in an empty QRectF)

    It is worth noting that there are actually two algorithms in the *RoughLocAlgorithm.h/cpp* file, they are **M**aximum **I**nternal **C**onnected **F**iled (MICF) and **F**lood **F**ill (FF). The MICF method performs rough positioning by finding the largest connected domain. The FF method first identifies the approximate extent of the indentation by searching for the largest connected field. Next, the area is filled by the Flood Fill method using the brightest and darkest points within that range as a reference. You can choose the method you want by modifying the following code:

    ```c++
    switch(gSystemProperty.imaAIMSelect) //This is an enumeration, please refer to the definition in the header file to choose which method to use
        {
            case RoughLocAlgorithm::eAIM_MaxInternalConnectFiled:
            {
                _AIM_MaxInternalConnectFiled(_optMat,_indentRect); //Maximum Internal Connected Domain Method
                break;
            }
            case RoughLocAlgorithm::eAIM_FloodFill:
            {
                _AIM_FloodFill(_optMat,_indentRect); //Flood Fill method
                break;
            }
            default:
            {
    
            }
        }
    ```
  
- tools:

  This folder stores some useful scripting tools that can be used during training.

  - **augmentor**: This script will call [Augmentor](https://github.com/mdbloice/Augmentor) to augment the original pictures in the specified folder and the corresponding annotated pictures. For more instructions, please refer : https://github.com/mdbloice/Augmentor
  - **remove_name_space**: A simple batch script file. Using this script can easily remove the space characters in the file name after batch renaming to avoid errors when reading files.
  - **trans.py**: A script to batch convert [labelme](https://github.com/wkentaro/labelme) annotated json files into pictures. It converts json files in the folder in batches and extracts the corresponding images to the specified folders. 

