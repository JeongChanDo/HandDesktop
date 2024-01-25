// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaze.h"

Blaze::Blaze()
{
	this->blazePalm = cv::dnn::readNet("c:/blazepalm_old.onnx");
	this->blazeHand = cv::dnn::readNet("c:/blazehand.onnx");
}

Blaze::~Blaze()
{
}


void Blaze::ResizeAndPad(
	cv::Mat& srcimg, cv::Mat& img256,
	cv::Mat& img128, float& scale, cv::Scalar& pad
)
{
    float h1, w1;
    int padw, padh;

    cv::Size size0 = srcimg.size();
    if (size0.height >= size0.width) {
        h1 = 256;
        w1 = 256 * size0.width / size0.height;
        padh = 0;
        padw = static_cast<int>(256 - w1);
        scale = size0.width / static_cast<float>(w1);
    }
    else {
        h1 = 256 * size0.height / size0.width;
        w1 = 256;
        padh = static_cast<int>(256 - h1);
        padw = 0;
        scale = size0.height / static_cast<float>(h1);
    }

    //UE_LOG(LogTemp, Log, TEXT("scale value: %f, size0.height: %d, size0.width : %d, h1 : %f"), scale, size0.height, size0.width, h1);

    int padh1 = static_cast<int>(padh / 2);
    int padh2 = static_cast<int>(padh / 2) + static_cast<int>(padh % 2);
    int padw1 = static_cast<int>(padw / 2);
    int padw2 = static_cast<int>(padw / 2) + static_cast<int>(padw % 2);
    pad = cv::Scalar(static_cast<float>(padh1 * scale), static_cast<float>(padw1 * scale));

    
    cv::resize(srcimg, img256, cv::Size(w1, h1));
    cv::copyMakeBorder(img256, img256, padh1, padh2, padw1, padw2, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    cv::resize(img256, img128, cv::Size(128, 128));

    /*
    // img256을 srcimg의 (0, 0) 좌표에 그리기
    cv::Mat roi1 = srcimg(cv::Rect(0, 0, img256.cols, img256.rows));
    img256.copyTo(roi1);

    // img128를 srcimg의 (300, 0) 좌표에 그리기
    cv::Mat roi2 = srcimg(cv::Rect(300, 0, img128.cols, img128.rows));
    img128.copyTo(roi2);
    */
}



std::vector<Blaze::PalmDetection> Blaze::PredictPalmDetections(cv::Mat& img)
{
    std::vector<Blaze::PalmDetection> beforeNMSResults;
    std::vector<Blaze::PalmDetection> afterNMSResults;
    std::vector<float> scores;
    std::vector<int> indices;
    std::vector<cv::Rect> boundingBoxes;


    cv::Mat inputImg;
    cv::cvtColor(img, inputImg, cv::COLOR_BGR2RGB);

    cv::Mat tensor;
    inputImg.convertTo(tensor, CV_32F, 1 / 127.5, -1.0);
    cv::Mat blob = cv::dnn::blobFromImage(tensor, 1.0, tensor.size(), 0, false, false, CV_32F);
    std::vector<cv::String> outNames(2);
    outNames[0] = "regressors";
    outNames[1] = "classificators";

    blazePalm.setInput(blob);
    std::vector<cv::Mat> outputs;
    blazePalm.forward(outputs, outNames);

    cv::Mat classificator = outputs[0];
    cv::Mat regressor = outputs[1];


    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            for (int a = 0; a < 2; ++a) {
                PalmDetection res = GetPalmDetection(regressor, classificator, 8, 2, x, y, a, 0);
                if (res.score != 0)
                {
                    beforeNMSResults.push_back(res);

                    cv::Point2d startPt = cv::Point2d(res.xmin, res.ymin);
                    cv::Point2d endPt = cv::Point2d(res.xmax, res.ymax);

                    boundingBoxes.push_back(cv::Rect(startPt, endPt));
                    scores.push_back(res.score);
                }
            }
        }
    }

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            for (int a = 0; a < 6; ++a) {
                PalmDetection res = GetPalmDetection(regressor, classificator, 16, 6, x, y, a, 512);
                if (res.score != 0)
                {
                    beforeNMSResults.push_back(res);
                    cv::Point2d startPt = cv::Point2d(res.xmin, res.ymin);
                    cv::Point2d endPt = cv::Point2d(res.xmax, res.ymax);

                    boundingBoxes.push_back(cv::Rect(startPt, endPt));
                    scores.push_back(res.score);
                }
            }
        }
    }


    cv::dnn::NMSBoxes(boundingBoxes, scores, palmMinScoreThresh, palmMinNMSThresh, indices);

    for (int i = 0; i < indices.size(); i++) {
        int idx = indices[i];
        afterNMSResults.push_back(beforeNMSResults[idx]);
    }

    return afterNMSResults;
}

