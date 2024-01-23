// Fill out your copyright notice in the Description page of Project Settings.


#include "DesktopGameModeBase.h"


void ADesktopGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	capture = cv::VideoCapture(0);

	if (!capture.isOpened())
	{
		UE_LOG(LogTemp, Log, TEXT("Open Webcam failed"));
		return;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Open Webcam Success"));
	}
}


void ADesktopGameModeBase::ReadFrame()
{
	/*
	if (!capture.isOpened())
	{
		return;
	}
	capture.read(image);
	*/

	cv::Mat desktopImage = GetScreenToCVMat();
	imageTexture = MatToTexture2D(desktopImage);
}


UTexture2D* ADesktopGameModeBase::MatToTexture2D(const cv::Mat InMat)
{
	//create new texture, set its values
	UTexture2D* Texture = UTexture2D::CreateTransient(InMat.cols, InMat.rows, PF_B8G8R8A8);

	if (InMat.type() == CV_8UC3)//example for pre-conversion of Mat
	{
		cv::Mat bgraImage;
		//if the Mat is in BGR space, convert it to BGRA. There is no three channel texture in UE (at least with eight bit)
		cv::cvtColor(InMat, bgraImage, cv::COLOR_BGR2BGRA);

		//Texture->SRGB = 0;//set to 0 if Mat is not in srgb (which is likely when coming from a webcam)
		//other settings of the texture can also be changed here
		//Texture->UpdateResource();

		//actually copy the data to the new texture
		FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
		FMemory::Memcpy(Data, bgraImage.data, bgraImage.total() * bgraImage.elemSize());//copy the data
		Mip.BulkData.Unlock();
		Texture->PostEditChange();
		Texture->UpdateResource();
		return Texture;
	}
	else if (InMat.type() == CV_8UC4)
	{
		//actually copy the data to the new texture
		FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
		FMemory::Memcpy(Data, InMat.data, InMat.total() * InMat.elemSize());//copy the data
		Mip.BulkData.Unlock();
		Texture->PostEditChange();
		Texture->UpdateResource();
		return Texture;
	}
	//if the texture hasnt the right pixel format, abort.
	Texture->PostEditChange();
	Texture->UpdateResource();
	return Texture;
}



cv::Mat ADesktopGameModeBase::GetScreenToCVMat()
{
	HDC hScreenDC = GetDC(NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	int screenWidth = GetDeviceCaps(hScreenDC, HORZRES);
	int screenHeight = GetDeviceCaps(hScreenDC, VERTRES);

	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
	BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);
	SelectObject(hMemoryDC, hOldBitmap);

	cv::Mat matImage(screenHeight, screenWidth, CV_8UC4);
	GetBitmapBits(hBitmap, matImage.total() * matImage.elemSize(), matImage.data);

	return matImage;
}
