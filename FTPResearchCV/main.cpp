//
//  main.c
//  FTPResearch
//
//  Created by HongQiantan on 2017/9/26.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "TestUtils.hpp"
int main(int argc, const char * argv[]) {
    printf("Arc Reactive Tech FTP Research 2017\nPowered By Tech Lead QT Hong\nE-mail:hongqt@arcreativetech.com\nOptions:\n");
    printf("1)Generate Simulated Clean Figure.\n");
    printf("2)Test OpenCV FFT 1024x1024.\n");
    printf("3)Test OpenCV FTP Algrithm 1024x1024.\n");
    printf("4)Test OpenCV Unwrap Algrithm 1024x1024.\n");
    printf("5)Generate Clean Figure from phase_original.jpg 1024x1024.\n");
    printf("Others: Exit\n");
    while (1) {
        int choice=0;
        //scanf("%d",&choice);
        choice=4;
        switch (choice) {
            case 1:
                free(generate_figure(1024, 1024));
                break;
            case 2:
            {
                cv::Mat test = cv::imread("test.jpg",CV_LOAD_IMAGE_GRAYSCALE);
                process_fft(test);
                break;
            }
            case 3:
            {
                cv::Mat test = cv::imread("test.jpg",CV_LOAD_IMAGE_GRAYSCALE);
                process_ftp(test);
                break;
            }
            case 4:
            {
                cv::Mat test = cv::imread("test.jpg",CV_LOAD_IMAGE_GRAYSCALE);
                process_uwp(test);
                exit(0);
                break;
            }
            case 5:
            {
                generate_figure_cv();
                break;
            }
            default:
                printf("Exiting.\n");
                exit(0);
                break;
        }
    }
    return 0;
}
