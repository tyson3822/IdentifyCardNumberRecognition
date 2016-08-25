#include <iostream>
#include <stdio.h>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;

//（main中使用）計算反光值，輸入影像為三通道
int CalculateReflectionValue(Mat& in)
{
    int reflectionValue;

    //將影像進行灰階
    Mat grayImg;
    cvtColor(in, grayImg, CV_BGR2GRAY);

    //計算平均值
    Scalar mean, sigma;
    meanStdDev(grayImg, mean, sigma);

    //兩種對統計圖動作的函數
    //equalizeHist( grayImg, grayImg );
    //normalize(grayImg, grayImg, 0, 255, NORM_MINMAX);

    //指定閥值，進行二值化
    int thresh = 1.2 * mean.val[0];
    int highValue = 255;
    int lowValue = 0;
    grayImg.setTo(lowValue, grayImg< thresh);
    grayImg.setTo(highValue, grayImg >= thresh);

    //再次計算平均值（二值化的成像）
    meanStdDev(grayImg, mean, sigma);

    //回傳值計算
    reflectionValue = mean.val[0];
    cout << "reflection value = " << reflectionValue << endl;
    return reflectionValue;
}

//（有使用在GetCardMat）利用灰階來濾波，輸入為單通道
void BinaryFilterByThresh(Mat& in, Mat& out)
{
    Mat grayImg = in.clone();
    //cvtColor(in, grayImg, CV_BGR2GRAY);

    //計算平均值
    Scalar mean, stddev;
    meanStdDev(grayImg, mean, stddev);

    //指定濾波閥值
    int thresh = mean.val[0] * 0.85;//有equalizeHist 0.2
    cout << "thresh = " << thresh << endl;

    //進行濾波
    grayImg.setTo(0, grayImg < thresh);
    grayImg.setTo(255, grayImg >= thresh);

    //輸出影像
    imshow("grayImg", grayImg);
    out = grayImg.clone();
}

//////////////////////////////////////////////////////
//以下是沒使用但日後可以參考或擴增的功能//
//////////////////////////////////////////////////////

