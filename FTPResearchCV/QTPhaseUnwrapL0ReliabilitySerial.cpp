//
//  QTPhaseUnwrapL0Reliability.cpp
//  FTPResearchCV
//
//  Created by HongQiantan on 2017/10/10.
//  Copyright © 2017年 ArcReactiveTech. All rights reserved.
//

#include <stdio.h>
#include "QTPhaseUnwrap.hpp"
#include <opencv2/photo.hpp>
using namespace cv;
const int queue_length = 2048;
const int dim = SIZE_X;
const int cut_threshold = 0;
#define queue_count 32
//#define discard_ratio 32
#define AT(x,y,map) (map[(y)*dim+(x)])
typedef struct _queue{
    int *x;
    int *y;
    int location;
    int length;
    _queue *next;
} queue;
void construct_queue(queue *aqueue){
    //aqueue = (queue *)malloc(sizeof(queue));
    memset(aqueue, 0, sizeof(queue));
    aqueue->x = (int *)malloc(queue_length*sizeof(int));
    //memset(aqueue->x,0,queue_length*sizeof(int));
    aqueue->y = (int *)malloc(queue_length*sizeof(int));
    //memset(aqueue->y,0,queue_length*sizeof(int));
}
void enqueue(int x,int y,queue* aqueue){
    if (aqueue->length<queue_length) {
        int loc = (aqueue->location+aqueue->length)%queue_length;
        aqueue->x[loc]=x;
        aqueue->y[loc]=y;
        aqueue->length ++;
    }
    else {
        if (!(aqueue->next)) {
            aqueue->next=(queue *)malloc(sizeof(queue));
            construct_queue(aqueue->next);
        }
        enqueue(x, y, aqueue->next);
    }
}
int dequeue(int *x,int *y,queue* aqueue){
    if (aqueue->length) {
        int loc = aqueue->location;
        *x = aqueue->x[loc];
        *y = aqueue->y[loc];
        aqueue->location++;
        aqueue->length--;
        aqueue->location%=queue_length;
    }
    else if(aqueue->next){
        if (dequeue(x, y, aqueue->next)) {
            return -1;
        }
    }
    else {
        return -1;
    }
    return 0;
}
void destroy_queue(queue* aqueue){
    free(aqueue->x);
    free(aqueue->y);
    if (aqueue->next) {
        destroy_queue(aqueue->next);
        free(aqueue->next);
    }
    //free(aqueue);
}
void find_startpoint(int *x,int *y,float *val,float *R){
    for (int i=0; i<SIZE_Y; i++) {
        for (int j=0; j<SIZE_X; j++) {
            if (R[i*SIZE_X+j]>*val) {
                *x=j;
                *y=i;
                *val =R[i*SIZE_X+j];
            }
        }
    }
}

inline void process_point(float *input,float *output,float *R,uchar *mask,int x,int y,float topline,float baseline,int *max,queue *queues,float max_val){
    float val = AT(x,y,input);
    if (val>topline) {
        AT(x,y,output)=AT(x,y,input)-2*M_PI;
    }
    else if (val<baseline) {
        AT(x,y,output)=AT(x,y,input)+2*M_PI;
    }
    else{
        AT(x,y,output)=AT(x,y,input);
    }
    int index =cvFloor(AT(x, y, R)*queue_count/max_val);
    AT(x,y,mask)=0;
    /*
    if (index==0&&(AT(x, y, R)>max_val/discard_ratio)) {
        index++;
    }*/
    enqueue(x, y, queues+index);
    if (index>*max) {
        *max=index;
    }
}
#define PHASE_THRESHOLD 3.1415926535
void unwrap_point(float *input,float *output,float *R,uchar *mask,int x,int y,int *max,queue *queues,float max_val){
    float val = AT(x,y,output);
    float topline = val+PHASE_THRESHOLD;
    float baseline = val-PHASE_THRESHOLD;
    /*
    if (AT(x, y, mask)) {
        printf("duplicated X %d Y %d\n",x,y);
        //return;
    }*/

    //printf("unwrap X %d Y %d\n",x,y);
    if (((x+1)<dim)&&AT(x+1,y,mask)) {
        process_point(input,output,R,mask, x+1, y, topline, baseline,max, queues, max_val);
    }
    if ((x>0)&&AT(x-1,y,mask)) {
        process_point(input,output,R,mask, x-1, y, topline, baseline,max, queues, max_val);
    }
    if ((y+1<dim)&&AT(x,y+1,mask)) {
        process_point(input,output,R,mask, x, y+1, topline, baseline,max, queues, max_val);
    }
    if ((y>0)&&AT(x,y-1,mask)) {
        process_point(input,output,R,mask, x, y-1, topline, baseline,max, queues, max_val);
    }
}

//output unused.
void unwrap_phase(cv::Mat &mat_input,cv::Mat &mat_output,cv::Mat &mat_R){
    Mat mat_mask = Mat::ones(mat_input.rows,mat_output.cols, CV_8UC1);
    int x=0,y=0,max=queue_count-1;
    float max_val=0;
    float *R = mat_R.ptr<float>(0);
    float *input = mat_input.ptr<float>(0);
    float *output = mat_output.ptr<float>(0);
    uchar *mask = mat_mask.ptr<uchar>(0);

    find_startpoint(&x, &y, &max_val, R);

    queue queues[queue_count];
    for (int i=0; i<queue_count; i++) {
        construct_queue(&queues[i]);
    }
    AT(x, y, output)=AT(x, y, input);
    AT(x, y, mask)=0;
    enqueue(x, y, queues+max);

    while (max>cut_threshold) {
        while (max>cut_threshold) {
            if (dequeue(&x, &y, queues+max)) {
                max--;
            }
            else
                break;
        }
        unwrap_point(input,output,R, mask, x, y,&max, queues, max_val);
    }
    /*
    for (int i=0; i<1024; i++) {
        enqueue(i, i+1, queues);
        dequeue(&x, &y, queues);
        printf("%d,%d\n",x,y);
    }*/
    for (int i=0; i<queue_count; i++) {
        destroy_queue(&queues[i]);
    }
    const int morph_radius = 5;
    Mat element =getStructuringElement(MORPH_RECT,Size(2*morph_radius+1,2*morph_radius+1),Point(morph_radius, morph_radius));
    morphologyEx(mat_mask, mat_mask, CV_MOP_TOPHAT, element);
    inpaint(mat_output, mat_mask, mat_output, 3.0, CV_INPAINT_NS);
}
