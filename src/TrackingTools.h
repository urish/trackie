#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/types_c.h"
#include <iostream>
#include <vector>

#pragma once
// define whether to use approximate nearest-neighbor search
#define USE_FLANN

class TrackingTools {
public:
	TrackingTools(void);
	~TrackingTools(void);
	static int
	locatePlanarObject(const CvSeq* objectKeypoints,
			const CvSeq* objectDescriptors, const CvSeq* imageKeypoints,
			const CvSeq* imageDescriptors, const CvPoint src_corners[4],
			CvPoint dst_corners[4]);
	static void
	flannFindPairs(const CvSeq*, const CvSeq* objectDescriptors, const CvSeq*,
			const CvSeq* imageDescriptors, std::vector<int>& ptpairs);
private:
	double compareSURFDescriptors(const float* d1, const float* d2, double best,
			int length);
	int
	naiveNearestNeighbor(const float* vec, int laplacian,
			const CvSeq* model_keypoints, const CvSeq* model_descriptors);
	void
	findPairs(const CvSeq* objectKeypoints, const CvSeq* objectDescriptors,
			const CvSeq* imageKeypoints, const CvSeq* imageDescriptors,
			std::vector<int>& ptpairs);
};