Blaze::PalmDetection Blaze::GetPalmDetection(cv::Mat regressor, cv::Mat classificator,
    int stride, int anchor_count, int column, int row, int anchor, int offset) {

    Blaze::PalmDetection res;

    int index = (int(row * 128 / stride) + column) * anchor_count + anchor + offset;
    float origin_score = regressor.at<float>(0, index, 0);
    float score = sigmoid(origin_score);
    if (score < palmMinScoreThresh) return res;

    float x = classificator.at<float>(0, index, 0);
    float y = classificator.at<float>(0, index, 1);
    float w = classificator.at<float>(0, index, 2);
    float h = classificator.at<float>(0, index, 3);

    x += (column + 0.5) * stride - w / 2;
    y += (row + 0.5) * stride - h / 2;

    res.ymin = (y) / blazePalmSize;
    res.xmin = (x) / blazePalmSize;
    res.ymax = (y + h) / blazePalmSize;
    res.xmax = (x + w) / blazePalmSize;

    if ((res.ymin < 0) || (res.xmin < 0) || (res.xmax > 1) || (res.ymax > 1)) return res;

    res.score = score;

    std::vector<cv::Point2d> kpts;
    for (int key_id = 0; key_id < palmMinNumKeyPoints; key_id++)
    {
        float kpt_x = classificator.at<float>(0, index, 4 + key_id * 2);
        float kpt_y = classificator.at<float>(0, index, 5 + key_id * 2);
        kpt_x += (column + 0.5) * stride;
        kpt_x  = kpt_x / blazePalmSize;
        kpt_y += (row + 0.5) * stride;
        kpt_y  = kpt_y / blazePalmSize;
        //UE_LOG(LogTemp, Log, TEXT("kpt id(%d) : (%f, %f)"), key_id, kpt_x, kpt_y);
        res.kp_arr[key_id] = cv::Point2d(kpt_x, kpt_y);

    }
    return res;
}

float Blaze::sigmoid(float x) {
    return 1 / (1 + exp(-x));
}


std::vector<Blaze::PalmDetection> Blaze::DenormalizePalmDetections(std::vector<Blaze::PalmDetection> detections, int width, int height, cv::Scalar pad)
{

    std::vector<Blaze::PalmDetection> denormDets;

    int scale = 0;
    if (width > height)
        scale = width;
    else
        scale = height;

    for (auto& det : detections)
    {
        Blaze::PalmDetection denormDet;
        denormDet.ymin = det.ymin * scale - pad[0];
        denormDet.xmin = det.xmin * scale - pad[1];
        denormDet.ymax = det.ymax * scale - pad[0];
        denormDet.xmax = det.xmax * scale - pad[1];
        denormDet.score = det.score;

        for (int i = 0; i < palmMinNumKeyPoints; i++)
        {
            cv::Point2d pt_new = cv::Point2d(det.kp_arr[i].x * scale - pad[1], det.kp_arr[i].y * scale - pad[0]);
            //UE_LOG(LogTemp, Log, TEXT("denorm kpt id(%d) : (%f, %f)"), i, pt_new.x, pt_new.y);
            denormDet.kp_arr[i] = pt_new;
        }
        denormDets.push_back(denormDet);
    }
    return denormDets;
}

