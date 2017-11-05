//
//  Wrapper.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/11/5.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "Wrapper.hpp"

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
