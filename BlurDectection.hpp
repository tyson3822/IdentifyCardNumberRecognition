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
    Laplacian(src, lap, CV_64F);

    Scalar mu, sigma;
    meanStdDev(lap, mu, sigma);

    double focusMeasure = sigma.val[0]*sigma.val[0];
    return focusMeasure;
}

void BlurDectect(Mat img)
{
    Mat imgGary;
    cvtColor(img, imgGary, CV_BGR2GRAY);
    float result = VarianceOfLaplacian(imgGary);
    char resultText[100] = "Not Blurry";
    if(result < 100)
        strcpy(resultText, "Blurry");

    cout << "the image is " << result << ", so that image is " << resultText << endl;
}
