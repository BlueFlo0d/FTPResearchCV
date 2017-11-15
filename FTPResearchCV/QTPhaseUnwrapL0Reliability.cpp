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
#include <opencv2/highgui.hpp>
#include <pthread.h>
#include "TestUtils.hpp"
using namespace cv;
const int queue_length = 2048;
int _dimx = SIZE_X;
int _pdimx =0;
const int cut_threshold = 0;
#define queue_count 32
//#define discard_ratio 32
#define AT(x,y,map) (map[(y)*_dimx+(x)])
#define ATP(x,y,map) (map[(y)*_pdimx+(x)])
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
void find_startpoint(int *x,int *y,float *val,float *R,int xstart,int ystart,int dimx,int dimy){
    for (int i=ystart; i<dimy+ystart; i++) {
        for (int j=xstart; j<dimx+xstart; j++) {
            if (R[i*dimx+j]>*val) {
                *x=j;
                *y=i;
                *val =R[i*dimx+j];
            }
        }
    }
}

inline void process_point(float *input,float *output,float *R,uchar *mask,int x,int y,float topline,float baseline,int *max,queue *queues,int xstart,int ystart,float max_val){
    float val = AT(x,y,input);
    if (val>topline) {
        ATP(x-xstart,y-ystart,output)=AT(x,y,input)-2*M_PI;
    }
    else if (val<baseline) {
        ATP(x-xstart,y-ystart,output)=AT(x,y,input)+2*M_PI;
    }
    else{
        ATP(x-xstart,y-ystart,output)=AT(x,y,input);
    }
    int index =cvFloor(AT(x, y, R)*queue_count/max_val);
    index = (index < queue_count)?index:(queue_count-1);
    ATP(x-xstart,y-ystart,mask)=0;
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
void unwrap_point(float *input,float *output,float *R,uchar *mask,int x,int y,int *max,queue *queues,float max_val,int xstart,int ystart,int dimx,int dimy){
    float val = ATP(x-xstart,y-ystart,output);
    float topline = val+PHASE_THRESHOLD;
    float baseline = val-PHASE_THRESHOLD;
    /*
    if (AT(x, y, mask)) {
        printf("duplicated X %d Y %d\n",x,y);
        //return;
    }*/

    //printf("unwrap X %d Y %d\n",x,y);
    if (((x+1)<dimx+xstart)&&ATP(x+1-xstart,y-ystart,mask)) {
        process_point(input,output,R,mask, x+1, y, topline, baseline,max, queues,xstart,ystart, max_val);
    }
    if ((x>xstart)&&ATP(x-1-xstart,y-ystart,mask)) {
        process_point(input,output,R,mask, x-1, y, topline, baseline,max, queues,xstart,ystart, max_val);
    }
    if ((y+1<dimy+ystart)&&ATP(x-xstart,y+1-ystart,mask)) {
        process_point(input,output,R,mask, x, y+1, topline, baseline,max, queues,xstart,ystart, max_val);
    }
    if ((y>ystart)&&ATP(x-xstart,y-1-ystart,mask)) {
        process_point(input,output,R,mask, x, y-1, topline, baseline,max, queues,xstart,ystart, max_val);
    }
}
typedef struct _unwrap_args{
    float *input;
    float *output;
    float *R;
    uchar *mask;
    int dimx;
    int dimy;
    int xstart;
    int ystart;
} unwrap_args;
void *uwp_worker(void* arg_raw){
    
    unwrap_args *args=(unwrap_args *)arg_raw;
    int x=0,y=0,max=queue_count-1;
    float max_val=0;
    float *R = args->R;
    float *input = args->input;
    float *output = args->output;
    uchar *mask = args->mask;
    int dimx = args->dimx;//!!!NEED TO SET _DIMX PROPERLY!!!
    int dimy = args->dimy;
    int xstart = args->xstart;
    int ystart= args->ystart;
    
    find_startpoint(&x, &y, &max_val, R,xstart,ystart,dimx,dimy);
    
    queue queues[queue_count];
    for (int i=0; i<queue_count; i++) {
        construct_queue(&queues[i]);
    }
    ATP(x-xstart, y-ystart, output)=AT(x, y, input);
    ATP(x-xstart, y-ystart, mask)=0;
    enqueue(x, y, queues+max);
    TIC
    while (max>cut_threshold) {
        while (max>cut_threshold) {
            if (dequeue(&x, &y, queues+max)) {
                max--;
            }
            else
                break;
        }
        unwrap_point(input,output,R, mask, x, y,&max, queues, max_val,xstart,ystart,dimx,dimy);
    }
    TOC
    /*
     for (int i=0; i<1024; i++) {
     enqueue(i, i+1, queues);
     dequeue(&x, &y, queues);
     printf("%d,%d\n",x,y);
     }*/
    for (int i=0; i<queue_count; i++) {
        destroy_queue(&queues[i]);
    }
    return 0;
}
//output unused.
inline float stat_edge_R(unwrap_args *arg){
    float* input=arg->input;
    uchar* mask=arg->mask;
    int dimx=arg->dimx;
    int dimy=arg->dimy;
    float result = 0;
    int factor = 0;
    for (int i=0; i<dimy; i++) {
        int t=1-ATP(dimx-2, i, mask);
        factor+=t;
        result+=t*ATP(dimx-2, i, input);
    }
    return result/factor;
}
inline float stat_edge_L(unwrap_args *arg){
    float* input=arg->input;
    uchar* mask=arg->mask;
    int dimy=arg->dimy;
    float result = 0;
    int factor = 0;
    for (int i=0; i<dimy; i++) {
        int t=1-ATP(0, i, mask);
        result+=t*ATP(0, i, input);
        factor+=t;
    }
    return result/factor;
}
inline float stat_edge_U(unwrap_args *arg){
    float* input=arg->input;
    uchar* mask=arg->mask;
    int dimx=arg->dimx;
    float result = 0;
    int factor = 0;
    for (int i=0; i<dimx; i++) {
        int t=1-ATP(i, 0, mask);
        result+=t*ATP(i, 0, input);
        factor+=t;
    }
    return result/factor;
}
inline float stat_edge_D(unwrap_args *arg){
    float* input=arg->input;
    uchar* mask=arg->mask;
    int dimx=arg->dimx;
    int dimy=arg->dimy;
    float result = 0;
    int factor = 0;
    for (int i=0; i<dimx; i++) {
        int t=1-ATP(i, dimy-1, mask);
        result+=t*ATP(i, dimy-1, input);
        factor+=t;
    }
    return result/factor;
}
inline int wrap_k(float x,float y){
    float delta = (x-y)/M_PI;
    if (delta>0.5) {
        delta+=0.5;
        return cvFloor((delta+1)/2);
    }
    else if (delta<-0.5){
        delta-=0.5;
        return -cvFloor((-delta+1)/2);
    }
    else
        return 0;
}
//#define UWP_THREAD_4
void unwrap_phase(cv::Mat &mat_input,cv::Mat &mat_output,cv::Mat &mat_R){
    Mat mat_mask = Mat::ones(mat_input.rows,mat_output.cols, CV_8UC1);
    
    float *R = mat_R.ptr<float>(0);
    float *input = mat_input.ptr<float>(0);
    
    int dimx = mat_input.cols;
    int dimy = mat_input.rows;
    _dimx = dimx;
    TIC
#ifdef UWP_THREAD_4
    Mat sections_output[4],sections_mask[4];
    _pdimx=dimx/2+1;
    
    for (int i=0; i<4; i++) {
        sections_output[i]=Mat( dimy/2+1,dimx/2+1, CV_32F,Scalar_<float>(0));
        sections_mask[i]=Mat( dimy/2+1,dimx/2+1, CV_8U,Scalar_<uchar>(1));
    }
    
    pthread_t workers[4];
    unwrap_args uwp_args[4];
    uwp_args[0].xstart=uwp_args[2].xstart=0;
    uwp_args[1].xstart=uwp_args[3].xstart=dimx/2-1;
    uwp_args[0].ystart=uwp_args[1].ystart=0;
    uwp_args[2].ystart=uwp_args[3].ystart=dimy/2-1;
    for (int i=0; i<4; i++) {
        
        uwp_args[i].input=input;
        uwp_args[i].output=sections_output[i].ptr<float>();
        uwp_args[i].R=R;
        uwp_args[i].mask=sections_mask[i].ptr();
        uwp_args[i].dimx=dimx/2+1;
        uwp_args[i].dimy=dimy/2+1;
        pthread_create(workers+i, NULL, uwp_worker, uwp_args+i);
    }
    
    for (int i=0; i<4; i++) {
        pthread_join(workers[i], NULL);
    }
    int k;
    if ((k=wrap_k(stat_edge_R(uwp_args), stat_edge_L(uwp_args+1)))!=0) {
        sections_output[1].convertTo(sections_output[1], -1,1,-2*M_PI*k);
    }
    if ((k=wrap_k(stat_edge_D(uwp_args), stat_edge_U(uwp_args+2)))!=0) {
        sections_output[2].convertTo(sections_output[2], -1,1,-2*M_PI*k);
    }
    if ((k+=wrap_k(stat_edge_R(uwp_args+2), stat_edge_L(uwp_args+3)))!=0) {
        sections_output[3].convertTo(sections_output[3], -1,1,-2*M_PI*k);
    }
    printf("%f,%f\n",stat_edge_D(uwp_args),stat_edge_U(uwp_args+2));
    
    for (int i=0; i<4; i++) {
        sections_output[i].copyTo(mat_output(cvRect(uwp_args[i].xstart, uwp_args[i].ystart, dimx/2+1, dimy/2+1)));
        sections_mask[i].copyTo(mat_mask(cvRect(uwp_args[i].xstart, uwp_args[i].ystart, dimx/2+1, dimy/2+1)));
    }
#elif defined UWP_THREAD_2
    _pdimx=dimx;
    Mat sections_output[2],sections_mask[2];
    for (int i=0; i<2; i++) {
        sections_output[i]=Mat(dimy/2+1,dimx,  CV_32F,Scalar_<float>(0));
        sections_mask[i]=Mat(dimy/2+1,dimx,  CV_8U,Scalar_<uchar>(1));
    }
    
    pthread_t workers[2];
    unwrap_args uwp_args[2];
    uwp_args[0].xstart=uwp_args[1].xstart=0;
    uwp_args[0].ystart=0;
    uwp_args[1].ystart=dimy/2-1;
    for (int i=0; i<2; i++) {
        
        uwp_args[i].input=input;
        uwp_args[i].output=sections_output[i].ptr<float>();
        uwp_args[i].R=R;
        uwp_args[i].mask=sections_mask[i].ptr();
        uwp_args[i].dimx=dimx;
        uwp_args[i].dimy=dimy/2+1;
        pthread_create(workers+i, NULL, uwp_worker, uwp_args+i);
    }
    
    for (int i=0; i<2; i++) {
        pthread_join(workers[i], NULL);
    }
    //imwrite("test_p.jpg", sections_output[0]);
    for (int i=0; i<2; i++) {
        sections_output[i].copyTo(mat_output(cvRect(uwp_args[i].xstart, uwp_args[i].ystart, dimx, dimy/2+1)));
        sections_mask[i].copyTo(mat_mask(cvRect(uwp_args[i].xstart, uwp_args[i].ystart, dimx, dimy/2+1)));
    }
#else
    float *output = mat_output.ptr<float>(0);
    uchar *mask = mat_mask.ptr<uchar>(0);
    _pdimx=dimx;
    unwrap_args uwp_args[1];
    uwp_args[0].input=input;
    uwp_args[0].output=output;
    uwp_args[0].R=R;
    uwp_args[0].mask=mask;
    uwp_args[0].dimx=dimx;
    uwp_args[0].dimy=dimy;
    uwp_args[0].ystart=0;
    uwp_args[0].xstart=0;
    uwp_worker((void*)uwp_args);
    
#endif
    TOC
    const int morph_radius = 5;
    
    Mat element =getStructuringElement(MORPH_RECT,Size(2*morph_radius+1,2*morph_radius+1),Point(morph_radius, morph_radius));
    morphologyEx(mat_mask, mat_mask, CV_MOP_TOPHAT, element);
    inpaint(mat_output, mat_mask, mat_output, 3.0, CV_INPAINT_NS);
}
