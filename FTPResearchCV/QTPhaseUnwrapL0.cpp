//
//  QTPhaseUnwrapL0.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/7.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "QTPhaseUnwrap.hpp"
#include "TestUtils.hpp"
using namespace cv;
void unwrap_phase(cv::Mat input,cv::Mat &output){
    float *ptr = output.ptr<float>(0);
    float *ptr_src = input.ptr<float>(0);
    float previous_val = ptr_src[0];
    int *k_seed = (int *)malloc(SIZE_Y*sizeof(int));
    float *val_seed = (float *)malloc(SIZE_Y*sizeof(float));
    ptr[0]=ptr_src[0];
    val_seed[0]=ptr[0];
    int k=0;
    for (int i=1; i<SIZE_Y; i++) {
        float delta = ptr_src[i*SIZE_X]-previous_val;
        if (delta>M_PI) {
            k--;
        }
        else if (delta<-M_PI){
            k++;
        }
        /*
        if (k>10||k<-10) {
            printf("WARNING %d:Y %d X 0\n",k,i);
        }*/
        printf("%d:Y %d X 0\n",k,i);
        previous_val=ptr_src[i*SIZE_X];
        k_seed[i]=k;
        val_seed[i]=ptr[i*SIZE_X];
        ptr[i*SIZE_X]=ptr_src[i*SIZE_X]+((float)k*2)*M_PI;
    }
    for (int i=0; i<SIZE_Y;i++) {
        float previous_val = val_seed[i];
        int k=k_seed[i];
        for (int j=1; j<SIZE_X; j++) {
            float delta = ptr_src[i*SIZE_X+j]-previous_val;
            if (delta>M_PI) {
                k--;
            }
            else if (delta<-M_PI){
                k++;
            }
            
            if (k>10||k<-10) {
                printf("WARNING %d:Y %d X %d\n",k,i,j);
            }
            previous_val=ptr_src[i*SIZE_X+j];
            ptr[i*SIZE_X+j]=ptr_src[i*SIZE_X+j]+((float)k*2)*M_PI;
        }
    }
    free(k_seed);
    free(val_seed);
}
