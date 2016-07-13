#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"

#include <iostream>
#include <cctype>

#include "camshift.hpp"
#include "surfMatcher.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

int main(int argc, const char ** argv)
{
    Mat imgObject =imread( argv[1], IMREAD_COLOR );//object.png
    Mat imgScene =imread( argv[2], IMREAD_COLOR );//scene.png

    //resize(imgScene, imgScene, Size(1000, 625));
    Mat imgID =  SurfMatch(imgObject, imgScene);//切割出身份證樣本區域
    //驗證樣本區域size是否大於size(800(寬),480(高))
    resize(imgID, imgID, Size(800, 480));//大於的話就resize成較好辨識的大小；否則不辨識
    imshow("resized", imgID);
    Mat imgIdNumber = imgID(Rect(550, 400, 250, 70)).clone();
    imshow("IdNumber", imgIdNumber);

    //Mat img =imread( argv[1], IMREAD_COLOR );
    //camshift(output);
    //camshift2(output);
    waitKey(0);

    return EXIT_SUCCESS;
}
