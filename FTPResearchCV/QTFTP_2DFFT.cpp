//
//  QTFTP.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/2.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include "QTFTP.hpp"
#include "TestUtils.hpp"
#include "QTFFT.hpp"

using namespace cv;
const int width = (int)(FREQ_CENTER*0.9);
#define CHANNELS 2
void testcontinious(Mat input){
    printf("%d",input.channels());
    if(input.isContinuous())
        printf("continious.\n");
    else
        printf("not continious.\n");
}

Mat process_spectrum(Mat input_mat,int centerx,int centery,int width){
    const int SIZEY=input_mat.rows,SIZEX=input_mat.cols;
    Mat output_mat = Mat(SIZEX, SIZEY, CV_32FC(2));
    //testcontinious(input_mat);
    //testcontinious(output_mat);
    float wid2 = width*width;
    float *input = input_mat.ptr<float>(0);
    float *output = output_mat.ptr<float>(0);
    #pragma omp parallel for
    for (int i=0; i<SIZEY; i++) {
        int movedi = (i-centery+SIZEY)%SIZEY;
        int reali = (i<SIZEY/2)?i:(SIZEY-i);
        for (int j=0; j<SIZEX; j++) {
            int movedj = (j-centerx+SIZEX)%SIZEX;
            int realj = (movedj<SIZEX/2)?movedj:(SIZEX-movedj);
            int dis2 = reali*reali+realj*realj;
            if (dis2<wid2) {
                float factor = exp(-((float)dis2)/(wid2/3));
                output[(movedj+movedi*SIZEX)*2]=input[(j+i*SIZEX)*2]*factor;
                output[(movedj+movedi*SIZEX)*2+1]=input[(j+i*SIZEX)*2+1]*factor;
            }
        }
    }
    return output_mat;
}
Mat calculate_arg(Mat input_mat){
    const int SIZEY=input_mat.rows,SIZEX=input_mat.cols;
    float *input = input_mat.ptr<float>(0);
    Mat output_mat = Mat(SIZEX, SIZEY, CV_32F);
    float *output = output_mat.ptr<float>(0);
    #pragma omp parallel for
    for (size_t i=0; i<SIZEY; i++) {
        for (size_t j=0; j<SIZEX; j++) {
            float y=input[(i*SIZEX+j)*2+1];
            float x=input[(i*SIZEX+j)*2];
            float val = cvFastArctan(y, x)/180.0*M_PI;
            output[i*SIZEX+j]=val;
        }
    }
    return output_mat;
}

Mat calculate_arg_and_mag2(Mat input_mat){
    const int SIZEY=input_mat.rows,SIZEX=input_mat.cols;
    float *input = input_mat.ptr<float>(0);
    Mat output_mat = Mat(SIZEX, SIZEY, CV_32FC2);
    float *output = output_mat.ptr<float>(0);
    #pragma omp parallel for
    for (size_t i=0; i<SIZEY; i++) {
        for (size_t j=0; j<SIZEX; j++) {
            float y=input[(i*SIZEX+j)*2+1];
            float x=input[(i*SIZEX+j)*2];
            float val = cvFastArctan(y, x)/180.0*M_PI;
            output[(i*SIZEX+j)*2]=val;
            output[(i*SIZEX+j)*2+1]=x*x+y*y;
        }
    }
    return output_mat;
}
void merge_phase(Mat &phase,Mat delta,float factor=1.0,int diag=0){
    const int SIZEY=phase.rows,SIZEX=phase.cols;
    float *ptr=phase.ptr<float>(0);
    float *ptr_delta=delta.ptr<float>(0);
    #pragma omp parallel for
    for (int i=0; i<SIZEY; i++) {
        for (int j=0; j<SIZEX; j++) {
            if(ptr_delta[(i*SIZEX+j)*2+1]*factor>ptr[(i*SIZEX+j)*2+1]){
                ptr[(i*SIZEX+j)*2]=ptr_delta[(i*SIZEX+j)*2];
                ptr[(i*SIZEX+j)*2+1]=ptr_delta[(i*SIZEX+j)*2+1]*factor;
                
                if (diag) {
                    printf("%d corrected:Y %d X %d\n",diag,i,j);
                }
            }
        }
    }
}
inline void magmax(Mat &input,int* x,int *y,float *val){
    Mat planes[2];
    split(input, planes);
    pow(planes[0], 2.0, planes[0]);
    pow(planes[1], 2.0, planes[0]);
    add(planes[0], planes[1], planes[0]);
    Point maxpos;
    double val_;
    minMaxLoc(planes[0],NULL,&val_,NULL,&maxpos);
    *val = (float)val_;
    *x = maxpos.x;
    *y = maxpos.y;
}
void tune_springe(Mat &input,int *centerx,int *centery){
    float valu,vald;
    int xu,xd,yu,yd;
    Rect upper_area=Rect(FREQ_CENTER-ACCEPTANCE,0,ACCEPTANCE*2,ACCEPTANCE);
    Mat buffer = Mat(input.rows, input.cols, CV_32FC2);
    input(upper_area).copyTo(buffer);
    magmax(buffer, &xu, &yu,&valu);
    Rect down_area=Rect(FREQ_CENTER-ACCEPTANCE,
                        input.rows-ACCEPTANCE,ACCEPTANCE*2,ACCEPTANCE);
    input(down_area).copyTo(buffer);
    magmax(buffer, &xd, &yd, &vald);
    if (valu>vald) {
        *centerx=FREQ_CENTER-ACCEPTANCE+xu;
        *centery=yu;
    }
    else{
        *centerx=FREQ_CENTER-ACCEPTANCE+xd;
        *centery=yu-input.rows;
    }
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
    int centerx,centery;
    tune_springe(merged, &centerx, &centery);
    processed =process_spectrum(merged, centerx,centery, width);
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

