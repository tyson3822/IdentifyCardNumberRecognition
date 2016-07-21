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
#include "ColorBalance.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

char scenePath[100] = "../scene/";
TestFile _TF;

int main(int argc, const char ** argv)
{
    //匯入檔案
    Mat imgObject = imread( argv[1], IMREAD_COLOR );//object.png
    _TF.InitTestFile(argv[2], argv[3], argv[4]);//input,output,result


for(int index = 0; index < 2; index++)
{
strcpy(scenePath, "../scene/");
    //讀取scene
    //imgBuffer = new char[_TF.GetImgByIndex(index).length() + 1];
    char imgBuffer[50];
    strcpy(imgBuffer, _TF.GetImgByIndex(index).c_str());
    strcat(scenePath, imgBuffer);    //從檔案輸入scene圖片及加上路徑
    cout << "scenePath = " << scenePath << endl;
    Mat imgScene = imread(scenePath , IMREAD_COLOR );  //讀取場景圖片

    //先色彩平衡再灰階  失敗
    //Mat imgSceneCB;
    //Mat imgObjectCB;
    //ColorBalance(imgScene,imgSceneCB,1);
    //ColorBalance(imgObject,imgObjectCB,1);

    //Mat imgSceneGary;
    //Mat imgObjectGary;
    //cvtColor(imgSceneCB, imgSceneGary, CV_BGR2GRAY);
    //cvtColor(imgObjectCB, imgObjectGary, CV_BGR2GRAY);

    //File測試
//    for(int i = 0; i < _TF.GetImgVectorSize(); i++)
//    {
//        _TF.WriteToOutput(_TF.GetImgByIndex(i));
//    }
//    _TF.WriteDownOutput();
//
//    _TF.Close();
    //File測試結束

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

    //顯示各個數字＆儲存
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

    //灰階
    cvtColor(imgIdNumber, imgIdNumber, CV_BGR2GRAY);
    imwrite("../subNum/imgIdNumber_gary.png",imgIdNumber);

    //二值化
    // Set threshold and maxValue
    double thresh = 127;
    double maxValue = 255;

    Mat whiteWord;
    Mat whiteLight;
    char whiteWordPath[50];
    char whiteLightPath[50];
    int threshTemp;
    for(int i = 0; i < 8; i++)
    {
        threshTemp = i * 32;
        char s1[10];
        sprintf(s1, "%d", threshTemp);

        threshold(imgIdNumber,whiteWord,threshTemp, maxValue, THRESH_BINARY_INV);//字變白底變黑
        strcpy(whiteWordPath, "../subNum/");
        strcat(whiteWordPath, "1_whiteWord");
        strcat(whiteWordPath, s1);
        strcat(whiteWordPath, ".png");
        imwrite(whiteWordPath,whiteWord);

        threshold(imgIdNumber,whiteLight, threshTemp, maxValue, THRESH_BINARY);//反光變白
        strcpy(whiteLightPath, "../subNum/");
        strcat(whiteLightPath, "2_whiteLight");
        strcat(whiteLightPath, s1);
        strcat(whiteLightPath, ".png");
        imwrite(whiteLightPath,whiteLight);
    }


    // Binary Threshold
    threshold(imgIdNumber,imgIdNumber, thresh, maxValue, THRESH_BINARY_INV);//字變白底變黑
    imwrite("../subNum/imgIdNumber_binary.png",imgIdNumber);

    //閉合(先膨脹再侵蝕)
    Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
    Mat closeImg;
    morphologyEx( imgIdNumber, imgIdNumber, MORPH_CLOSE, element);

    //中值濾波
    medianBlur(imgIdNumber, imgIdNumber, 3);
    imwrite("../subNum/imgIdNumber_medianBlur.png",imgIdNumber);

    //OCR處理
    //Mat imgTest =imread( "scenetext02.jpg", IMREAD_COLOR );
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    api -> Init("/home/tyson/tessdata/", "eng", tesseract::OEM_DEFAULT);
	api -> SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	api -> SetImage((uchar*)imgIdNumber.data, imgIdNumber.size().width, imgIdNumber.size().height,
		imgIdNumber.channels(), imgIdNumber.step1());
	api -> Recognize(0);
	const char* eng = api -> GetUTF8Text();
	char outputString[15] = "";
	strncpy(outputString, eng, 10);
	//string str (eng);
	cout << "String:" <<  outputString << endl;


	api -> End();
    //Mat img =imread( argv[1], IMREAD_COLOR );
    //camshift(output);
    //camshift2(output);

    _TF.WriteToOutput(outputString);
}

    _TF.WriteDownOutput();

    waitKey(0);

    return EXIT_SUCCESS;
}

//./main ../template/object2.png ../test/inputTest.txt ../test/outputTest.txt ../test/testResult.txt
