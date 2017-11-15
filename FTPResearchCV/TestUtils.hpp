//
//  TestUtils.h
//  FTPResearch
//
//  Created by HongQiantan on 2017/9/26.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#ifndef TestUtils_h
#define TestUtils_h
#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "QTFTP.hpp"
#include "QTPhaseUnwrap.hpp"
//#define TIC_TIME
#ifdef TIC_TIME
#define TIC clock_t tic_t1 = clock();
#define TOC clock_t tic_t2 = clock();printf("msg time %f ms\n",((float)(tic_t2-tic_t1))/CLOCKS_PER_SEC*1000);
#else
#define TIC
#define TOC
#endif
#define FREQ_CENTER_Y 0
void normalize_for_display(cv::Mat &src,bool bypassed=false);
float* generate_figure(int N0,int N1);//y,x
float* pad_image(float *input,int N0,int N1);//Will free the input
void save_bmpbuffer(int N0,int N1,float *X);
void process_fft(cv::Mat input);
void process_ftp(cv::Mat input);
void process_uwp(cv::Mat input);
void generate_figure_cv(void);
#endif /* TestUtils_h */
