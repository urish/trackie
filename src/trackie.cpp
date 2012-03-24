//============================================================================
// Name        : trackie.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <cv.h>
#include "TrackingTools.h"
#include "ServerCommunication.h"

using namespace cv;
using namespace std;

static CvScalar colors[] = { { { 0, 0, 255 } }, { { 0, 128, 255 } }, { { 0, 255,
		255 } }, { { 0, 255, 0 } }, { { 255, 128, 0 } }, { { 255, 255, 0 } }, {
		{ 255, 0, 0 } }, { { 255, 0, 255 } }, { { 255, 255, 255 } } };

int main(int argc, char** argv) {
	ServerCommunication* serverCommunication = new ServerCommunication();

	const char* object_filename = "..\\data\\rect.jpg";

	CvMemStorage* storage = cvCreateMemStorage(0);

	cvNamedWindow("Object", 1);
	IplImage* object_original = 0;
	object_original = cvLoadImage(object_filename, CV_LOAD_IMAGE_COLOR);
	if (!object_original) {
		fprintf(stderr, "Cannot load image %s\n", object_filename);
		exit(-1);
	}
	cvShowImage("Object", object_original);
	IplImage* object = cvCreateImage(cvGetSize(object_original), 8, 1);
	cvCvtColor(object_original, object, CV_RGB2GRAY);

	// TODO INIT CONNECTION

	// init camera
	//cv::VideoCapture mCapture;
	//mCapture.open(0);

	CvCapture* capture = 0;
	capture = cvCaptureFromCAM(0);

	// Couldn't get a device? Throw an error and quit
	if (!capture) {
		printf("Could not initialize capturing...\n");
		return -1;
	}

	//if (!mCapture.isOpened())  // check if we succeeded
	//{
	//	fprintf( stderr, "Cannot initialize camera");
	//	exit(-1);
	//}

	cvNamedWindow("Original feed", 1);
	cvNamedWindow("Object Correspond", 1);
	cvNamedWindow("Location", 1);

	CvSeq *objectKeypoints = 0, *objectDescriptors = 0;
	CvSeq *imageKeypoints = 0, *imageDescriptors = 0;
	//Mat cam_capture;
	IplImage* image_original = 0;
	int xAveragePrevious = 0;
	int yAveragePrevious = 0;
	while (true) {
		image_original = cvQueryFrame(capture);
		//mCapture >> cam_capture; // get a new frame from camera and display it in window
		//image_original = cvCreateImage(cvSize(cam_capture.
		//*image_original = cam_capture;
		cvShowImage("Original feed", image_original);

		IplImage* image = cvCreateImage(cvGetSize(image_original), IPL_DEPTH_8U,
				1);
		cvCvtColor(image_original, image, CV_RGB2GRAY);

		/* SURF points extraction */
		int i;
		CvSURFParams params = cvSURFParams(500, 1);
		double tt = (double) cvGetTickCount();
		cvExtractSURF(object, 0, &objectKeypoints, &objectDescriptors, storage,
				params);
		printf("Object Descriptors: %d\n", objectDescriptors->total);
		cvExtractSURF(image, 0, &imageKeypoints, &imageDescriptors, storage,
				params);
		printf("Image Descriptors: %d\n", imageDescriptors->total);
		tt = (double) cvGetTickCount() - tt;
		printf("Extraction time = %gms\n", tt / (cvGetTickFrequency() * 1000.));
		CvPoint src_corners[4] = { { 0, 0 }, { object->width, 0 }, {
				object->width, object->height }, { 0, object->height } };
		CvPoint dst_corners[4];

		IplImage* correspond = cvCreateImage(
				cvSize(image->width, object->height + image->height), 8, 1);
		cvSetImageROI(correspond, cvRect(0, 0, object->width, object->height));
		cvCopy(object, correspond);
		cvSetImageROI(correspond,
				cvRect(0, object->height, correspond->width,
						correspond->height));
		cvCopy(image, correspond);
		cvResetImageROI(correspond);

#ifdef USE_FLANN
		printf("Using approximate nearest neighbor search\n");
#endif

		if (TrackingTools::locatePlanarObject(objectKeypoints,
				objectDescriptors, imageKeypoints, imageDescriptors,
				src_corners, dst_corners)) {
			for (i = 0; i < 4; i++) {
				CvPoint r1 = dst_corners[i % 4];
				CvPoint r2 = dst_corners[(i + 1) % 4];
				cvLine(correspond, cvPoint(r1.x, r1.y + object->height),
						cvPoint(r2.x, r2.y + object->height), colors[8]);
			}
		}
		vector<int> ptpairs;
#ifdef USE_FLANN
		TrackingTools::flannFindPairs(objectKeypoints, objectDescriptors,
				imageKeypoints, imageDescriptors, ptpairs);
#else
		findPairs( objectKeypoints, objectDescriptors, imageKeypoints, imageDescriptors, ptpairs );
#endif

		int xSum = 0;
		int ySum = 0;
		int xAverage = 0;
		int yAverage = 0;
		int numOfPoints = 0;
		for (i = 0; i < (int) ptpairs.size(); i += 2) {
			CvSURFPoint* r1 = (CvSURFPoint*) cvGetSeqElem(objectKeypoints,
					ptpairs[i]);
			CvSURFPoint* r2 = (CvSURFPoint*) cvGetSeqElem(imageKeypoints,
					ptpairs[i + 1]);
			cvLine(correspond, cvPointFrom32f(r1->pt),
					cvPoint(cvRound(r2->pt.x),
							cvRound(r2->pt.y + object->height)), colors[8]);

			xSum = xSum + r2->pt.x;
			ySum = ySum + r2->pt.y;
			numOfPoints++;
		}

		if (numOfPoints) {
			xAverage = xSum / numOfPoints;
			xAveragePrevious = xAverage;
			yAverage = ySum / numOfPoints;
			yAveragePrevious = yAverage;
		} else {
			xAverage = xAveragePrevious;
			yAverage = yAveragePrevious;
		}

		IplImage* image_with_location = cvCreateImage(
				cvSize(image->width, image->height), 8, 3);
		cvSetImageROI(image_with_location,
				cvRect(0, 0, image->width, image->height));
		cvCopy(image_original, image_with_location);
		cvRectangle(image_with_location, cvPoint(xAverage - 20, yAverage + 20),
				cvPoint(xAverage, yAverage), cvScalar(0, 0, 255, 0), 5, 8, 0);
		cvShowImage("Location", image_with_location);

		cvShowImage("Object Correspond", correspond);

		//cvSaveImage("result.jpg", correspond, 0);
		for (i = 0; i < objectKeypoints->total; i++) {
			CvSURFPoint* r = (CvSURFPoint*) cvGetSeqElem(objectKeypoints, i);
			CvPoint center;
			int radius;
			center.x = cvRound(r->pt.x);
			center.y = cvRound(r->pt.y);
			radius = cvRound(r->size * 1.2 / 9. * 2);
			cvCircle(object, center, radius, colors[0], 1, 8, 0);
		}
		cvShowImage("Object", object);
		//cvSaveImage("result2.jpg", object, 0);

		int c = cvWaitKey(10);
		if (c != -1) {
			// If pressed, break out of the loop
			return 0;
		}
	}
	cvDestroyWindow("Object");

	return 0;
}
