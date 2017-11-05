//
//  TestUtils.c
//  FTPResearch
//
//  Created by HongQiantan on 2017/9/26.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "TestUtils.hpp"

#include <stdlib.h>
#include <math.h>
using namespace cv;
void normalize_for_display(Mat &src,bool bypassed){
    float *ptr = src.ptr<float>(0);
    float min=0,max=0;
    int maxi=0,maxj=0;
    for (int i=0; i<SIZE_Y; i++) {
        for (int j=0; j<SIZE_X; j++) {
            float val = ptr[i*SIZE_X+j];
            //printf("%f\n",val);
            if (val<min) {
                min = val;
            }
            if (val>max) {
                max = val;
                maxi = i;
                maxj = j;
            }
        }
    }
    printf("min:%f,max:%f @ Y %d X %d\n",min,max,maxi,maxj);
    if (!bypassed) {
        float factor = 255.0/(max-min);
       // float factor = 255.0/7;
        for (int i=0; i<SIZE_Y; i++) {
            for (int j=0; j<SIZE_X; j++) {
                ptr[i*SIZE_X+j]=(ptr[i*SIZE_X+j]-min)*factor;
                //ptr[i*SIZE_X+j]=ptr[i*SIZE_X+j]*factor;
            }
        }
    }
    
}
float *generate_figure(int N0,int N1){
    float *X;
    size_t buffer_size  = N0 * N1 * sizeof(*X);
    X = (float *)malloc(buffer_size);
    size_t i, j;
    
    i = j = 0;
    for (i=0; i<N0; ++i) {
        for (j=0; j<N1; ++j) {
            float delta_x = (int)i-(int)N0/2;
            float delta_y = (int)j-(int)N1/2;
            float x = 0.5+0.5*sin(((float)j/N1)*M_PI*2*100+(delta_x*delta_x+delta_y*delta_y)/((float)N0*N0)*10);
            size_t idx = j+i*N1;
            X[idx] = x;
        }
    }
    save_bmpbuffer(N0, N1, X);
    return X;
}
float rect_grating(float x){
    //const int n = 16;
    //int nx = (int)(x/M_PI/2*n);
    //return 0.5+0.5*sin(nx*M_PI*2/n);
    float sx = sin(x);
    return 0.5+0.25*sx+0.25*sx*sx*sx;
}
void generate_figure_cv(){
    Mat cv_output = Mat(SIZE_Y, SIZE_X, CV_32F);
    Mat cv_file = imread("phase_original.jpg",CV_LOAD_IMAGE_GRAYSCALE);
    float *output = cv_output.ptr<float>(0);
    uchar *file = cv_file.ptr<uchar>(0);
    for (int i=0; i<SIZE_Y; i++) {
        for (int j=0; j<SIZE_X; j++) {
            //output[i*SIZE_X+j]=0.5+0.5*cos(((float)j/SIZE_X)*M_PI*2*FREQ_CENTER+((float)file[i*SIZE_X+j])/255.0);
            output[i*SIZE_X+j]=rect_grating(((float)j/SIZE_X)*M_PI*2*FREQ_CENTER+((float)i/SIZE_X)*M_PI*2*FREQ_CENTER_Y+((float)file[i*SIZE_X+j])/255.0*5);
            output[i*SIZE_X+j]*=output[i*SIZE_X+j];
        }
    }
    normalize_for_display(cv_output);
    imwrite("test.jpg", cv_output);
}
float *pad_image(float *input,int N0,int N1){
    float *output;
    size_t buffer_size  = N0 * (N1 + 2) * sizeof(*output);
    output = (float *)malloc(buffer_size);
    size_t i, j;
    i = j = 0;
    for (i=0; i<N0; ++i) {
        for (j=0; j<N1; ++j) {
            size_t idx = j+i*(N1+2);
            size_t idx_orig = j+i*N1;
            output[idx] = input[idx_orig];
        }
        output[i*(N1+2)+N1]=0.0;
        output[i*(N1+2)+N1+1]=0.0;
    }
    free(input);
    return output;
}
void save_bmpbuffer(int N0,int N1,float *X){
    unsigned char *bmp;
    size_t bmp_size = N0*N1*sizeof(*bmp);
    bmp = (unsigned char *)malloc(bmp_size);
    size_t i,j;
    for (i=0; i<N0; i++) {
        for (j=0; j<N1; j++) {
            size_t idx_bmp = j+i*N1;
            size_t idx_X = j+i*N1;
            bmp[idx_bmp]=((unsigned char)(X[idx_X]*256));
        }
    }
    Mat cvBitmap = Mat(N0, N1, CV_8UC1, bmp);
    imwrite("test.jpg",cvBitmap);
    free(bmp);
}
void process_fft(cv::Mat input){
    Mat planes[] = {Mat_<float>(input), Mat::zeros(input.size(), CV_32F)};
    Mat merged;
    merge(planes, 2, merged);
    clock_t t1=clock();
    dft(merged,merged);
    clock_t t2=clock();
    printf("DFT Time used:%f\n",((double)(t2-t1))/CLOCKS_PER_SEC);
    
    split(merged, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    imwrite("dftRe.jpg", planes[0]);
    imwrite("dftIm.jpg", planes[1]);
}
void process_ftp(cv::Mat input){
    Mat R = Mat(SIZE_Y,SIZE_X,CV_32F);
    clock_t t1 = clock();
    input = depthMap(input,R);//dummy
    clock_t t2 = clock();
    printf("Time:%fms\n",((float)(t2-t1)/CLOCKS_PER_SEC)*1000);
    normalize_for_display(input);
    imwrite("ftpPhase.jpg", input);
}
void process_uwp(cv::Mat input){
    Mat output = Mat(SIZE_Y,SIZE_X,CV_32F,Scalar(0));
    Mat R = Mat(SIZE_Y,SIZE_X,CV_32F);
    //Mat R = Mat(SIZE_Y,SIZE_X,CV_8UC1);
    clock_t t1 = clock();
    input = depthMap(input,R);
    
    unwrap_phase(input, output,R);
    clock_t t2 = clock();
    printf("Time:%fms\n",((float)(t2-t1)/CLOCKS_PER_SEC)*1000);
    normalize_for_display(output);
    imwrite("unwrapped.jpg",output);
    /*
    unwrap_phase(input, input);
    normalize_for_display(input);
    imwrite("unwrapped.jpg",input);*/
}
