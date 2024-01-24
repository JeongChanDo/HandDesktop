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
	imageTexture = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);



	imageScreen1 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageScreen2 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageScreen3 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageTextureScreen1 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);
	imageTextureScreen2 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);
	imageTextureScreen3 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);

}


void ADesktopGameModeBase::ReadFrame()
{
	/*
	if (!capture.isOpened())
	{
		return;
	}
	capture.read(image);

	cv::Mat desktopImage = GetScreenToCVMat();
	MatToTexture2D(desktopImage);
	*/

	ScreensToCVMats();
	CVMatsToTextures();

}


void ADesktopGameModeBase::MatToTexture2D(const cv::Mat InMat)
{
	if (InMat.type() == CV_8UC3)//example for pre-conversion of Mat
	{
		cv::Mat bgraImage;
		//if the Mat is in BGR space, convert it to BGRA. There is no three channel texture in UE (at least with eight bit)
		cv::cvtColor(InMat, bgraImage, cv::COLOR_BGR2BGRA);

		//Texture->SRGB = 0;//set to 0 if Mat is not in srgb (which is likely when coming from a webcam)
		//other settings of the texture can also be changed here
		//Texture->UpdateResource();

		//actually copy the data to the new texture
		FTexture2DMipMap& Mip = imageTexture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
		FMemory::Memcpy(Data, bgraImage.data, bgraImage.total() * bgraImage.elemSize());//copy the data
		Mip.BulkData.Unlock();
		imageTexture->PostEditChange();
		imageTexture->UpdateResource();
	}
	else if (InMat.type() == CV_8UC4)
	{
		//actually copy the data to the new texture
		FTexture2DMipMap& Mip = imageTexture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
		FMemory::Memcpy(Data, InMat.data, InMat.total() * InMat.elemSize());//copy the data
		Mip.BulkData.Unlock();
		imageTexture->PostEditChange();
		imageTexture->UpdateResource();
	}
	//if the texture hasnt the right pixel format, abort.
	imageTexture->PostEditChange();
	imageTexture->UpdateResource();
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

	DeleteDC(hScreenDC);
	DeleteDC(hMemoryDC);

	DeleteObject(hBitmap);
	DeleteObject(hOldBitmap);


	return matImage;
}


void ADesktopGameModeBase::ScreensToCVMats()
{
	HDC hScreenDC = GetDC(NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	int screenWidth = GetDeviceCaps(hScreenDC, HORZRES);
	int screenHeight = GetDeviceCaps(hScreenDC, VERTRES);

	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

	//screen 1
	BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);
	GetBitmapBits(hBitmap, imageScreen1.total() * imageScreen1.elemSize(), imageScreen1.data);

	//screen 2
	BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 1920, 0, SRCCOPY);
	GetBitmapBits(hBitmap, imageScreen2.total() * imageScreen2.elemSize(), imageScreen2.data);

	//screen 3
	BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 3840, 0, SRCCOPY);
	GetBitmapBits(hBitmap, imageScreen3.total() * imageScreen3.elemSize(), imageScreen3.data);
	SelectObject(hMemoryDC, hOldBitmap);


	DeleteDC(hScreenDC);
	DeleteDC(hMemoryDC);

	DeleteObject(hBitmap);
	DeleteObject(hOldBitmap);

}


void ADesktopGameModeBase::CVMatsToTextures()
{
	for (int i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			FTexture2DMipMap& Mip = imageTextureScreen1->GetPlatformData()->Mips[0];
			void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
			FMemory::Memcpy(Data, imageScreen1.data, imageScreen1.total() * imageScreen1.elemSize());//copy the data
			Mip.BulkData.Unlock();

			imageTextureScreen1->PostEditChange();
			imageTextureScreen1->UpdateResource();
		}
		else if (i == 1)
		{
			FTexture2DMipMap& Mip = imageTextureScreen2->GetPlatformData()->Mips[0];
			void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
			FMemory::Memcpy(Data, imageScreen2.data, imageScreen2.total() * imageScreen2.elemSize());//copy the data
			Mip.BulkData.Unlock();

			imageTextureScreen2->PostEditChange();
			imageTextureScreen2->UpdateResource();
		}
		else if (i == 2)
		{
			FTexture2DMipMap& Mip = imageTextureScreen3->GetPlatformData()->Mips[0];
			void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
			FMemory::Memcpy(Data, imageScreen3.data, imageScreen3.total() * imageScreen3.elemSize());//copy the data
			Mip.BulkData.Unlock();

			imageTextureScreen3->PostEditChange();
			imageTextureScreen3->UpdateResource();
		}


	}


}