void Blaze::DrawPalmDetections(cv::Mat& img, std::vector<Blaze::PalmDetection> denormDets)
{
    for (auto& denormDet : denormDets)
    {
        cv::Point2d startPt = cv::Point2d(denormDet.xmin, denormDet.ymin);
        cv::Point2d endPt = cv::Point2d(denormDet.xmax, denormDet.ymax);
        cv::rectangle(img, cv::Rect(startPt, endPt), cv::Scalar(255, 0, 0), 1);

        for (int i = 0; i < palmMinNumKeyPoints; i++)
            cv::circle(img, denormDet.kp_arr[i], 5, cv::Scalar(255, 0, 0), -1);

        std::string score_str = std::to_string(static_cast<int>(denormDet.score * 100))+ "%";
        cv::putText(img, score_str, cv::Point(startPt.x, startPt.y - 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

    }
}

std::vector<Blaze::PalmDetection> Blaze::FilteringDets(std::vector<Blaze::PalmDetection> detections, int width, int height)
{
    std::vector<Blaze::PalmDetection> filteredDets;

    for (auto& denormDet : detections)
    {
        cv::Point2d startPt = cv::Point2d(denormDet.xmin, denormDet.ymin);
        if (startPt.x < 10 || startPt.y < 10)
            continue;
        if (startPt.x > width || startPt.y > height)
            continue;
        int w = denormDet.xmax - denormDet.xmin;
        int y = denormDet.ymax - denormDet.ymin;
        if ((w * y < 50 * 40 )|| (w * y  > (width * 0.7) * (height * 0.7)))
            continue;
        filteredDets.push_back(denormDet);
    }
    return filteredDets;
}


void Blaze::Detections2ROI(std::vector<Blaze::PalmDetection> dets, std::vector<float>& vec_xc, std::vector<float>& vec_yc, std::vector<float>& vec_scale, std::vector<float>& vec_theta)
{
    for (int i = 0; i < dets.size(); i++)
    {
        Blaze::PalmDetection det = dets.at(i);
        float xc = (det.xmax + det.xmin) / 2;
        float yc = (det.ymax + det.ymin) / 2;
        float scale = blazePalmDScale;
        /*
        float scale = (det.xmax - det.xmin); //assumes square box

        yc = blazePalmDy * scale;
        scale *= blazePalmDScale;
        */

        //compute box rot
        float theta = std::atan2(det.kp_arr[0].y - det.kp_arr[1].y, det.kp_arr[0].x - det.kp_arr[1].x) - blazePalmTheta0;

        vec_xc.push_back(xc);
        vec_yc.push_back(yc);
        vec_scale.push_back(scale);
        vec_theta.push_back(theta);
    }
}


void Blaze::extract_roi(cv::Mat frame, std::vector<float>& vec_xc, std::vector<float>& vec_yc, 
    std::vector<float>& vec_scale, std::vector<float>& vec_theta, std::vector<Blaze::PalmDetection> denormDets,
    std::vector<cv::Mat>& vec_img, std::vector<cv::Mat>& vec_affine, std::vector<BoxROI>& vec_boxROI)
{
    cv::Mat img = frame.clone();
    for (int i = 0; i < vec_xc.size(); i++)
    {
        cv::Point2f center = cv::Point2f(vec_xc.at(i), vec_yc.at(i));

        Blaze::PalmDetection denormDet = denormDets.at(i);
        float theta = vec_theta.at(i);
        theta = theta * 180 / CV_PI;
        float scale = vec_scale.at(i);




        // 회전 변환 행렬 계산
        cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, theta, 1.0);


        // 원래 detection 꼭지점
        cv::Point2f originaDetPoints[4] = {
            cv::Point2f(denormDet.xmin, denormDet.ymin),
            cv::Point2f(denormDet.xmax, denormDet.ymin),
            cv::Point2f(denormDet.xmax, denormDet.ymax),
            cv::Point2f(denormDet.xmin, denormDet.ymax)
        };

        Blaze::BoxROI roi;
        for (int j = 0; j < 4; j++)
            roi.pts[j] = originaDetPoints[j];


        // 회전된 상자의 꼭지점 계산
        cv::Point2f startRotatedPoints[4];

        // 목표 꼭지점(블라즈 핸드는 256이므로
        cv::Point2f targetRotatedPoint[4] = {
            cv::Point2f(0, 0),
            cv::Point2f(256, 0),
            cv::Point2f(256, 256),
            cv::Point2f(0, 256)

        };

        for (int j = 0; j < 4; j++)
        {
            cv::Point2f startRotatedPoint = originaDetPoints[j] - center;
            float x = startRotatedPoint.x * std::cos(theta * CV_PI / 180) - startRotatedPoint.y * std::sin(theta * CV_PI / 180);
            float y = startRotatedPoint.x * std::sin(theta * CV_PI / 180) + startRotatedPoint.y * std::cos(theta * CV_PI / 180);
            x = x * scale;
            y = y * scale;
            startRotatedPoints[j] = cv::Point2f(x, y) + center;
        }

        // 어파인 변환 행렬 계산
        cv::Mat affineTransform_Mat = cv::getAffineTransform(startRotatedPoints, targetRotatedPoint);
        cv::Mat inv_affine_tranform_Mat;
        cv::invertAffineTransform(affineTransform_Mat, inv_affine_tranform_Mat);

        // 정 어파인변환 행렬로 이미지 변환
        cv::Mat affine_img;
        cv::warpAffine(img, affine_img, affineTransform_Mat, cv::Size(blazeHandSize, blazeHandSize));




        /*

        std::ostringstream oss_info;
        oss_info << "center(" << center.x << "," << center.y << "), scale : " << scale;
        std::string oss_info_str = oss_info.str();
        cv::putText(frame, oss_info_str, cv::Point(30, 370), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 2);



        std::ostringstream oss_originaDetPoints;
        oss_originaDetPoints << "originaDetPoints pt0(" << originaDetPoints[0].x << "," << originaDetPoints[0].y << ")"
                << ", pt1(" << originaDetPoints[1].x << ", " << originaDetPoints[1].y << ")";
        std::string originaDetPoints_str = oss_originaDetPoints.str();
        cv::putText(frame, originaDetPoints_str, cv::Point(30, 400), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 2);

        std::ostringstream oss_startRotatedPoints;
        oss_startRotatedPoints << "startRotatedPoint pt0(" << startRotatedPoints[0].x << "," << startRotatedPoints[0].y<<")"
            << ", pt1(" << startRotatedPoints[1].x << ", " << startRotatedPoints[1].y << ")";

        std::string startRotatedPoints_str = oss_startRotatedPoints.str();
        cv::putText(frame, startRotatedPoints_str, cv::Point(30, 420), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 2);
        */




        vec_img.push_back(affine_img);
        vec_affine.push_back(inv_affine_tranform_Mat);
        vec_boxROI.push_back(roi);
    }
}
