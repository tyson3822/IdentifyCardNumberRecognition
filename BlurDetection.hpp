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

double VarianceOfLaplacian(const Mat& src)
{
    Mat lap;
    //執行laplace演算法
    Laplacian( src, lap, CV_16S, 3, 1, 0, BORDER_DEFAULT );//CV_64F
    convertScaleAbs( lap, lap );
    imshow("Laplacian", lap);

    Scalar mu, sigma;
    meanStdDev(lap, mu, sigma);

    //回傳值計算
    double focusMeasure = sigma.val[0]*sigma.val[0];
    return focusMeasure;
}

bool BlurDectect(Mat img)
{
    Mat imgGary;
    cvtColor(img, imgGary, CV_BGR2GRAY);
    bool isBlurry = false;
    float result = VarianceOfLaplacian(imgGary);
    char resultText[100] = "Not Blurry";

    //偵測到模糊
    if(result < 300)
    {
        strcpy(resultText, "Blurry");
        isBlurry = true;
    }

    cout << "the image is " << result << ", so that image is " << resultText << endl;
    return isBlurry;
}
