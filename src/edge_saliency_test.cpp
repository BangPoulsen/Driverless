#include "opencv2/opencv.hpp"
#include "opencv2/saliency.hpp"
#include <iostream>
using namespace cv;
using namespace std;
using namespace saliency;

RNG rng(12345);

Mat makeEdges(Mat image){
    Mat output;
    medianBlur( image, image, 21 );
    Canny( image, output, 100, 200 );
    return output;
}

Mat contourDrawing(Mat binary, Mat original){
    Mat kernel = Mat( 5, 5, CV_8UC1, Scalar(1) );
    Mat output = original.clone();
    vector< vector<Point> > contours;
    dilate( binary, binary, kernel );
    erode( binary, binary, kernel );

    findContours(binary, contours, CV_RETR_TREE, CV_CHAIN_APPROX_TC89_L1);
    vector< vector<Point> > contour_poly( contours.size() );
    vector<Rect> boundingBox( contours.size() );

    for( size_t i = 0; i < contours.size(); i++){
        Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0, 256) );
        approxPolyDP( contours[i], contour_poly[i], 3, true );
        boundingBox[i] = boundingRect( contour_poly[i] );
        if(boundingBox[i].area() > 1000){
            rectangle( output, boundingBox[i].tl(), boundingBox[i].br(), color, 2 );
        }
    }
    return output;
}

Mat fineGrained( Mat image ){
    Mat saliencyMap, output;
    Ptr<Saliency> saliencyAlgorithm = StaticSaliencyFineGrained::create();
    if( saliencyAlgorithm->computeSaliency( image, saliencyMap ) ){
        threshold( saliencyMap, output, 50, 255, 0 );
    }
    return output;
}

int main(int, char**)
{   
    VideoCapture cap(0);
    if(!cap.isOpened())
        return -1;

    Mat frame;

    /*frame = imread("test.jpg");
    frame = imread("road-maintenance-vehicle.jpg");
    Mat edges = makeEdges(frame.clone());
    imshow("Edge Test", edges);
    Mat edgeBoxes = contourDrawing(edges, frame.clone());
    imshow("Edge Boxes", edgeBoxes);

    Mat saliency = fineGrained(frame.clone());
    imshow("Saliency Test", saliency);
    Mat saliencyBoxes = contourDrawing(saliency, frame.clone());
    imshow("Saliency Boxes", saliencyBoxes);*/

    for(;;)
    {
        cap >> frame;
	Mat edges = makeEdges(frame.clone());
        imshow("Edge Test", edges);
        Mat edgeBoxes = contourDrawing(edges, frame.clone());
        imshow("Edge Boxes", edgeBoxes);

        /*Mat saliency = fineGrained(frame.clone());
        imshow("Saliency Test", saliency);
        Mat saliencyBoxes = contourDrawing(saliency, frame.clone());
        imshow("Saliency Boxes", saliencyBoxes);*/
        if(waitKey(30) >= 0) break;
    }
    return 0;
}


