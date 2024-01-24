// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


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
	cv::VideoCapture capture;
	cv::Mat image;

	UFUNCTION(BlueprintCallable)
	void ReadFrame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* imageTexture;
	void MatToTexture2D(const cv::Mat InMat);


	cv::Mat GetScreenToCVMat();
};
