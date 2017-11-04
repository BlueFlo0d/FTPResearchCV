//
//  QTFFT.hpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/12.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#ifndef QTFFT_hpp
#define QTFFT_hpp

#include <stdio.h>
#include "opencv2/core/core.hpp"
void QTDFT_rows(const cv::Mat &input,cv::Mat &output);
void QTIDFT_rows(const cv::Mat &input,cv::Mat &output);

#endif /* QTFFT_hpp */
