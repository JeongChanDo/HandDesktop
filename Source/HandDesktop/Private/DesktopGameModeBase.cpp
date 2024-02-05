// Fill out your copyright notice in the Description page of Project Settings.

#include "DesktopGameModeBase.h"


void ADesktopGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	blaze = Blaze();

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
	capture.set(cv::CAP_PROP_FRAME_WIDTH, webcamWidth);
	capture.set(cv::CAP_PROP_FRAME_HEIGHT, webcamHeight);


	webcamTexture = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);


	imageScreen1 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageScreen2 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageScreen3 = cv::Mat(monitorHeight, monitorWidth, CV_8UC4);
	imageTextureScreen1 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);
	imageTextureScreen2 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);
	imageTextureScreen3 = UTexture2D::CreateTransient(monitorWidth, monitorHeight, PF_B8G8R8A8);
	
	make_map_bone();
	make_map_for_location();
	make_map_bone_name();
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





	std::vector<cv::Rect> handRects = blaze.convertHandRects(filteredDets);
	std::vector<cv::Mat> handImgs;
	blaze.GetHandImages(webcamImage, handRects, handImgs);
	std::vector<cv::Mat> imgs_landmarks = blaze.PredictHandDetections(handImgs);
	std::vector<cv::Mat> denorm_imgs_landmarks = blaze.DenormalizeHandLandmarksForBoneLocation(imgs_landmarks, handRects);

	check_hand_exist(imgs_landmarks);
	//make_map_for_rotators(denorm_imgs_landmarks);
	set_map_for_location();
	set_hand_pos_world();

	//draw hand rects/ plam detection/ dets info/ hand detection
	blaze.DrawRects(webcamImage, handRects);
	blaze.DrawPalmDetections(webcamImage, filteredDets);
	blaze.DrawDetsInfo(webcamImage, filteredDets, normDets, denormDets);
	blaze.DrawHandDetections(webcamImage, denorm_imgs_landmarks);

	//cv::mat to utexture2d
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





void ADesktopGameModeBase::get_pitch_yaw(cv::Point3f pt_start, cv::Point3f pt_end, float& pitch, float& yaw) {
	float dx = pt_end.x - pt_start.x;
	float dy = pt_end.y - pt_start.y;
	dy *= -1;
	yaw = std::atan2(dy, dx) * 180 / CV_PI;
	yaw = yaw - 90;

	float dz = pt_end.z - pt_start.z;
	float xy_norm = std::sqrt(dx * dx + dy * dy);
	pitch = std::atan2(dz, xy_norm) * 180 / CV_PI;
	pitch *= -1;
}

/*
void ADesktopGameModeBase::calculateRotation(const cv::Point3f& pt1, const cv::Point3f& pt2, float& roll, float& pitch, float& yaw) {
	cv::Point3f vec = pt2 - pt1;  // 두 벡터를 연결하는 벡터 계산

	roll = atan2(vec.y, vec.x) * 180 / CV_PI;  // 롤(Roll) 회전 계산
	pitch = asin(vec.z / cv::norm(vec)) * 180 / CV_PI;  // 피치(Pitch) 회전 계산

	cv::Point2f vecXY(vec.x, vec.y);  // xy 평면으로 투영
	yaw = (atan2(vec.y, vec.x) - atan2(vec.z, cv::norm(vecXY))) * 180 / CV_PI;  // 요(Yaw) 회전 계산
}
*/



void ADesktopGameModeBase::make_map_for_rotators(std::vector<cv::Mat> denorm_imgs_landmarks)
{
	for (auto& denorm_landmarks : denorm_imgs_landmarks)
	{
		std::vector<std::array<int, 2>> HAND_CONNECTIONS = blaze.HAND_CONNECTIONS;
		for (auto& hand_conns_index : hand_conns_indexes)
		{
			std::array<int, 2> hand_conns = HAND_CONNECTIONS.at(hand_conns_index);
			float roll, pitch, yaw;
			cv::Point3f pt_start, pt_end;

			pt_start.x = denorm_landmarks.at<float>(hand_conns.at(0), 0);
			pt_start.y = denorm_landmarks.at<float>(hand_conns.at(0), 1);
			pt_start.z = denorm_landmarks.at<float>(hand_conns.at(0), 2);

			pt_end.x = denorm_landmarks.at<float>(hand_conns.at(1), 0);
			pt_end.y = denorm_landmarks.at<float>(hand_conns.at(1), 1);
			pt_end.z = denorm_landmarks.at<float>(hand_conns.at(1), 2);


			//calculateRotation(pt_start, pt_end, roll, pitch, yaw);

			get_pitch_yaw(pt_start, pt_end, pitch, yaw);
			MapRoll.Add(hand_conns_index, roll);
			MapPitch.Add(hand_conns_index, pitch);
			MapYaw.Add(hand_conns_index, yaw);

		}
		HandLeftX = denorm_landmarks.at<float>(9, 0);
		HandLeftY = denorm_landmarks.at<float>(9, 1);

	}
}


