#include "RoughLocAlgorithm.h"
#include "QCoreApplication"



/******SLOT******/


/*
 *  对图像进行智能优化
 */
void IndentMesAlgorithm::slotImgAutoOpt(cv::Mat inMat)
{
    //转灰度通道节省空间
    if(inMat.channels() == 3)
    {
        cv::cvtColor(inMat,inMat,cv::COLOR_BGR2GRAY);
    }
    else if(inMat.channels() == 4)
    {
        cv::cvtColor(inMat,inMat,cv::COLOR_BGRA2GRAY);
    }

//    //直方图规范化
//    cv::normalize(inMat,inMat,0,255,cv::NORM_MINMAX);

//    //直方图均衡
//    cv::equalizeHist(inMat,inMat);

    //限制对比度的自适应阈值均衡化 CLAHE
    double _clipLimit = gSystemProperty.imaConfig.imaCLAHEclipLimit;
    int _tileSize = gSystemProperty.imaConfig.imaCLAHEtileSize;
    auto _clahe = cv::createCLAHE(_clipLimit,cv::Size(_tileSize,_tileSize));
    _clahe->apply(inMat,inMat);

    emit sigImgAutoOptFin(inMat);
}

/*
 *  寻找最大内部连通域方法
 *
 *  optMat:输入图像，8uc1
 *  bbOut:输出包围框
 */
void _AIM_MaxInternalConnectFiled(cv::Mat optMat,QRectF& bbOut)
{
    if(optMat.channels() != 1)
    {
        qDebug()<<"_AIM_MaxInternalConnectFiled optMat must be CV_8UC1";
        return;
    }

    //中值滤波
    cv::medianBlur(optMat,optMat,3);

    //otsu反转阈值，optMat中压痕为暗部，_otsuInvMat中压痕为亮部
    cv::Mat _otsuInvMat;
    cv::threshold(optMat,_otsuInvMat,0,255,cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    if(gSystemProperty.imaShowProcessView)
    {
        QByteArray _ba = QStringLiteral("1-二值化图像").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_otsuInvMat);
    }

    //腐蚀  封闭压痕边沿，避免边上的一些划痕被联通
    int _erodeKsize = gSystemProperty.imaConfig.imaErodeSize;
    auto _erodeKernel = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(_erodeKsize,_erodeKsize));
    cv::erode(_otsuInvMat,_otsuInvMat,_erodeKernel);
    if(gSystemProperty.imaShowProcessView)
    {
        QByteArray _ba = QStringLiteral("2-缝隙填补").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_otsuInvMat);
    }

    //找外侧轮廓
    std::vector<std::vector<cv::Point>> _contoursOutside;
    cv::findContours(_otsuInvMat,_contoursOutside,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);

    //查找_contoursOutside中最大轮廓
    double _maxArea = 0;
    int _maxAreaIndex = 0;
    if(0 == _contoursOutside.size())                                                        //若没有轮廓，直接返回
    {
        qDebug()<<"_AIM_MaxInternalConnectFiled, 0 == _contoursOutside.size()";
        return;
    }
    for(int _index = int(_contoursOutside.size() - 1) ; _index >= 0 ; _index--)
    {
        double _area = fabs(cv::contourArea(_contoursOutside[_index]));
        if(_area >= _maxArea)
        {
            //记录最大轮廓
            _maxArea = _area;
            _maxAreaIndex = _index;
        }
    }
    auto _maxContour = _contoursOutside[_maxAreaIndex];

    //绘制最大轮廓
    if(gSystemProperty.imaShowProcessView)
    {
        cv::Mat _matMaxFiledShow;
        cv::cvtColor(optMat,_matMaxFiledShow,cv::COLOR_GRAY2BGR);
        cv::drawContours(_matMaxFiledShow,_contoursOutside,_maxAreaIndex,cv::Scalar(255,0,0),-1);

        QByteArray _ba = QStringLiteral("3-最大连通域").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_matMaxFiledShow);
    }

    /*
     *  求最大内轮廓的外接矩形
     */
    auto _boundingRect = cv::boundingRect(_maxContour);
    bbOut.moveTo(_boundingRect.x,_boundingRect.y);
    bbOut.setWidth(_boundingRect.width);
    bbOut.setHeight(_boundingRect.height);

    return;
}

/*
 *  自动压痕测量 - 漫水填充法
 *  https://docs.opencv.org/2.4.9/modules/imgproc/doc/miscellaneous_transformations.html#floodfill
 *
 *  optMat:输入图像，8uc1
 *  bbOut:输出包围框
 */
