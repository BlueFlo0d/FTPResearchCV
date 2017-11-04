//
//  QTFTP.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/2.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "QTFTP.hpp"
#include "TestUtils.hpp"
using namespace cv;
const int width = (int)(FREQ_CENTER*0.9);
/*
Mat window(int center,int width){
    Mat output = Mat(SIZE_X, SIZE_Y, CV_32F);
    float *buffer;
    float wid2 = width*width;
    buffer = (float*)malloc(sizeof(buffer)*SIZE_X*SIZE_Y);
    for (size_t i=0; i<SIZE_X; i++) {
        for (size_t j=0; j<SIZE_Y; j++) {
            int realj=(j>SIZE_Y/2)?(SIZE_Y-j):j;
            float dis2 = (i-center)*(i-center)+realj*realj;
            if(dis2<wid2){
                buffer[i*SIZE_X+j]=cos(dis2/wid2*M_PI/2);
            }
        }
    }
    return output;
}
 */
#define CHANNELS 2
void testcontinious(Mat input){
    printf("%d",input.channels());
    if(input.isContinuous())
        printf("continious.\n");
    else
        printf("not continious.\n");
}
/*
#define KILL_HARM_WID 50
void kill_harm(Mat &src){
    //2 Channels
    float *ptr=src.ptr<float>(0);
    int N_x = src.cols;
    int N_y = src.rows;
    for (int i=0; i<N_y; i++) {
        
        for (int j=FREQ_CENTER*2; j<N_x; j+=FREQ_CENTER) {
            ptr[(N_x*i+j)*2]=0.0;
            ptr[(N_x*i+j)*2+1]=0.0;
            for (int k=0; k<KILL_HARM_WID; k++) {
                ptr[(N_x*i+j+k)*2]=0.0;
                ptr[(N_x*i+j+k)*2+1]=0.0;
                ptr[(N_x*i+j-k)*2]=0.0;
                ptr[(N_x*i+j-k)*2+1]=0.0;
            }
        }
 
        for (int j=FREQ_CENTER*2; j<N_x; j++) {
            ptr[(N_x*i+j)*2]=0.0;
            ptr[(N_x*i+j)*2+1]=0.0;
        }
    }
}*/
Mat process_spectrum(Mat input_mat,int center,int width){
    
    Mat output_mat = Mat(SIZE_X, SIZE_Y, CV_32FC(2));
    //testcontinious(input_mat);
    //testcontinious(output_mat);
    float wid2 = width*width;
    float *input = input_mat.ptr<float>(0);
    float *output = output_mat.ptr<float>(0);
    for (size_t i=0; i<SIZE_Y; i++) {
        int reali = (i<SIZE_Y/2)?i:(SIZE_Y-i);
        for (size_t j=0; j<SIZE_X; j++) {
            int movedj = (j-center)%SIZE_X;
            int realj = (movedj<SIZE_X/2)?movedj:(SIZE_X-movedj);
            int dis2 = reali*reali+realj*realj;
            if (dis2<wid2) {
                float factor = exp(-((float)dis2)/(wid2/3));
                output[(movedj+i*SIZE_X)*2]=input[(j+i*SIZE_X)*2]*factor;
                output[(movedj+i*SIZE_X)*2+1]=input[(j+i*SIZE_X)*2+1]*factor;
            }
        }
    }
    return output_mat;
}
Mat calculate_arg(Mat input_mat){
    float *input = input_mat.ptr<float>(0);
    Mat output_mat = Mat(SIZE_X, SIZE_Y, CV_32F);
    float *output = output_mat.ptr<float>(0);
    for (size_t i=0; i<SIZE_Y; i++) {
        for (size_t j=0; j<SIZE_X; j++) {
            float y=input[(i*SIZE_X+j)*2+1];
            float x=input[(i*SIZE_X+j)*2];
            float val = cvFastArctan(y, x)/180.0*M_PI;
            output[i*SIZE_X+j]=val;
        }
    }
    return output_mat;
}

Mat calculate_arg_and_mag2(Mat input_mat){
    float *input = input_mat.ptr<float>(0);
    Mat output_mat = Mat(SIZE_X, SIZE_Y, CV_32FC2);
    float *output = output_mat.ptr<float>(0);
    for (size_t i=0; i<SIZE_Y; i++) {
        for (size_t j=0; j<SIZE_X; j++) {
            float y=input[(i*SIZE_X+j)*2+1];
            float x=input[(i*SIZE_X+j)*2];
            float val = cvFastArctan(y, x)/180.0*M_PI;
            output[(i*SIZE_X+j)*2]=val;
            output[(i*SIZE_X+j)*2+1]=x*x+y*y;
        }
    }
    return output_mat;
}
void merge_phase(Mat &phase,Mat delta,float factor=1.0,int diag=0){
    float *ptr=phase.ptr<float>(0);
    float *ptr_delta=delta.ptr<float>(0);
    for (int i=0; i<SIZE_Y; i++) {
        for (int j=0; j<SIZE_X; j++) {
            if(ptr_delta[(i*SIZE_X+j)*2+1]*factor>ptr[(i*SIZE_X+j)*2+1]){
                ptr[(i*SIZE_X+j)*2]=ptr_delta[(i*SIZE_X+j)*2];
                ptr[(i*SIZE_X+j)*2+1]=ptr_delta[(i*SIZE_X+j)*2+1]*factor;
                
                if (diag) {
                    printf("%d corrected:Y %d X %d\n",diag,i,j);
                }
            }
        }
    }
}
Mat _depthMap(Mat input){
    Mat planes[] = {Mat_<float>(input), Mat::zeros(input.size(), CV_32F)};
    Mat merged;
    merge(planes, 2, merged);
    dft(merged,merged,CV_DXT_FORWARD);
    Mat processed;
    processed =process_spectrum(merged, FREQ_CENTER, 100);
    //processed = merged;
    idft(processed,processed,DFT_SCALE);
    split(processed, planes);
    return calculate_arg(processed);
    //return planes[0];
}
Mat depthMap(Mat input,Mat &reliability){
    Mat planes[] = {Mat_<float>(input), Mat::zeros(input.size(), CV_32F)};
    Mat merged;
    merge(planes, 2, merged);
    dft(merged,merged,CV_DXT_FORWARD);
    //kill_harm(merged);
    //split(merged, planes);
    //imwrite("fft_re.jpg", planes[0]);
    Mat processed;
    processed =process_spectrum(merged, FREQ_CENTER, width);
    //processed = merged;
    idft(processed,processed,DFT_SCALE);
    Mat arg_and_mag2 = calculate_arg_and_mag2(processed);
    /*
    for (int i=1; i<4; i++) {
        Mat _processed = process_spectrum(merged, FREQ_CENTER, 180+i*40);
        idft(_processed,_processed,DFT_SCALE);
        merge_phase(arg_and_mag2, calculate_arg_and_mag2(_processed),180/(i*40+180.0));
    }*/
    split(arg_and_mag2, planes);
    reliability = planes[1];
    return planes[0];
}