/*
	std::vector<std::array<int, 2>> HAND_CONNECTIONS = {
	{0, 1}, {1, 2}, {2, 3}, {3, 4},
	{5, 6}, {6, 7}, {7, 8},
	{9, 10}, {10, 11}, {11, 12},
	{13, 14}, {14, 15}, {15, 16},
	{17, 18}, {18, 19}, {19, 20},
	{0, 5}, {5, 9}, {9, 13}, {13, 17}, {0, 17}
	};
*/


void ADesktopGameModeBase::make_map_bone()
{
	MapBoneLeft.Add(2, FString("b_l_thumb2"));
	MapBoneLeft.Add(3, FString("b_l_thumb3"));
	MapBoneLeft.Add(4, FString("b_l_index1"));
	MapBoneLeft.Add(5, FString("b_l_index2"));
	MapBoneLeft.Add(6, FString("b_l_index3"));
	MapBoneLeft.Add(7, FString("b_l_middle1"));
	MapBoneLeft.Add(8, FString("b_l_middle2"));
	MapBoneLeft.Add(9, FString("b_l_middle3"));
	MapBoneLeft.Add(10, FString("b_l_ring1"));
	MapBoneLeft.Add(11, FString("b_l_ring2"));
	MapBoneLeft.Add(12, FString("b_l_ring3"));
	MapBoneLeft.Add(13, FString("b_l_pinky1"));
	MapBoneLeft.Add(14, FString("b_l_pinky2"));
	MapBoneLeft.Add(15, FString("b_l_pinky3"));


	MapBoneRight.Add(2, FString("b_r_thumb2"));
	MapBoneRight.Add(3, FString("b_r_thumb3"));
	MapBoneRight.Add(4, FString("b_r_index1"));
	MapBoneRight.Add(5, FString("b_r_index2"));
	MapBoneRight.Add(6, FString("b_r_index3"));
	MapBoneRight.Add(7, FString("b_r_middle1"));
	MapBoneRight.Add(8, FString("b_r_middle2"));
	MapBoneRight.Add(9, FString("b_r_middle3"));
	MapBoneRight.Add(10, FString("b_r_ring1"));
	MapBoneRight.Add(11, FString("b_r_ring2"));
	MapBoneRight.Add(12, FString("b_r_ring3"));
	MapBoneRight.Add(13, FString("b_r_pinky1"));
	MapBoneRight.Add(14, FString("b_r_pinky2"));
	MapBoneRight.Add(15, FString("b_r_pinky3"));



	MapBoneLeft.Add(16, FString("b_l_wrist"));
	MapBoneLeft.Add(17, FString("b_l_thumb0"));
	MapBoneLeft.Add(18, FString("b_l_thumb1"));
	MapBoneLeft.Add(19, FString("b_l_pinky0"));
	MapBoneRight.Add(16, FString("b_r_wrist"));
	MapBoneRight.Add(17, FString("b_r_thumb0"));
	MapBoneRight.Add(18, FString("b_r_thumb1"));
	MapBoneRight.Add(19, FString("b_r_pinky0"));
}





void ADesktopGameModeBase::set_map_for_location()
{
	cv::Mat HandLeftLocMat = blaze.handLeft;
	cv::Mat HandRightLocMat = blaze.handRight;


	//UE_LOG(LogTemp, Log, TEXT("rows : %d, cols : %d"), HandLeftLocMat.size[0], HandLeftLocMat.size[1]);

	for (int j = 0; j < HandLeftLocMat.size[0]; j++)
	{
		FVector vec(HandLeftLocMat.at<float>(j, 0), HandLeftLocMat.at<float>(j, 1), HandLeftLocMat.at<float>(j, 2));
		MapBoneLocationLeft.Add(j, vec);

	}
	for (int j = 0; j < HandRightLocMat.size[0]; j++)
	{
		FVector vec(HandRightLocMat.at<float>(j, 0), HandRightLocMat.at<float>(j, 1), HandRightLocMat.at<float>(j, 2));
		MapBoneLocationRight.Add(j, vec);
	}
	
	//UE_LOG(LogTemp, Log, TEXT("HandRightLocMat.size[0] : %d"), HandRightLocMat.size[0]);

}