//（沒用到）簡單的利用三通道進行色彩平衡
void ColorBalance(Mat& in, Mat& out, float percent)
{
    assert(in.channels() == 3);
    assert(percent > 0 && percent < 100);

    float half_percent = percent / 200.0f;

    vector<Mat> tmpsplit; split(in,tmpsplit);
    for(int i=0;i<3;i++) {
        //find the low and high precentile values (based on the input percentile)
        Mat flat; tmpsplit[i].reshape(1,1).copyTo(flat);
        cv::sort(flat,flat,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
        int lowval = flat.at<uchar>(cvFloor(((float)flat.cols) * half_percent));
        int highval = flat.at<uchar>(cvCeil(((float)flat.cols) * (1.0 - half_percent)));
        //cout << "lowval = " << lowval << ", highval = " << highval << endl;

        //saturate below the low percentile and above the high percentile
        tmpsplit[i].setTo(lowval,tmpsplit[i] < lowval);
        tmpsplit[i].setTo(highval,tmpsplit[i] > highval);

        //scale the channel
        normalize(tmpsplit[i],tmpsplit[i],0,255,NORM_MINMAX);
    }
    merge(tmpsplit,out);
}

//原是用在sort排序用的struct
struct myclass
{
    bool operator() (cv::Point pt1, cv::Point pt2) { return (pt1.y < pt2.y);}
} myobject;

//（沒使用＆失敗）尋找統計圖上第一個出現最接近平均值index
int FindFirstMeanIndex(Mat &in)
{
    int meanIndex;
    Scalar mean, stddev;
    meanStdDev(in, mean, stddev);
    vector<Point> meanSub;

    Mat imgInTemp;
    in.reshape(1,1).copyTo(imgInTemp);

    for(int index = 0; index < imgInTemp.cols; index++)
    {
        Scalar value = imgInTemp.at<uchar>(index);
        meanSub.push_back(Point(index, abs(mean.val[0] - value.val[0])));
        //cout << "meanSub[" << index<< "] = " << meanSub[index]  << endl;
    }
//    cout << "FindFirstMeanIndex for mean index 12148= " << imgInTemp.at<int>(12148) << endl;
//    cout << "FindFirstMeanIndex for mean index 12149= " << imgInTemp.at<int>(12149) << endl;
//    cout << "FindFirstMeanIndex for mean index 4216= " << imgInTemp.at<int>(4216) << endl;
//    cout << "FindFirstMeanIndex for mean index 12151= " << imgInTemp.at<int>(12151) << endl;
//    cout << "FindFirstMeanIndex for mean index 12152= " << imgInTemp.at<int>(12152) << endl;

    sort(meanSub.begin(), meanSub.end(), myobject);

//    meanIndex = meanSub[0].x;
    //cout << "the most less sub is " << meanSub[0].y << endl;
    for(int i = 0; i < 10; i++)
    {
        meanIndex = meanSub[i].x;
        Scalar value = imgInTemp.at<int>(meanIndex);
        cout << "mean = " << value.val[i] << endl;
    }
//    Scalar value = imgInTemp.at<uchar>(meanIndex);
//    cout << "mean = " << value.val[0] << endl;

    return meanIndex;
}

//（沒使用）本來是打算用R通道來濾波，只取紅色的身份證字號，但效果沒有灰階好
void ColorFilterRed(Mat& in, Mat& out)
{
    assert(in.channels() == 3);
//    assert(percent > 0 && percent < 100);
//
//    float half_percent = 1 / 200.0f;

    Mat grayImg;
    cvtColor(in, grayImg, CV_BGR2GRAY);

    vector<Mat> tmpsplit;
    split(in,tmpsplit);

    //跑三個通道
    for(int i=0;i<3;i++) {
        //find the low and high precentile values (based on the input percentile)
        Mat flat;
        tmpsplit[i].reshape(1,1).copyTo(flat);
//        cv::sort(flat,flat,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
//        int lowval = flat.at<uchar>(cvFloor(((float)flat.cols) * half_percent));
//        int highval = flat.at<uchar>(cvCeil(((float)flat.cols) * (1.0 - half_percent)));
//        cout << "half_percent" << half_percent << endl;
//        cout << "lowval = flat.at<uchar>(cvFloor((" << (float)flat.cols << " ) *" << half_percent << ")); = " << lowval  << endl
//        << ", highval = flat.at<uchar>(cvCeil((" << (float)flat.cols << " ) * (1.0 - " << half_percent << "))); = " << highval << endl;

        //saturate below the low percentile and above the high percentile  (BGR)
        if(i < 2)
        {
            tmpsplit[i].setTo(255,tmpsplit[i] < 255);
        }
        else
        {
            //cout << "flat = " << flat << endl;
            //imshow("before", tmpsplit[i]);

//            tmpsplit[i].setTo(lowval,tmpsplit[i] < lowval);
//            tmpsplit[i].setTo(highval,tmpsplit[i] > highval);
//            normalize(tmpsplit[i],tmpsplit[i], 0, 255, NORM_MINMAX);

            Scalar mean, stddev;
            meanStdDev(flat, mean, stddev);
//            cout <<  "mean.val[0] = "  << mean.val[0] << endl;
//            cout <<  "stddev.val[0] = "  << stddev.val[0] << endl;

            //equalizeHist( tmpsplit[i], tmpsplit[i] );
            normalize(tmpsplit[i], tmpsplit[i], 0, 255, NORM_MINMAX);
            //imshow("after", tmpsplit[i]);


////////////////////            Mat sortGrayImg = tmpsplit[i].clone();
////////////////////            cv::sort(sortGrayImg, sortGrayImg, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
////////////////////            int meanIndex = FindFirstMeanIndex(sortGrayImg);
////////////////////            cout << "meanIndex = " << meanIndex << endl;
////////////////////            meanIndex -= 50;
////////////////////            int thresh = sortGrayImg.at<uchar>(meanIndex);

            int thresh = mean.val[0] * 0.85;//有equalizeHist 0.2
            //2雜訊變多
            //1.4不錯

            //原1.25
            cout << "thresh = " << thresh << endl;
            tmpsplit[i].setTo(0, tmpsplit[i] < thresh);
            tmpsplit[i].setTo(255, tmpsplit[i] >= thresh);
            imshow("tmpsplit[2]", tmpsplit[2]);
            //tmpsplit[i].setTo(0,tmpsplit[i] < 255);
            grayImg.setTo(0, grayImg < thresh);
            grayImg.setTo(255, grayImg >= thresh);
            imshow("grayImg", grayImg);



        }
//        tmpsplit[i].setTo(lowval,tmpsplit[i] < lowval);
//        tmpsplit[i].setTo(highval,tmpsplit[i] > highval);
        //scale the channel
        //normalize(tmpsplit[i],tmpsplit[i],0,255,NORM_MINMAX);
    }
    //merge(tmpsplit,out);
    out = grayImg.clone();
    //imshow("ColorFilterRed", out);
}

//http://docs.opencv.org/2.4/modules/core/doc/basic_structures.html#mat-reshape

//    float sz[5] = {1,2,3,4,5};
//    Mat testMat(1, 5, CV_32F, sz);
//    cout << "testMat = " << testMat << endl;
//
//    Scalar mean, stddev;
//    meanStdDev(testMat, mean, stddev);
//    cout <<  "mean.val[0] = "  << mean.val[0] << endl;
//    cout <<  "stddev.val[0] = "  << stddev.val[0] << endl;
//    waitKey(0);

//mean = 3;
//stddev = 1.414;
