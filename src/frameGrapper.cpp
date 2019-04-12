#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>

using namespace cv;
using namespace std;

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
    int index = 0;
    stringstream name;
    
    struct dirent *de;
    DIR *dir = opendir("/home/bp/driverless/frames");
    if(!dir)
    {
        printf("opendir() failed! Does it exist?\n");
        return 1;
    }

    unsigned long count=0;
        while(de = readdir(dir))
     {
          ++count;
     }

    closedir(dir);
    
    index = count*5;
    cout << index;

    for(;;){
        cap >> frame;	
        index++;
	    fps = 1./(what_time_is_it_now() - t);
        t = what_time_is_it_now();
        printf("\nFPS:%.1f\n",fps);
        if(index%3 == 0){
            name << "frames/frame" << index << ".jpg";
            imwrite(name.str(), frame);
            name.str(string());
        }
	    imshow("Test", frame);
        if(waitKey(1) >= 0) break;
    }
}