void ADesktopGameModeBase::make_map_for_location()
{

	for (int j = 0; j < 21; j++)
	{
		MapBoneLocationLeft.Add(j, FVector(0, 0, 0));
		MapBoneLocationRight.Add(j, FVector(0, 0, 0));
	}
}



void ADesktopGameModeBase::make_map_bone_name()
{
	MapBoneLocationNameLeft.Add(0, FString("b_l_wrist"));
	MapBoneLocationNameLeft.Add(1, FString("b_l_thumb0"));
	MapBoneLocationNameLeft.Add(2, FString("b_l_thumb1"));
	MapBoneLocationNameLeft.Add(3, FString("b_l_thumb2"));
	MapBoneLocationNameLeft.Add(4, FString("b_l_thumb3"));
	MapBoneLocationNameLeft.Add(5, FString("b_l_index1"));
	MapBoneLocationNameLeft.Add(6, FString("b_l_index2"));
	MapBoneLocationNameLeft.Add(7, FString("b_l_index3"));
	MapBoneLocationNameLeft.Add(9, FString("b_l_middle1"));
	MapBoneLocationNameLeft.Add(10, FString("b_l_middle2"));
	MapBoneLocationNameLeft.Add(11, FString("b_l_middle3"));
	MapBoneLocationNameLeft.Add(13, FString("b_l_ring1"));
	MapBoneLocationNameLeft.Add(14, FString("b_l_ring2"));
	MapBoneLocationNameLeft.Add(15, FString("b_l_ring3"));
	MapBoneLocationNameLeft.Add(17, FString("b_l_pinky1"));
	MapBoneLocationNameLeft.Add(18, FString("b_l_pinky2"));
	MapBoneLocationNameLeft.Add(19, FString("b_l_pinky3"));


	MapBoneLocationNameRight.Add(0, FString("b_r_wrist"));

	MapBoneLocationNameRight.Add(1, FString("b_r_thumb0"));
	MapBoneLocationNameRight.Add(2, FString("b_r_thumb1"));
	MapBoneLocationNameRight.Add(3, FString("b_r_thumb2"));
	MapBoneLocationNameRight.Add(4, FString("b_r_thumb3"));

	MapBoneLocationNameRight.Add(5, FString("b_r_index1"));
	MapBoneLocationNameRight.Add(6, FString("b_r_index2"));
	MapBoneLocationNameRight.Add(7, FString("b_r_index3"));

	MapBoneLocationNameRight.Add(9, FString("b_r_middle1"));
	MapBoneLocationNameRight.Add(10, FString("b_r_middle2"));
	MapBoneLocationNameRight.Add(11, FString("b_r_middle3"));

	MapBoneLocationNameRight.Add(13, FString("b_r_ring1"));
	MapBoneLocationNameRight.Add(14, FString("b_r_ring2"));
	MapBoneLocationNameRight.Add(15, FString("b_r_ring3"));

	MapBoneLocationNameRight.Add(17, FString("b_r_pinky1"));
	MapBoneLocationNameRight.Add(18, FString("b_r_pinky2"));
	MapBoneLocationNameRight.Add(19, FString("b_r_pinky3"));
}


void ADesktopGameModeBase::set_hand_pos_world()
{
	cv::Mat handLeft = blaze.handLeftImg;
	cv::Mat handRight = blaze.handRightImg;

	if (handLeft.size[0] == 21)
	{
		HandLeftX = handLeft.at<float>(9, 0);
		HandLeftY = handLeft.at<float>(9, 1);
	}
	if (handRight.size[0] == 21)
	{
		HandRightX = handRight.at<float>(9, 0);
		HandRightY = handRight.at<float>(9, 1);
	}


}


void ADesktopGameModeBase::check_hand_exist(std::vector<cv::Mat>& imgs_landmarks)
{
	if (imgs_landmarks.size() == 0)
	{
		isExistLeft = false;
		isExistRight = false;
	}
	else
	{
		isExistLeft = blaze.isExistLeft;
		isExistRight = blaze.isExistRight;
	}
}
