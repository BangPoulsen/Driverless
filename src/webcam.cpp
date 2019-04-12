#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <sys/time.h>

using namespace cv;

double what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

int main(){
    VideoCapture cap(0);
    if(!cap.isOpened())
        return -1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 416);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 416);
    cap.set(CV_CAP_PROP_FPS, 120);
    double fps;
    Mat frame;
    double t = what_time_is_it_now();

    for(;;){
        cap >> frame;	
	    fps = 1./(what_time_is_it_now() - t);
        t = what_time_is_it_now();
        printf("\nFPS:%.1f\n",fps);
	    imshow("Test", frame);
        if(waitKey(1) >= 0) break;
    }
}
