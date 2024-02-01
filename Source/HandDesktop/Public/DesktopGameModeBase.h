// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Blaze.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

#include "PreOpenCVHeaders.h"
#include <opencv2/opencv.hpp>
#include "PostOpenCVHeaders.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DesktopGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class HANDDESKTOP_API ADesktopGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void ReadFrame();




	int monitorWidth = 1920;
	int monitorHeight = 1080;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* imageTextureScreen1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* imageTextureScreen2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* imageTextureScreen3;
	cv::Mat imageScreen1;
	cv::Mat imageScreen2;
	cv::Mat imageScreen3;

	void ScreensToCVMats();
	void CVMatsToTextures();



	int webcamWidth = 640;
	int webcamHeight = 480;

	cv::VideoCapture capture;
	cv::Mat webcamImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* webcamTexture;
	void MatToTexture2D(const cv::Mat InMat);


	//var and functions with blaze
	Blaze blaze;
	cv::Mat img256;
	cv::Mat img128;
	float scale;
	cv::Scalar pad;





	// vars and funcs for rotator
	int hand_conns_indexes[14] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	void get_pitch_yaw(cv::Point3f pt_start, cv::Point3f pt_end, float& pitch, float& yaw);
	//void calculateRotation(const cv::Point3f& pt1, const cv::Point3f& pt2, float& roll, float& pitch, float& yaw);

	void make_map_for_rotators(std::vector<cv::Mat> denorm_imgs_landmarks);
	void make_map_bone();

	UPROPERTY(BlueprintReadWrite, Category = "RotatorMap")
	TMap<int32, float> MapRoll;
	UPROPERTY(BlueprintReadWrite, Category="RotatorMap")
	TMap<int32, float> MapPitch;
	UPROPERTY(BlueprintReadWrite, Category = "RotatorMap")
	TMap<int32, float> MapYaw;

	UPROPERTY(BlueprintReadWrite, Category = "RotatorMap")
	TMap<int32, FString> MapBoneLeft;
	UPROPERTY(BlueprintReadWrite, Category = "RotatorMap")
	TMap<int32, FString> MapBoneRight;

	UPROPERTY(BlueprintReadWrite, Category = "HandCoord")
	float HandLeftX;
	UPROPERTY(BlueprintReadWrite, Category = "HandCoord")
	float HandLeftY;
};
