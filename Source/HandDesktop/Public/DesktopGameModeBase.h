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


};
