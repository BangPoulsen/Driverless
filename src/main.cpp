#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <sys/time.h>
#include <queue>
#include <iterator>

// Initialize the parameters
float confThreshold = 0.2; // Confidence threshold
float nmsThreshold = 0.4; // Non-maximum suppression threshold
int inpWidth = 416; // Width of network's input image
int inpHeight = 416; // Height of network's input image
static bool run;
std::vector<std::string> classes;
std::vector<cv::String> imageNames;
std::vector<cv::String> targetFileNames;
std::queue<cv::Mat> images;
std::queue<std::vector<cv::Point2d>> imageTargets;
std::vector<cv::Point2d> targets;
std::vector<cv::Point2d> predictions;
std::vector<double> precisions;
std::ifstream file;

double start;
double totalTime;

struct Target {
    double x, y;

    Target(double a, double b) {
        this->x = a;
        this->y = b;
    }
};

double what_time_is_it_now() {
    struct timeval time;
    if (gettimeofday(&time, NULL)) {
        return 0;
    }
    return (double) time.tv_sec + (double) time.tv_usec * .000001;
}

// Get the names of the output layers
std::vector<cv::String> getOutputsNames(const cv::dnn::Net &net) {
    static std::vector<cv::String> names;
    if (names.empty()) {
        //Get the indices of the output layers, i.e. the layers with unconnected outputs
        std::vector<int> outLayers = net.getUnconnectedOutLayers();

        //get the names of all the layers in the network
        std::vector<cv::String> layersNames = net.getLayerNames();

        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

// Remove the bounding boxes with low confidence using non-maximum suppression
void postprocess(cv::Mat &frame, const std::vector<cv::Mat> &outs) {
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i) {
        float *data = (float *) outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
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
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    predictions.clear();
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        cv::Rect box = boxes[idx];
        predictions.push_back(cv::Point2d(box.x, box.y));
    }

    totalTime += what_time_is_it_now() - start;
    double score = 0;
    int predictionsN = predictions.size();
    int closest = NULL;
    double distance;
    for (int l = 0; l < targets.size(); ++l) {
        for (int k = 0; k < predictionsN; ++k) {
            if (closest == NULL) {
                closest = k;
            } else {
                distance = cv::norm(targets[l] - predictions[k]);
                if (distance < (cv::norm(targets[l] - predictions[closest]))) {
                    closest = k;
                }
            }
        }
        if(predictionsN != 0) {
            score += 1 - (cv::norm(targets[l] - predictions[closest]) /
                          cv::norm(cv::Point2d(0, 0) - cv::Point2d(frame.size().width, frame.size().height)));
        } else {
            score += 0;
        }
        predictions.erase(predictions.begin()+closest);
        closest = NULL;
    }
    double precision;
    if(predictionsN != 0){
        precision = score/predictionsN;
    } else {
        precision = 0;
    }
    precisions.push_back(precision);
}

void process() {
    // Give the configuration and weight files for the model
    cv::String modelConfiguration = "cone-tiny.cfg";
    cv::String modelWeights = "cone-tiny_900.weights";

    std::cout << "Loading network" << std::endl;
    // Load the network
    cv::dnn::Net net = cv::dnn::readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(3);
    net.setPreferableTarget(0);

    cv::Mat frame, blob;
    std::vector<cv::Mat> outs;

    while (!images.empty()) {
        start = what_time_is_it_now();
        frame = images.front();
        images.pop();
        targets = imageTargets.front();
        imageTargets.pop();
        cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cvSize(inpWidth, inpHeight), cv::Scalar(0, 0, 0), true, false);
        net.setInput(blob);
        net.forward(outs, getOutputsNames(net));

        postprocess(frame, outs);
    }
}

int main() {
    cv::glob("/home/bangpoulsen/Driverless/testImages/*.jpg", imageNames, false);
    for (int i = 0; i < imageNames.size(); ++i) {
        cv::Mat image = cv::imread(imageNames[i]);
        cv::resize(image, image, cvSize(inpWidth, inpHeight));
        images.emplace(image);
    }
    cv::glob("/home/bangpoulsen/Driverless/testImages/*.txt", targetFileNames, false);
    std::string line;
    for (int j = 0; j < targetFileNames.size(); ++j) {
        file.open(targetFileNames[j]);
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::vector<std::string> strings( (std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>() );

            std::cout << "testx: " << (inpWidth*std::stod(strings[1])) << " testy: " << (inpHeight*(1-std::stod(strings[2]))) << std::endl;

            cv::Point2d target((inpWidth*std::stod(strings[1])), (inpHeight*(1-std::stod(strings[2]))));
            targets.push_back(target);
        }
        file.close();
        imageTargets.emplace(targets);
        targets.clear();
    }

    std::cout << "Start" << std::endl;
    process();

    double totalPrecision = 0;
    for (int k = 0; k < precisions.size(); ++k) {
        std::cout << precisions[k] << std::endl;
        totalPrecision += precisions[k];
    }
    std::cout << "Precision: " << (totalPrecision/(precisions.size()+1)) << std::endl;
    std::cout << "Time: " << totalTime << std::endl;

    return 0;
}