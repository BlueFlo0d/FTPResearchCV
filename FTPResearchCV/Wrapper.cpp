//
//  Wrapper.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/11/5.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "Wrapper.hpp"
#include "QTFTP.hpp"
#include "QTPhaseUnwrap.hpp"
using namespace cv;
int FREQ_CENTER = 100;
int ACCEPTANCE = 20;
void convert2absolute(cv::Mat &input,float factor){
    const int SIZEY=input.rows,SIZEX=input.cols;
    float *ptr=input.ptr<float>(0);
#pragma omp parallel for
    for (int i=0; i<SIZEY; i++) {
        for (int j=0; j<SIZEX; j++) {
            ptr[i*SIZEX+j]*=factor;
        }
    }
}
cv::Mat depth_map(cv::Mat &img0,cv::Mat &img1,cv::Mat &img2){
    Mat _img0,_img1,_img2,output,uwp;
    img0.convertTo(_img0, CV_32F);
    img1.convertTo(_img1, CV_32F);
    img2.convertTo(_img2, CV_32F);
    add(_img0, _img2, _img0);
    invert(_img0, _img0);
    add(_img1, _img0, _img1);
    Mat R = Mat(img1.rows,img1.cols,CV_32F,Scalar(0));
    uwp = Mat(img1.rows,img1.cols,CV_32F,Scalar(0));
    output=depthMap(_img1, R);
    unwrap_phase(output, uwp, R);
    return uwp;
}