void _AIM_FloodFill(cv::Mat optMat,QRectF& bbOut)
{
    if(optMat.channels() != 1)
    {
        qDebug()<<"_AIM_FloodFill optMat must be CV_8UC1";
        return;
    }

    //中值滤波
    cv::medianBlur(optMat,optMat,3);

    /*
     *  寻找最大暗部连通域上的最暗与最亮点
     */
    double _minLuxVal = 255,_maxLuxVal = 0;
    cv::Point _minLuxLoc,_maxLuxLoc;

    //otsu反转阈值，optMat中压痕为暗部，_otsuInvMat中压痕为亮部
    cv::Mat _otsuInvMat;
    cv::threshold(optMat,_otsuInvMat,0,255,cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    if(gSystemProperty.imaShowProcessView)
    {
        QByteArray _ba = QStringLiteral("1-二值化图像").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_otsuInvMat);
    }

    //腐蚀  封闭压痕边沿，避免边上的一些划痕被联通
    int _erodeKsize = gSystemProperty.imaConfig.imaErodeSize;
    auto _erodeKernel = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(_erodeKsize,_erodeKsize));
    cv::erode(_otsuInvMat,_otsuInvMat,_erodeKernel);
    if(gSystemProperty.imaShowProcessView)
    {
        QByteArray _ba = QStringLiteral("2-缝隙填补").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_otsuInvMat);
    }

    //找外侧轮廓
    std::vector<std::vector<cv::Point>> _contoursOutside;
    cv::findContours(_otsuInvMat,_contoursOutside,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);

    //查找_contoursOutside中最大轮廓
    double _maxArea = 0;
    int _maxAreaIndex = 0;
    if(0 == _contoursOutside.size())                                                        //若没有轮廓，直接返回
    {
        qDebug()<<"_AIM_FloodFill, 0 == _contoursOutside.size()";
        return;
    }
    for(int _index = int(_contoursOutside.size() - 1) ; _index >= 0 ; _index--)
    {
        double _area = fabs(cv::contourArea(_contoursOutside[_index]));
        if(_area >= _maxArea)
        {
            //记录最大轮廓
            _maxArea = _area;
            _maxAreaIndex = _index;
        }
    }
    auto _maxContour = _contoursOutside[_maxAreaIndex];

    //标记最大轮廓
    cv::Mat _matMaxFiledMask = cv::Mat::zeros(_otsuInvMat.size(),CV_8UC1);
    cv::drawContours(_matMaxFiledMask,_contoursOutside,_maxAreaIndex,cv::Scalar(255),-1);

    //绘制最大轮廓
    if(gSystemProperty.imaShowProcessView)
    {
        cv::Mat _matMaxFiledShow;
        cv::cvtColor(optMat,_matMaxFiledShow,cv::COLOR_GRAY2BGR);
        cv::drawContours(_matMaxFiledShow,_contoursOutside,_maxAreaIndex,cv::Scalar(255,0,0),-1);

        QByteArray _ba = QStringLiteral("3-最大连通域").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_matMaxFiledShow);
    }

    //查找最大轮廓内的最亮最暗点
    for(int i = 0 ; i < _otsuInvMat.rows ; i++)
    {
        for(int j = 0 ;j < _otsuInvMat.cols ; j++)
        {
            //检查当前点是否在最大轮廓内
            auto _maskPoint = _matMaxFiledMask.at<uchar>(i,j);
            if(255 == _maskPoint)
            {
                auto _pointVal = optMat.at<uchar>(i,j);

                if(_pointVal >= _maxLuxVal)
                {
                    _maxLuxVal = _pointVal;
                    _maxLuxLoc = cv::Point(j,i);        //注意Point赋值时行列与前面的颠倒
                }
                else if(_pointVal <= _minLuxVal)
                {
                    _minLuxVal = _pointVal;
                    _minLuxLoc = cv::Point(j,i);        //注意Point赋值时行列与前面的颠倒
                }
            }
        }
    }

    //绘制最亮最暗点位置
    if(gSystemProperty.imaShowProcessView)
    {
        cv::Mat _matMinMaxLuxLoc;
        cv::cvtColor(optMat,_matMinMaxLuxLoc,cv::COLOR_GRAY2BGR);
        cv::circle(_matMinMaxLuxLoc,_minLuxLoc,5,cv::Scalar(255,0,0));
        cv::circle(_matMinMaxLuxLoc,_maxLuxLoc,5,cv::Scalar(0,0,255));

        QByteArray _ba = QStringLiteral("4-连通域中最暗点(蓝)与最亮点(红)").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_matMinMaxLuxLoc);
    }


    /*
     * 漫水填充
     */

    //初始化mask
    cv::Mat _matFloodFillMask;
    _matFloodFillMask.create(optMat.rows + 2 , optMat.cols + 2 , CV_8UC1);      //官方要求，mask比原图的宽、高各大2
    _matFloodFillMask = cv::Scalar::all(0);

    //floodFill
    cv::Mat _floodFillSrc = optMat.clone();
    cv::Rect _floodFillRect;
    int _floodFillFlags = 8 | (255 << 8) | cv::FLOODFILL_FIXED_RANGE | cv::FLOODFILL_MASK_ONLY;     //8 连通域搜索，在mask中绘制时填255， 只在mask中绘制
    int _loDiff = gSystemProperty.imaConfig.imaFloodFillLoDiff , _highDiff = gSystemProperty.imaConfig.imaFloodFillHighDiff;
    cv::floodFill(_floodFillSrc,_matFloodFillMask,_minLuxLoc,cv::Scalar::all(255),&_floodFillRect,cv::Scalar::all(_loDiff),cv::Scalar::all(_highDiff),_floodFillFlags);
    if(gSystemProperty.imaShowProcessView)
    {
        QByteArray _ba = QStringLiteral("5-以最暗点递归搜索到的区域").toLocal8Bit();
        std::string _stdString = _ba.data();
        cv::imshow(_stdString,_matFloodFillMask);
    }

    /*
     *  设置输出矩形
     */
    bbOut.moveTo(_floodFillRect.x,_floodFillRect.y);
    bbOut.setWidth(_floodFillRect.width);
    bbOut.setHeight(_floodFillRect.height);

    return;
}

