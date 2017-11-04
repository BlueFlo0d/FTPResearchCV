//
//  QTPhaseUnwrap.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/5.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "QTPhaseUnwrap.hpp"
#include "TestUtils.hpp"
using namespace cv;
void fold_phase(Mat &src){
    const int N_y = src.rows;
    const int N_x = src.cols;
    float *ptr = src.ptr<float>(0);
    for (int i=0; i<N_y; i++) {
        for (int j=0; j<N_x; j++) {
            /*
            while (ptr[i*SIZE_X+j]>M_PI*2) {
                ptr[i*SIZE_X+j]-=2*M_PI;
            }*/
            while (ptr[i*N_x+j]>M_PI) {
                ptr[i*N_x+j]-=2*M_PI;
            }
            while (ptr[i*N_x+j]<-M_PI) {
                ptr[i*N_x+j]+=2*M_PI;
            }
            /*
            if (ptr[i*SIZE_X+j]>M_PI) {
                ptr[i*SIZE_X+j]=0.0;
            }*/
        }
    }
}
//output:N_x-- N_y--
void partial_x( const Mat &img, Mat &gx,int N_x,int N_y,int shift)
{
    const float *ptr_in = img.ptr<float>(0);
    float *ptr_out = gx.ptr<float>(0);
    if (shift == 1) {
        for (int i=0; i<N_y; i++) {
            for (int j=0; j<N_x-1; j++) {
                ptr_out[i*N_x+j]=ptr_in[i*N_x+j+1]-ptr_in[i*N_x+j];
            }
            ptr_out[i*N_x+N_x-1]=-ptr_in[i*N_x+N_x-1];
        }
    }
    else if (shift == -1) {
        for (int i=0; i<N_y; i++) {
            ptr_out[i*N_x]=ptr_in[i*N_x];
            for (int j=1; j<N_x; j++) {
                ptr_out[i*N_x+j]=ptr_in[i*N_x+j]-ptr_in[i*N_x+j-1];
            }
        }
    }
}
void partial_y( const Mat &img, Mat &gy,int N_x,int N_y,int shift)
{
    const float *ptr_in = img.ptr<float>(0);
    float *ptr_out = gy.ptr<float>(0);
    if (shift == 1) {
        for (int i=0; i<N_y-1; i++) {
            for (int j=0; j<N_x; j++) {
                ptr_out[i*N_x+j]=ptr_in[(i+1)*N_x+j]-ptr_in[i*N_x+j];
            }
        }
        for (int j=0; j<N_x; j++) {
            ptr_out[(N_y-1)*N_x+j]=-ptr_in[(N_y-1)*N_x+j];
        }
    }
    else if (shift == -1){
        for (int i=1; i<N_y; i++) {
            for (int j=0; j<N_x; j++) {
                ptr_out[i*N_x+j]=ptr_in[i*N_x+j]-ptr_in[(i-1)*N_x+j];
            }
        }
        for (int j=0; j<N_x; j++) {
            ptr_out[j]=ptr_in[j];
        }
    }
}
void eat_edge(Mat &src,int dis=1){
    float *ptr = src.ptr<float>(0);
    for (int i=0; i<SIZE_X; i++) {
        ptr[(dis-1)*SIZE_X+i]=0.0;
        ptr[(SIZE_Y-dis)*SIZE_X+i]=0.0;
    }
    for (int i=0; i<SIZE_Y; i++) {
        ptr[i*SIZE_X+dis-1]=0.0;
        ptr[i*SIZE_X+SIZE_X-dis]=0.0;
    }
}

void unwrap_phase(Mat input,Mat &output){
    
    const int dim=SIZE_X;
    float *ptr = output.ptr<float>(0);
    
    
    clock_t t1 = clock();
    dct(input, output);
    
    for (int i=0; i<SIZE_Y; i++) {
        for (int j=0; j<SIZE_X; j++) {
            if (i==0&&j==0) {
                
            }
            else{
                float factor=2*cos(i*M_PI/dim)+2*cos(j*M_PI/dim)-4;
                ptr[i*dim+j]=ptr[i*dim+j]*factor;
            }
        }
    }
    
    idct(output, output);
    clock_t t2 = clock();
    printf("DCT Laplacian time: %fms\n",((float)(t2-t1)/CLOCKS_PER_SEC*1000));
    /*
    clock_t t1 = clock();
    Laplacian(input, output, CV_32F,3);
    clock_t t2 = clock();
    printf("Direct Laplacian time: %fms\n",((float)(t2-t1)/CLOCKS_PER_SEC*1000));*/
    //Sobel(output, output, CV_32F, 2, 2,5);
    //eat_edge(output);
    //fold_phase(output);
    
    /*
    Mat gx=Mat(dim, dim, CV_32F);
    Mat gy=Mat(dim, dim, CV_32F);
    Mat ggx=Mat(dim, dim, CV_32F);
    Mat ggy=Mat(dim, dim, CV_32F);
    partial_x(input, gx,dim,dim,1);
    partial_y(input, gy,dim,dim,1);
    fold_phase(gx);
    fold_phase(gy);
    partial_x(gx, ggx,dim, dim,-1);
    partial_y(gy, ggy,dim, dim,-1);
    output = ggx+ggy;
    eat_edge(output);
    //fold_phase(output);
    */
    fold_phase(output);
    dct(output, output);
    //dim-=2;
    
    for (int i=0; i<dim; i++) {
        for (int j=0; j<dim; j++) {
            if (i==0&&j==0) {
                
            }
            else{
                float factor=2*cos(i*M_PI/dim)+2*cos(j*M_PI/dim)-4;
                ptr[i*dim+j]=ptr[i*dim+j]/factor;
            }
        }
    }
    //ptr[0]=-(ptr[1]+ptr[dim]-ptr[0])/2;
    idct(output, output);
    //normalize_for_display(output);
}
