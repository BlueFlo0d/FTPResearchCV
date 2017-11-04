//
//  QTFFT.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/12.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "QTFFT.hpp"
#include "pffft.h"
#include <pthread.h>
using namespace cv;
typedef struct _fft_args{
    PFFFT_Setup *setup;
    const float *ptri;
    float *ptro;
    int i;
    int length;
    int dim;
} fft_args;
//#define QTFFT_PARALLEL
#ifdef QTFFT_PARALLEL
void *QTDFT_worker(void *args_raw){
    fft_args *arg = (fft_args *)args_raw;
    for (int i=arg->i; i<arg->i+arg->length; i++) {
        pffft_transform_ordered(arg->setup,arg->ptri+i*arg->dim , arg->ptro+i*arg->dim, NULL, PFFFT_FORWARD);
    }
    return 0;
}
void QTDFT_rows(const cv::Mat &input,cv::Mat &output){
    printf("Threads %d\n",getNumThreads());
    const int threads_num = getNumThreads();
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t)*threads_num);
        PFFFT_Setup *setup = pffft_new_setup(input.cols, PFFFT_COMPLEX);
    fft_args *args_list = (fft_args *)malloc(sizeof(fft_args)*threads_num);
    const int dim = input.cols*2;
    const float *ptri =input.ptr<float>(0);
    float *ptro = output.ptr<float>(0);
    for (int i=0; i<threads_num; i++) {
        args_list[i].setup=setup;
        args_list[i].ptri=ptri;
        args_list[i].ptro=ptro;
        args_list[i].length=input.rows/threads_num;
        args_list[i].dim=dim;
        args_list[i].i=input.rows/threads_num*i;
        pthread_create(threads+i, NULL, QTDFT_worker, args_list+i);
    }
    for (int i=0; i<threads_num; i++) {
        //int ret;
        pthread_join(threads[i], NULL);
    }
    /*
    for (int i=0; i<input.rows; i++) {
        pffft_transform_ordered(setup,ptri+i*dim , ptro+i*dim, NULL, PFFFT_FORWARD);
    }*/
    
    pffft_destroy_setup(setup);
}
#else
void QTDFT_rows(const cv::Mat &input,cv::Mat &output){
    PFFFT_Setup *setup = pffft_new_setup(input.cols, PFFFT_COMPLEX);
    const int dim = input.cols*2;
    const float *ptri =input.ptr<float>(0);
    float *ptro = output.ptr<float>(0);
    
     for (int i=0; i<input.rows; i++) {
     pffft_transform_ordered(setup,ptri+i*dim , ptro+i*dim, NULL, PFFFT_FORWARD);
     }
    
    pffft_destroy_setup(setup);
}

#endif
void QTIDFT_rows(const cv::Mat &input,cv::Mat &output){
    PFFFT_Setup *setup = pffft_new_setup(input.cols, PFFFT_COMPLEX);
    const int dim = input.cols*2;
    //float factor = 1/((float)input.cols);
    const float *ptri =input.ptr<float>(0);
    float *ptro = output.ptr<float>(0);
    for (int i=0; i<input.rows; i++) {
        pffft_transform_ordered(setup,ptri+i*dim , ptro+i*dim, NULL, PFFFT_BACKWARD);
        /*
        for (int j=0; j<input.cols; j++) {
            ptro[i*dim+j]*=factor;
        }*/
    }
    pffft_destroy_setup(setup);
}
