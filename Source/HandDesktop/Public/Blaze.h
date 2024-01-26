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
	float palmMinScoreThresh = 0.6;
	float palmMinNMSThresh = 0.2;
	int palmMinNumKeyPoints = 7;

	void ResizeAndPad(
		cv::Mat& srcimg, cv::Mat& img256,
		cv::Mat& img128, float& scale, cv::Scalar& pad
	);

	std::vector<PalmDetection> PredictPalmDetections(cv::Mat& img);
	PalmDetection GetPalmDetection(cv::Mat regressor, cv::Mat classificator,
		int stride, int anchor_count, int column, int row, int anchor, int offset);
	float sigmoid(float x);
	std::vector<PalmDetection> DenormalizePalmDetections(std::vector<PalmDetection> detections, int width, int height, cv::Scalar pad);
	void DrawPalmDetections(cv::Mat& img, std::vector<Blaze::PalmDetection> denormDets);
	std::vector<PalmDetection> FilteringDets(std::vector<PalmDetection> detections, int width, int height);


};