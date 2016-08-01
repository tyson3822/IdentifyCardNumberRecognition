/*
*   Copyright 2016 Tai-Yuan Chen
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*/

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
#include "BlurDectection.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

char scenePath[100] = "../scene/";
//char diagonal[1] = "/";
TestFile _TF;

int main(int argc, const char ** argv)
{
    //匯入檔案
    Mat imgObject = imread( argv[1], IMREAD_COLOR );//object.png
    _TF.InitTestFile(argv[2], argv[3], argv[4]);//input,output,result

    for(int index = 0; index < 15; index++)
    {
        strcpy(scenePath, "../scene/");
        //讀取scene
        //imgBuffer = new char[_TF.GetImgByIndex(index).length() + 1];
        char imgBuffer[50];
        strcpy(imgBuffer, _TF.GetImgByIndex(index).c_str());
        strcat(scenePath, imgBuffer);    //從檔案輸入scene圖片及加上路徑

        cout << "scenePath = " << scenePath << endl;
        Mat imgScene = imread(scenePath , IMREAD_COLOR );  //讀取場景圖片

        //模糊偵測
        //BlurDectect(imgScene);

        //將圖片檔名稱去除副檔名
        char  imageBasePath[20] = "../imageOutput/";
        int folderIndex = index + 1;
        char folderIndexChar[5];
        sprintf(folderIndexChar, "%d", folderIndex);
        strcpy(folderIndexChar, _TF.FillDigit(folderIndexChar));
        cout << "folderIndexChar = " << folderIndexChar << endl;

//        waitKey(0);
//        return EXIT_SUCCESS;

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
        char imgIdNumberName[] = "/IdNum.png";
        char imgIdNumberPath[50];
        strcpy(imgIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgIdNumberName));
        cout << "imgIdNumberPath = " << imgIdNumberPath << endl;
        imwrite(imgIdNumberPath, imgIdNumber);

        //把身份證字號樣本放到較大的黑底圖上
        Mat bigSizeMat(960, 1280, CV_8UC3, Scalar::all(0));
        imgIdNumber.copyTo(bigSizeMat(Rect(100, 100, imgIdNumber.cols, imgIdNumber.rows)));
        char imgBigSizeName[] = "/bigSizeMat.png";
        char imgBigSizePath[50];
        strcpy(imgBigSizePath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgBigSizeName));
        imwrite(imgBigSizePath, bigSizeMat);

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
        char imgSubIdNumberPath[50];
        for(int i = 0; i < 10; i++)
        {
            char charNum[1];
            sprintf(charNum, "%d", i);
            imshow(charNum, subNumber[i]);

            char fileName[] = "/subNum";
            strcat(fileName, charNum);
            char type[] = ".png";
            strcat(fileName, type);

            strcpy(imgSubIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, fileName));

            cout << "imgSubIdNumberPath = " << imgSubIdNumberPath << endl;
            imwrite(imgSubIdNumberPath, subNumber[i]);
        }

        //灰階
        char imgGrayIdNumberPath[50];
        char imgGrayIdNumberName[] =  "/imgIdNumber_gray.png";
        cvtColor(imgIdNumber, imgIdNumber, CV_BGR2GRAY);
        strcpy(imgGrayIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgGrayIdNumberName));
        imwrite(imgGrayIdNumberPath, imgIdNumber);

        //做等化統計圖
        char imgEqualizeIdNumberPath[50];
        char imgEqualizeIdNumberName[] =  "/imgIdNumber_Equalize.png";
        equalizeHist( imgIdNumber, imgIdNumber );
        strcpy(imgEqualizeIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgEqualizeIdNumberName));
        imwrite(imgEqualizeIdNumberPath, imgIdNumber);

        //二值化
        // Set threshold and maxValue
        double thresh = 127;
        double maxValue = 255;

        Mat whiteWord;
        Mat whiteLight;
        char whiteWordPath[50];
        char whiteWordName[20];
        char whiteLightPath[50];
        char whiteLightName[20];
        int threshTemp;
        for(int i = 0; i < 8; i++)
        {
            threshTemp = i * 32;
            char s1[10];
            sprintf(s1, "%d", threshTemp);

            threshold(imgIdNumber,whiteWord,threshTemp, maxValue, THRESH_BINARY_INV);//字變白底變黑
            strcpy(whiteWordName, "/1_whiteWord");
            strcat(whiteWordName, s1);
            strcat(whiteWordName, ".png");
            strcpy(whiteWordPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, whiteWordName));
            imwrite(whiteWordPath, whiteWord);

            threshold(imgIdNumber,whiteLight, threshTemp, maxValue, THRESH_BINARY);//反光變白
            strcpy(whiteLightName, "/2_whiteLight");
            strcat(whiteLightName, s1);
            strcat(whiteLightName, ".png");
            strcpy(whiteLightPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, whiteLightName));
            imwrite(whiteLightPath,whiteLight);
        }

        // Binary Threshold
        char imgBiraryIdNumberPath[50];
        char imgBinaryName[] = "/imgIdNumber_binary.png";
        threshold(imgIdNumber,imgIdNumber, 32, maxValue, THRESH_BINARY_INV);//字變白底變黑
        strcpy(imgBiraryIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgBigSizeName));
        imwrite(imgBiraryIdNumberPath, imgIdNumber);

        //閉合(先膨脹再侵蝕)
        Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
        Mat closeImg;
        morphologyEx( imgIdNumber, imgIdNumber, MORPH_CLOSE, element);

        //中值濾波
        char imgMedianIdNumberPath[50];
        char imgMedianName[] = "/imgIdNumber_medianBlur.png";
        medianBlur(imgIdNumber, imgIdNumber, 3);
        strcpy(imgMedianIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgMedianName));
        imwrite(imgMedianIdNumberPath, imgIdNumber);

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
        cout << "String:" <<  outputString << endl;

        api -> End();
        //Mat img =imread( argv[1], IMREAD_COLOR );
        //camshift(output);
        //camshift2(output);

        _TF.WriteToOutput(outputString);
    }

    _TF.WriteDownOutput();
    _TF.MatchResult();

    waitKey(0);
    return EXIT_SUCCESS;
}

//./main ../template/object2.png ../test/inputTest.txt ../test/outputTest.txt ../test/testResult.txt
