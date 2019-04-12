// This code is written at BigVision LLC. It is based on the OpenCV project. It is subject to the license terms in the LICENSE file found in this distribution and at http://opencv.org/license.html

// Usage example:  ./object_detection_yolo.out --video=run.mp4
//                 ./object_detection_yolo.out --image=bird.jpg
#include <fstream>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <mutex>
#include <thread>
#include <queue>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

const char* keys =
        "{help h usage ? | | Usage examples: \n\t\t./object_detection_yolo.out --image=dog.jpg \n\t\t./object_detection_yolo.out --video=run_sm.mp4}"
        "{image i        |<none>| input image   }"
        "{video v       |<none>| input video   }"
        ;
using namespace cv;
using namespace dnn;
using namespace std;

// Initialize the parameters
float confThreshold = 0.7; // Confidence threshold
float nmsThreshold = 0.4; // Non-maximum suppression threshold
int inpWidth = 416; // Width of network's input image
int inpHeight = 416; // Height of network's input image
static bool run;
vector<string> classes;
queue<Mat> capFrames;
queue<Mat> blobs;
queue<Mat> savedFrames;
queue<vector<Mat>> out;
queue<Mat> dones;

// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame, const vector<Mat>& out);

// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net);

double what_time_is_it_now() {
    struct timeval time;
    if (gettimeofday(&time, NULL)) {
        return 0;
    }
    return (double) time.tv_sec + (double) time.tv_usec * .000001;
}

void processFrame() {
    // Give the configuration and weight files for the model
    String modelConfiguration = "cone-tiny.cfg";
    String modelWeights = "cone-tiny_2000.weights";

    // Load the network
    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(3);
    net.setPreferableTarget(0);

    Mat frame, blob;
    vector<Mat> outs;

    while (run) {
        if (!capFrames.empty()) {
            frame = capFrames.front();
            capFrames.pop();
            // Create a 4D blob from a frame.
            blobFromImage(frame, blob, 1 / 255.0, cvSize(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);
            //Sets the input to the network
            net.setInput(blob);
            // Runs the forward pass to get output of the output layers
            net.forward(outs, getOutputsNames(net));

            // Remove the bounding boxes with low confidence
            postprocess(frame, outs);

            // Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes)
            //vector<double> layersTimes;
            //double freq = getTickFrequency() / 1000;
            //double t = net.getPerfProfile(layersTimes) / freq;
            //string label = format("FPS: %.1f", fps);
            //putText(frame, label, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));

            // Write the frame with the detection boxes
            Mat detectedFrame;
            frame.convertTo(detectedFrame, CV_8U);
            dones.emplace(frame);
        }
    }
}

void getFrame(VideoCapture* cap) {
    Mat frame;
    while (run) {
        cap->read(frame);
        capFrames.emplace(frame);
    }
}

void blobbing() {
    Mat blob;
    Mat frame;
    while (run) {
        if (!capFrames.empty()) {
            frame = capFrames.front();
            capFrames.pop();
            blobFromImage(frame, blob, 1 / 255.0, cvSize(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);
            blobs.emplace(blob);
            savedFrames.emplace(frame);
        }
    }
}

void detection() {
    vector<Mat> outs;

    // Give the configuration and weight files for the model
    String modelConfiguration = "cone-tiny.cfg";
    String modelWeights = "cone-tiny_2000.weights";

    // Load the network
    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(3);
    net.setPreferableTarget(0);

    while (run) {
        if (!blobs.empty()) {
            //Sets the input to the network
            net.setInput(blobs.front());
            blobs.pop();

            // Runs the forward pass to get output of the output layers
            net.forward(outs, getOutputsNames(net));
            out.emplace(outs);
        }
    }
}

void postprocessor() {
    Mat done;
    vector<Mat> outs;
    while (run) {
        if (!out.empty() && !savedFrames.empty()) {
            done = savedFrames.front();
            savedFrames.pop();
            outs = out.front();
            out.pop();
            postprocess(done, outs);
            dones.emplace(done);
        }
    }
}

int main() {
    run = true;


    // Load names of classes
    string classesFile = "cone.names";
    ifstream ifs(classesFile.c_str());
    string line;
    while (getline(ifs, line)) classes.push_back(line);

    // Open a video file or an image file or a camera stream.
    VideoCapture cap;

    try {
        cap.open(0);
        cap.set(CV_CAP_PROP_FRAME_WIDTH, inpWidth);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, inpHeight);
        cap.set(CV_CAP_PROP_FPS, 2000);

    }    catch (...) {
        cout << "Could not open the input image/video stream" << endl;
        return 0;
    }

    // Create a window
    static const string kWinName = "Deep learning object detection in OpenCV";
    namedWindow(kWinName, WINDOW_NORMAL);

    thread capturing(getFrame, &cap);
    //thread blobber(blobbing);
    //thread detect(detection);
    //thread postprcs(postprocessor);
    thread processor(processFrame);

    // Process frames.

    Mat frameDone;
    while (waitKey(1) < 0) {
        if (!dones.empty()) {
            frameDone = dones.front();
            dones.pop();
            imshow(kWinName, frameDone);
        }
    }

    run = false;
    capturing.join();
    //blobber.join();
    //detect.join();
    //postprcs.join();
    processor.join();
    cap.release();

    return 0;
}

// Remove the bounding boxes with low confidence using non-maxima suppression

void postprocess(Mat& frame, const vector<Mat>& outs) {
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i) {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*) outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold) {
                int centerX = (int) (data[0] * frame.cols);
                int centerY = (int) (data[1] * frame.rows);
                int width = (int) (data[2] * frame.cols);
                int height = (int) (data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float) confidence);
                boxes.push_back(Rect(left, top, width, height));
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        Rect box = boxes[idx];
        drawPred(classIds[idx], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height, frame);
    }
}

// Draw the predicted bounding box

void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame) {
    //Draw a rectangle displaying the bounding box
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

    //Get the label for the class name and its confidence
    string label = format("%.2f", conf);
    if (!classes.empty()) {
        CV_Assert(classId < (int) classes.size());
        label = classes[classId] + ":" + label;
    }

    //Display the label at the top of the bounding box
    int baseLine;
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    rectangle(frame, Point(left, top - round(1.5 * labelSize.height)), Point(left + round(1.5 * labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
}

// Get the names of the output layers

vector<String> getOutputsNames(const Net& net) {
    static vector<String> names;
    if (names.empty()) {
        //Get the indices of the output layers, i.e. the layers with unconnected outputs
        vector<int> outLayers = net.getUnconnectedOutLayers();

        //get the names of all the layers in the network
        vector<String> layersNames = net.getLayerNames();

        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

