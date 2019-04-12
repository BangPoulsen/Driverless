#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <queue>
#include <sys/time.h>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;
using namespace dnn;

static mutex theLock;
static bool run;
queue<Mat> capFrames;

double what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

/*void blobbing(){
	Mat blob;
	Mat frame;
	while(run){
		if(!capFrames.empty() ){
			frame = capFrames.front();
			capFrames.pop();
			blobFromImage(frame, blob, 1/255.0, cvSize(inpWidth, inpHeight), Scalar(0,0,0), true, false);
			blobs.emplace(blob);
		}
	}
}

void detection(){
    vector<Mat> outs;

    // Give the configuration and weight files for the model
    String modelConfiguration = "cone-tiny.cfg";
    String modelWeights = "cone-tiny_2000.weights";

    // Load the network
    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(3);
    net.setPreferableTarget(0);
	
	while(run){
		if(!blobs.empty()){
			//Sets the input to the network
			net.setInput(blobs.front());
			blobs.pop();
      
			// Runs the forward pass to get output of the output layers
			net.forward(outs, getOutputsNames(net));
			out.emplace(outs);
		}
	}
}

void postprocessor(){
	Mat done;
	vector<Mat> outs;
	while(run){
		if(!out.empty()){
			outs = out.front();
			out.pop();
			postprocess(done, outs);
			dones.emplace(done);
		}
	}
}

void processing(){
	Mat frame, blob;
    vector<Mat> outs;
    
	// Give the configuration and weight files for the model
    String modelConfiguration = "cone-tiny.cfg";
    String modelWeights = "cone-tiny_2000.weights";

    // Load the network
    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(3);
    net.setPreferableTarget(0);

	while(run){
		if(!capFrames.empty()){
			cout << "There's a frame\n";
			frame = capFrames.front();
			capFrames.pop();
			cout << "Got a new frame\n";
			// Create a 4D blob from a frame.
			blobFromImage(frame, blob, 1/255.0, cvSize(inpWidth, inpHeight), Scalar(0,0,0), true, false);
    	    cout << "Made a blob from the frame\n";
			//Sets the input to the network
			net.setInput(blob);
      		cout << "Input set\n";
			// Runs the forward pass to get output of the output layers
			net.forward(outs, getOutputsNames(net));
			cout << "Forward pass run\n";
			// Remove the bounding boxes with low confidence
			postprocess(frame, outs);
			cout << "Postprocessing done\n";
			dones.emplace(frame);
		}
	}
}*/

int main() {

}

