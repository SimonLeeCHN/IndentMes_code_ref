/*
 *  硬度测量计算线程
 */
#ifndef ROUGHLOCALGORITHM_H
#define ROUGHLOCALGORITHM_H

#include <QObject>
#include "CommonResource.h"

class RoughLocAlgorithm : public QObject
{
    Q_OBJECT
public:
    enum EnumAIMAlgorithm                       //算法枚举
    {
        eAIM_MaxInternalConnectFiled = 0 ,      //寻找最大内部连通域方法
        eAIM_FloodFill = 1                      //漫水填充法
    };

public slots:
    void slotImgAutoOpt(cv::Mat inMat);
    void slotImgIndentMes(double scale,cv::Mat inputMat,QRectF roi);

private:

signals:
    void sigImgAutoOptFin(cv::Mat outMat);
    void sigImgIndentMesFin(QRectF boudingRect);
    void sigAutoFocusEnd();

};

#endif // ROUGHLOCALGORITHM_H