/*
 *  传入图像要放大的倍率，要搜寻的图像mat以及roi
 *  处理完毕发射信号，回报处理完毕的图像、压痕位置矩形框
 */
void IndentMesAlgorithm::slotImgIndentMes(double scale, cv::Mat inputMat, QRectF roi)
{
    cv::Mat _optMat;                //操作mat
    QRectF _indentRect;             //检测到压痕在roi中的位置

    //清理所有imshow window
    cv::destroyAllWindows();

    //转灰度通道节省空间
    if(inputMat.channels() == 3)
    {
        cv::cvtColor(inputMat,inputMat,cv::COLOR_BGR2GRAY);
    }
    else if(inputMat.channels() == 4)
    {
        cv::cvtColor(inputMat,inputMat,cv::COLOR_BGRA2GRAY);
    }

    _optMat = inputMat.clone();

    //检测是否有ROI
    if(!roi.isEmpty())
    {
        _optMat = _optMat(cv::Range(int(roi.top()),int(roi.bottom())),cv::Range(int(roi.left()),int(roi.right())));
    }

    /*
     *  搜索
     */
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

    /*
     * 外接矩形修正
     * 压痕宽高应当相近,若寻找到的宽、高差异过大 ，说明可能内轮廓查找时有缝隙，导致与外部表面缝隙联通
     * 取宽、高中最短的值作为差异较大的修正
     */
    auto _rectMinSideLength = fmin(_indentRect.width(),_indentRect.height());
    if(abs(_indentRect.width() - _indentRect.height()) >= 5)
    {
        _indentRect.setWidth(_rectMinSideLength);
        _indentRect.setHeight(_rectMinSideLength);
    }
//    if(_indentRect.width() >= (_rectMinSideLength * 1.2))                       //过大判断系数
//    {
//        _indentRect.setWidth(_rectMinSideLength);
//    }else if(_indentRect.height() >= (_rectMinSideLength * 1.2))
//    {
//        _indentRect.setHeight(_rectMinSideLength);
//    }

    /*
     * 回报压痕矩形在整体图片中的位置
     * 目前找到的压痕矩形仅为在roi中的位置（如果有roi）
     * 需要解算回其在原图中的位置
     * xy基于roi进行偏移
     * 宽高基于scale进行缩放
     */
    if(!roi.isEmpty())
    {
        _indentRect.moveLeft(_indentRect.x() + roi.x());
        _indentRect.moveTop(_indentRect.y() + roi.y());
    }
    _indentRect.setWidth(double(_indentRect.width() / scale));
    _indentRect.setHeight(double(_indentRect.height() / scale));

    emit sigImgIndentMesFin(_indentRect);

}
