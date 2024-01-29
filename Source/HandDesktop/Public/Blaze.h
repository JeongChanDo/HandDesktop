// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PreOpenCVHeaders.h"
#include <opencv2/opencv.hpp>
#include "PostOpenCVHeaders.h"

#include "CoreMinimal.h"

/**
 * 
 */
class HANDDESKTOP_API Blaze
{
public:
	Blaze();
	~Blaze();

	cv::dnn::Net blazePalm;
	cv::dnn::Net blazeHand;



	// var and funcs for blazepalm
	struct PalmDetection {
		float ymin;
		float xmin;
		float ymax;
		float xmax;
		cv::Point2d kp_arr[7];
		float score;
	};


	int blazeHandSize = 256;
	int blazePalmSize = 128;
	float palmMinScoreThresh = 0.7;
	float palmMinNMSThresh = 0.4;
	int palmMinNumKeyPoints = 7;

	void ResizeAndPad(
		cv::Mat& srcimg, cv::Mat& img256,
		cv::Mat& img128, float& scale, cv::Scalar& pad
	);

	// funcs for blaze palm
	std::vector<PalmDetection> PredictPalmDetections(cv::Mat& img);
	PalmDetection GetPalmDetection(cv::Mat regressor, cv::Mat classificator,
		int stride, int anchor_count, int column, int row, int anchor, int offset);
	float sigmoid(float x);
	std::vector<PalmDetection> DenormalizePalmDetections(std::vector<PalmDetection> detections, int width, int height, cv::Scalar pad);
	void DrawPalmDetections(cv::Mat& img, std::vector<Blaze::PalmDetection> denormDets);
	void DrawDetsInfo(cv::Mat& img, std::vector<Blaze::PalmDetection> filteredDets, std::vector<Blaze::PalmDetection> normDets, std::vector<Blaze::PalmDetection> denormDets);
	std::vector<PalmDetection> FilteringDets(std::vector<PalmDetection> detections, int width, int height);




	// vars and funcs for blaze hand
	std::vector<std::array<int, 2>> HAND_CONNECTIONS = {
	{0, 1}, {1, 2}, {2, 3}, {3, 4},
	{5, 6}, {6, 7}, {7, 8},
	{9, 10}, {10, 11}, {11, 12},
	{13, 14}, {14, 15}, {15, 16},
	{17, 18}, {18, 19}, {19, 20},
	{0, 5}, {5, 9}, {9, 13}, {13, 17}, {0, 17}
	};
	float blazeHandDScale = 3;
	int webcamWidth = 640;
	int webcamHeight = 480;



	std::vector<cv::Rect> convertHandRects(std::vector<PalmDetection> filteredDets);
	void DrawRects(cv::Mat& img, std::vector<cv::Rect> rects);
	void GetHandImages(cv::Mat& img, std::vector<cv::Rect> rects, std::vector<cv::Mat>& handImgs);
	std::vector<cv::Mat> PredictHandDetections(std::vector<cv::Mat>& imgs);
	std::vector<cv::Mat> DenormalizeHandLandmarks(std::vector<cv::Mat> imgs_landmarks, std::vector<cv::Rect> rects);
	void DrawHandDetections(cv::Mat img, std::vector<cv::Mat> denorm_imgs_landmarks);
};