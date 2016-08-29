#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <cctype>

using namespace std;
using namespace cv;

//模糊值計算 採用laplace演算法
//類似canny也是找尋輪廓的函數
//找到的輪廓會設為顏色白色255 否則黑色0
//藉由計算整個影像的標準差來判斷這個影像找到的輪廓或多或少
//通常數值越小代表影像越模糊
//所以制定計算值在300內的影像為模糊

//模糊值計算 採用laplace演算法
double VarianceOfLaplacian(const Mat& src)
{
    Mat lap;
    //執行laplace演算法
    Laplacian( src, lap, CV_16S, 3, 1, 0, BORDER_DEFAULT );
    convertScaleAbs( lap, lap );
    //imshow("Laplacian", lap);

    Scalar mu, sigma;
    meanStdDev(lap, mu, sigma);

    //回傳值計算
    double focusMeasure = sigma.val[0] * sigma.val[0];
    return focusMeasure;
}

float BlurDectect(Mat img)
{
    Mat imgGary;
    cvtColor(img, imgGary, CV_BGR2GRAY);
    bool isBlurry = false;
    float result = VarianceOfLaplacian(imgGary);
    char resultText[100] = "Not Blurry";

    //偵測到模糊
    if(result < 300 && result >= 0)
    {
        strcpy(resultText, "Blurry");
        isBlurry = true;
    }

    //cout << "the image's variance of Laplacian = " << result << ", so that image is " << resultText << endl;
    return result;
}
