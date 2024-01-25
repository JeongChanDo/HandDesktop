// Fill out your copyright notice in the Description page of Project Settings.

#include "DesktopGameModeBase.h"


void ADesktopGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	blaze = Blaze();

	capture = cv::VideoCapture(1);
	if (!capture.isOpened())
	{
		UE_LOG(LogTemp, Log, TEXT("Open Webcam failed"));
		return;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Open Webcam Success"));
	}
	capture.set(cv::CAP_PROP_FRAME_WIDTH, webcamWidth);
	capture.set(cv::CAP_PROP_FRAME_HEIGHT, webcamHeight);


	webcamTexture = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);


	imageScreen1 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageScreen2 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageScreen3 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageTextureScreen1 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);
	imageTextureScreen2 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);
	imageTextureScreen3 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);

}


void ADesktopGameModeBase::ReadFrame()
{
	if (!capture.isOpened())
	{
		return;
	}
	capture.read(webcamImage);

	/*
	get filtered detections
	*/
	blaze.ResizeAndPad(webcamImage, img256, img128, scale, pad);
	//UE_LOG(LogTemp, Log, TEXT("scale value: %f, pad value: (%f, %f)"), scale, pad[0], pad[1]);
	std::vector<Blaze::PalmDetection> normDets = blaze.PredictPalmDetections(img128);
	std::vector<Blaze::PalmDetection> denormDets = blaze.DenormalizePalmDetections(normDets, webcamWidth, webcamHeight, pad);
	std::vector<Blaze::PalmDetection> filteredDets = blaze.FilteringDets(denormDets, webcamWidth, webcamHeight);



	/*
	get affined hand img, affine mat, ROI
	*/
	std::vector<float> vec_xc;
	std::vector<float> vec_yc;
	std::vector<float> vec_scale;
	std::vector<float> vec_theta;
	blaze.Detections2ROI(filteredDets, vec_xc, vec_yc, vec_scale, vec_theta);


	std::vector<cv::Mat> vec_img;
	std::vector<cv::Mat> vec_affine;
	std::vector<Blaze::BoxROI> vec_boxROI;

	blaze.extract_roi(webcamImage, vec_xc, vec_yc,
		vec_scale, vec_theta, filteredDets,
		vec_img, vec_affine, vec_boxROI);


	/*
	Draw
	*/
	blaze.DrawPalmDetections(webcamImage, filteredDets);

	std::string dets_size_str = "filtered dets : " + std::to_string(filteredDets.size()) + ", norm dets : " + std::to_string(normDets.size()) + ", denorm dets : " + std::to_string(denormDets.size());
	cv::putText(webcamImage, dets_size_str, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);

	for (int i = 0; i < filteredDets.size(); i++)
	{
		auto& det = filteredDets.at(i);


		std::ostringstream oss;
		oss << "denorm dets : (" << det.xmin << ", " << det.ymin << "),(" <<
			det.xmax << ", " << det.ymax << "), center : " << vec_xc.at(i) <<","<<vec_yc.at(i);
		std::string det_str = oss.str();
		cv::putText(webcamImage, det_str, cv::Point(30, 50 + 20 * i), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
	}



	std::ostringstream oss_vec;
	oss_vec << "vec_xc : " << vec_xc.size() << ", vec_scale : " << vec_scale.size() << ", vec_theta" << vec_theta.size();
	std::string oss_vec_str = oss_vec.str();
	cv::putText(webcamImage, oss_vec_str, cv::Point(30, 450), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);



	std::ostringstream oss_mat;
	oss_mat << "vec_img : " << vec_img.size() << ", vec_affine : " << vec_affine.size() << ", vec_boxROI" << vec_boxROI.size();
	std::string oss_mat_str = oss_mat.str();
	cv::putText(webcamImage, oss_mat_str, cv::Point(30, 465), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);




	for (int i = 0; i < vec_img.size(); i++)
	{
		cv::Mat tmp = vec_img.at(i);
		cv::resize(tmp, tmp, cv::Size(100, 100));
		cv::Mat roi = webcamImage(cv::Rect(400 + (i * 100), 300, 100, 100));
		tmp.copyTo(roi);
	}

	/*
	blaze.DrawPalmDetections(img128, normDets);
	cv::Mat roi1 = webcamImage(cv::Rect(1000, 500, img128.cols, img128.rows));
	img128.copyTo(roi1);
	*/


	MatToTexture2D(webcamImage);


	/*
	모니터 시각화
	*/
	ScreensToCVMats();
	CVMatsToTextures();
}


void ADesktopGameModeBase::MatToTexture2D(const cv::Mat InMat)
{
	if (InMat.type() == CV_8UC3)//example for pre-conversion of Mat
	{
		cv::Mat resizedImage;
		cv::resize(InMat, resizedImage, cv::Size(monitorWidth, monitorHeight));
		cv::Mat bgraImage;
		//if the Mat is in BGR space, convert it to BGRA. There is no three channel texture in UE (at least with eight bit)
		cv::cvtColor(resizedImage, bgraImage, cv::COLOR_BGR2BGRA);

		//actually copy the data to the new texture
		FTexture2DMipMap& Mip = webcamTexture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
		FMemory::Memcpy(Data, bgraImage.data, bgraImage.total() * bgraImage.elemSize());//copy the data
		Mip.BulkData.Unlock();
		webcamTexture->PostEditChange();
		webcamTexture->UpdateResource();
	}
	else if (InMat.type() == CV_8UC4)
	{
		//actually copy the data to the new texture
		FTexture2DMipMap& Mip = webcamTexture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);//lock the texture data
		FMemory::Memcpy(Data, InMat.data, InMat.total() * InMat.elemSize());//copy the data
		Mip.BulkData.Unlock();
		webcamTexture->PostEditChange();
		webcamTexture->UpdateResource();
	}
	//if the texture hasnt the right pixel format, abort.
	webcamTexture->PostEditChange();
	webcamTexture->UpdateResource();
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



