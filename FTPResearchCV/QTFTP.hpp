//
//  QTFTP.hpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/2.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#ifndef QTFTP_hpp
#define QTFTP_hpp

#define NUM_THREADS 4

#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#define SIZE_X 1024
#define SIZE_Y 1024
extern int FREQ_CENTER;
extern int ACCEPTANCE;
cv::Mat depthMap(cv::Mat input,cv::Mat &reliability);

void analysis_phase(cv::Mat input);
#endif /* QTFTP_hpp */
