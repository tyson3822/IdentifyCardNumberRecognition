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

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <iostream>
#include <cctype>

#include "Camshift.hpp"
#include "SurfMatcher.hpp"
#include "TestFile.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

char scenePath[] = "../scene/";
TestFile _TF;

int main(int argc, const char ** argv)
{
    //匯入檔案
    Mat imgObject = imread( argv[1], IMREAD_COLOR );//object.png
    _TF.InitTestFile(argv[2], argv[3], argv[4]);

    char* imgBuffer = new char[_TF.GetImgByIndex(0).length() + 1];
    strcpy(imgBuffer, _TF.GetImgByIndex(0).c_str());
    strcat(scenePath, imgBuffer);    //從檔案輸入scene圖片及加上路徑
    Mat imgScene = imread(scenePath , IMREAD_COLOR );  //讀取場景圖片＼

    for(int i = 0; i < _TF.GetImgVectorSize(); i++)
    {
        _TF.WriteToOutput(_TF.GetImgByIndex(i));
    }
    _TF.WriteDownOutput();


    waitKey(0);

    return EXIT_SUCCESS;

    _TF.Close();

    //resize(imgScene, imgScene, Size(1000, 625));
    Mat imgID =  SurfMatch(imgObject, imgScene);//切割出身份證樣本區域
    //驗證樣本區域size是否大於size(800(寬),480(高))
    resize(imgID, imgID, Size(800, 480));//大於的話就resize成較好辨識的大小；否則不辨識
    imshow("resized", imgID);

    //割出身份證字號樣本
    Mat imgIdNumber = imgID(Rect(580, 400, 210, 70)).clone();
    imshow("IdNumber", imgIdNumber);
    imwrite("../subNum/IdNum.png", imgIdNumber);

    //把身份證字號樣本放到較大的黑底圖上
    Mat bigSizeMat(960, 1280, CV_8UC3, Scalar::all(0));
    imgIdNumber.copyTo(bigSizeMat(Rect(100, 100, imgIdNumber.cols, imgIdNumber.rows)));
    imwrite("../subNum/bigSizeMat.png", bigSizeMat);

    //割出字號上每個數字到subNumber
    vector<Mat> subNumber;
    for(int i = 0; i < 10; i++)
    {
        Mat subMat = imgIdNumber(Rect(i * 21, 0, 21, 70)).clone();
        Mat bigSizeSubMat(960, 1280, CV_8UC3, Scalar::all(0));//用成大圖片 較好用OCR
        subMat.copyTo(bigSizeSubMat(Rect(50, 50, subMat.cols, subMat.rows)));
        subNumber.push_back(bigSizeSubMat);//切割成一個一個的數字
    }

    //顯示各個數字
    for(int i = 0; i < 10; i++)
    {
        char str[] = "subNum";
        char charNum[1];
        sprintf(charNum, "%d", i);
        strcat(str, charNum);
        imshow(str, subNumber[i]);
        char path[] = "../subNum/";
        char type[] = ".png";
        strcat(path, str);
        strcat(path, type);
        cout << "Path = " << path << endl;
        imwrite(path, subNumber[i]);
    }

    //OCR處理
    //Mat imgTest =imread( "scenetext02.jpg", IMREAD_COLOR );
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    api -> Init("/home/tyson/tessdata/", "eng", tesseract::OEM_DEFAULT);
	api -> SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	api -> SetImage((uchar*)imgIdNumber.data, imgIdNumber.size().width, imgIdNumber.size().height,
		imgIdNumber.channels(), imgIdNumber.step1());
	api -> Recognize(0);
	const char* eng = api -> GetUTF8Text();
	string str (eng);
	cout << "String:" <<  str << endl;

	api -> End();
    //Mat img =imread( argv[1], IMREAD_COLOR );
    //camshift(output);
    //camshift2(output);
    waitKey(0);

    return EXIT_SUCCESS;
}
