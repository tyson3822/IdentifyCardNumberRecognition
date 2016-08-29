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
#include <dirent.h>

#include "TestFile.hpp"
#include "BlurDetection.hpp"
#include "GetCardMat.hpp"
#include "CreatIDNumRandom.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

#define PRINT_COUNT 0
#define PRINT_RESULT 1

char scenePath[100];
TestFile _TF;

int main(int argc, const char ** argv)
{
    cout << endl << "--if you want to see the output image--" << endl;
    cout << "--you can create folder in the ../imageOutput/(folderIndex)--" << endl;
    cout << "--eg. sample's output image folder path is ../imageOutput/0000/--" << endl;
    cout << "--the folder index is according to it's index in the inputTest.txt--" << endl;

    cout << endl << "--start identity card recognition program--" << endl << endl;

    //匯入測試樣本檔案
    _TF.InitTestFile(argv[1], argv[2], argv[3]);//input,output,result
    int indexMin = atoi(argv[4]);
    int indexMax = atoi(argv[5]);

    //進行每一張圖的影像處理
    for(int index = indexMin; index <= indexMax; index++)
    {
        //讀取scene的文字路徑處理
        strcpy(scenePath, "../scene/");
        char imgBuffer[50];
        strcpy(imgBuffer, _TF.GetImgByIndex(index).c_str());
        strcat(scenePath, imgBuffer);    //從檔案輸入scene圖片及加上路徑
        Mat imgScene = imread(scenePath , IMREAD_COLOR );  //讀取場景圖片
        cout << "scenePath = " << scenePath << endl;

        //檢查影像大小有沒有符合需求
        if(imgScene.cols < 800 || imgScene.rows < 600)
        {
            cout << "--this image is not suitable for size, you should take another picture with width and height more than 800 and  600." << endl << endl;

            //把訊息記錄下來
            char widthTemp[10];
            char heightTemp[10];
            char sizeMessage[100];
            strcpy(sizeMessage, "ignore, size is too small. the image's width is ");

            sprintf(widthTemp, "%d", imgScene.cols);
            sprintf(heightTemp, "%d", imgScene.rows);

            strcat(sizeMessage, widthTemp);
            strcat(sizeMessage, ", and the image's height is ");
            strcat(sizeMessage, heightTemp);
            strcat(sizeMessage, " (in the range, width < 800 or height < 600)");

            //輸出忽略結果及訊息到output vector
            _TF.WriteToOutputByIndex(sizeMessage, index);
            continue;
        }

        //模糊偵測過度模糊的話就忽略
        float varianceOfLaplacian = BlurDectect(imgScene);
        //若模糊值在指定範圍內則是為模糊
        if(varianceOfLaplacian < 300)
        {
            cout << "--this image is blurry, ignore." << endl << endl;

            //把訊息記錄下來
            char blurryVOP[10];
            char blurryMessage[100];
            strcpy(blurryMessage, "ignore, blurry. the variance of Laplacian is ");
            sprintf(blurryVOP, "%f", varianceOfLaplacian);
            strcat(blurryMessage, blurryVOP);
            strcat(blurryMessage, " (in the range 0 ~ 300)");

            //輸出忽略結果及訊息到output vector
            _TF.WriteToOutputByIndex(blurryMessage, index);
            continue;
        }

        //將圖片檔名稱去除副檔名以方便設定影像結果至輸出資料夾
        char  imageBasePath[20] = "../imageOutput/";
        int folderIndex = index;
        char folderIndexChar[5];
        sprintf(folderIndexChar, "%d", folderIndex);
        strcpy(folderIndexChar, _TF.FillDigit(folderIndexChar));
        cout << "the output image folder index = " << folderIndexChar << endl;

        //切割出場景上的身份證
        Mat imgID(480, 800, CV_8UC3, Scalar::all(0));
        GetCardMat(imgScene, imgID);

        //如果場景上找不到身份證的話
        if(imgID.empty())
        {
            cout << "--can't detect identity card in this image, ignore." << endl << endl;
            //輸出忽略結果到output vector
            _TF.WriteToOutputByIndex("ignore, can't find the identity card.", index);
            continue;
        }

        //割出身份證上的字號樣本
        Mat imgIdNumber = imgID(Rect(565, 400, 225, 70)).clone();
        imshow("IdNumber", imgIdNumber);
        char imgIdNumberName[] = "/IdNum.png";
        char imgIdNumberPath[50];
        strcpy(imgIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgIdNumberName));
        //cout << "imgIdNumberPath = " << imgIdNumberPath << endl;
        imwrite(imgIdNumberPath, imgIdNumber);

        //反光值做計算
        int reflectionValue = CalculateReflectionValue(imgIdNumber);
        //若反光值超出範圍則忽略
        if(reflectionValue > 5 || reflectionValue < 0)
        {
            cout << "--can't detect identity card number, maybe the image reflective, ignore." << endl << endl;

            //把訊息記錄下來
            char reflectionCharArr[10];
            char reflectionMessage[100];
            strcpy(reflectionMessage, "ignore, the image have reflection, and value is ");
            sprintf(reflectionCharArr, "%d", reflectionValue);
            strcat(reflectionMessage, reflectionCharArr);
            strcat(reflectionMessage, " (out of range 0 ~ 5).");

            //輸出忽略結果及訊息到output vector
            _TF.WriteToOutputByIndex(reflectionMessage, index);
            continue;
        }

        //宣告存字母和數字的Mat
        Mat singleAlphabet(imgIdNumber.size(), CV_8UC1, Scalar::all(255));
        Mat multiNumbers(imgIdNumber.size(), CV_8UC1, Scalar::all(255));;

        //把原圖分割成字母和數字
        SeparateIdentityNumberMethod2(imgIdNumber, singleAlphabet, multiNumbers);

        //灰階 二值濾波
        cvtColor(imgIdNumber, imgIdNumber, CV_BGR2GRAY);
        BinaryFilterByThresh(imgIdNumber, imgIdNumber);

        //將原圖切割下來
        imgIdNumber(Rect(0, 0, singleAlphabet.cols, singleAlphabet.rows)).copyTo(singleAlphabet);
        imgIdNumber(Rect(singleAlphabet.cols, 0, multiNumbers.cols, multiNumbers.rows)).copyTo(multiNumbers);

        //顯示切割結果
        imshow("singleAlphabet mat", singleAlphabet);
        imshow("multiNumbers mat", multiNumbers);

        //OCR處理
        //初始化
        tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
        api -> Init("../", "kaiu_eng", tesseract::OEM_DEFAULT );

        //先掃描數字
        //白名單和模式設定
        api -> TessBaseAPI::SetVariable("tessedit_char_whitelist", "0123456789");
        api -> SetPageSegMode(tesseract::PSM_SINGLE_LINE );

        //給定影像，辨識
        api -> SetImage((uchar*)multiNumbers.data, multiNumbers.size().width, multiNumbers.size().height,
            multiNumbers.channels(), multiNumbers.step1());
        api -> Recognize(0);
        const char* num = api -> GetUTF8Text();

        //白名單和模式設定
        api -> TessBaseAPI::SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        api -> SetPageSegMode(tesseract::PSM_SINGLE_CHAR);

        //給定影像，辨識
        api -> SetImage((uchar*)singleAlphabet.data, singleAlphabet.size().width, singleAlphabet.size().height,
            singleAlphabet.channels(), singleAlphabet.step1());
        api -> Recognize(0);
        const char* alphabet = api -> GetUTF8Text();

        //輸出設定
        char outputString[15] = "";
        strncat(outputString, alphabet, 1);
        strncat(outputString, num, 9);
        api -> End();

        cout << "String:" <<  outputString << endl;

        //輸出結果到output vector
        _TF.WriteToOutputByIndex(outputString, index);

        cout << endl;
    }

    //把資料寫進文件中
    _TF.WriteDownOutput();

     cout << endl << "--begin get test result--" << endl << endl;

    //比對結果
    _TF.MatchResult();

    //顯示成功失敗忽略的樣本
    _TF.ListSuccessTest(PRINT_RESULT);
    _TF.ListFailureTest(PRINT_RESULT);
    _TF.ListIgnoreTest(PRINT_RESULT);

    cout << endl;

    //顯示最終數據
    _TF.PrintResultData();

    cout << endl << "--program end--" << endl << endl;

    waitKey(0);
    return EXIT_SUCCESS;
}

//command line input
//./main ../test/inputTest.txt ../test/outputTest.txt ../test/testResult.txt 0 0

//////////////////////////////////////////////////////
//以下是沒使用但日後可以參考或擴增的功能//
//////////////////////////////////////////////////////

//Mat imgObject = imread( argv[1], IMREAD_COLOR );//object.png

//OCR原始
//        api -> SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
//
//        api -> SetImage((uchar*)imgIdNumber.data, imgIdNumber.size().width, imgIdNumber.size().height,
//            imgIdNumber.channels(), imgIdNumber.step1());
//        api -> Recognize(0);
//        const char* eng = api -> GetUTF8Text();

//資料夾控制
//    DIR *dir;
//    struct dirent *ent;
//    if ((dir = opendir ("../scene/")) != NULL)
//    {
//        /* print all the files and directories within directory */
//        while ((ent = readdir (dir)) != NULL)
//        {
//            printf ("%s\n", ent->d_name);
//        }
//        closedir (dir);
//    }
//    else
//    {
//        /* could not open directory */
//        perror ("");
//        return EXIT_FAILURE;
//    }

//閉合(先膨脹再侵蝕)
//        Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
//        erode(imgIdNumber, imgIdNumber, element, Point(-1,-1), 2);//侵蝕
//        dilate(imgIdNumber, imgIdNumber, element, Point(-1,-1), 1);//膨脹
//        //morphologyEx( imgIdNumber, imgIdNumber, MORPH_CLOSE, element);
//        imshow("closeImg", imgIdNumber);

//中值濾波
//        char imgMedianIdNumberPath[50];
//        char imgMedianName[] = "/03imgIdNumber_medianBlur.png";
//        medianBlur(imgIdNumber, imgIdNumber, 3);
//        strcpy(imgMedianIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgMedianName));
//        imwrite(imgMedianIdNumberPath, imgIdNumber);

//二值化
//        double thresh = 127;
//        double maxValue = 255;
//        char imgBiraryIdNumberPath[50];
//        char imgBinaryName[] = "/02imgIdNumber_binary.png";
//        threshold(imgIdNumber,imgIdNumber, 120, 255, THRESH_BINARY_INV);//字變白底變黑
//        strcpy(imgBiraryIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgBinaryName));
//        imwrite(imgBiraryIdNumberPath, imgIdNumber);

///非必要  除錯用
//        Mat whiteWord;
//        Mat whiteLight;
//        char whiteWordPath[50];
//        char whiteWordName[20];
//        char whiteLightPath[50];
//        char whiteLightName[20];
//        int threshTemp;
//        for(int i = 0; i < 8; i++)
//        {
//            threshTemp = i * 32;
//            char s1[10];
//            sprintf(s1, "%d", threshTemp);
//
//            threshold(imgIdNumber,whiteWord,threshTemp, maxValue, THRESH_BINARY_INV);//字變白底變黑
//            strcpy(whiteWordName, "/1_whiteWord");
//            strcat(whiteWordName, s1);
//            strcat(whiteWordName, ".png");
//            strcpy(whiteWordPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, whiteWordName));
//            imwrite(whiteWordPath, whiteWord);
//
//            threshold(imgIdNumber,whiteLight, threshTemp, maxValue, THRESH_BINARY);//反光變白
//            strcpy(whiteLightName, "/2_whiteLight");
//            strcat(whiteLightName, s1);
//            strcat(whiteLightName, ".png");
//            strcpy(whiteLightPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, whiteLightName));
//            imwrite(whiteLightPath,whiteLight);
//        }

//灰階
//        char imgGrayIdNumberPath[50];
//        char imgGrayIdNumberName[] =  "/00imgIdNumber_gray.png";
//        cvtColor(imgIdNumber, imgIdNumber, CV_BGR2GRAY);
//        strcpy(imgGrayIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgGrayIdNumberName));
//        imwrite(imgGrayIdNumberPath, imgIdNumber);

//做等化統計圖
//        char imgEqualizeIdNumberPath[50];
//        char imgEqualizeIdNumberName[] =  "/01imgIdNumber_Equalize.png";
//        equalizeHist( imgIdNumber, imgIdNumber );
//        strcpy(imgEqualizeIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgEqualizeIdNumberName));
//        imwrite(imgEqualizeIdNumberPath, imgIdNumber);

///非必要 除錯用
//把身份證字號樣本放到較大的黑底圖上
//        Mat bigSizeMat(960, 1280, CV_8UC3, Scalar::all(0));
//        imgIdNumber.copyTo(bigSizeMat(Rect(100, 100, imgIdNumber.cols, imgIdNumber.rows)));
//        char imgBigSizeName[] = "/bigSizeMat.png";
//        char imgBigSizePath[50];
//        strcpy(imgBigSizePath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgBigSizeName));
//        imwrite(imgBigSizePath, bigSizeMat);
//
//        //割出字號上每個數字到subNumber
//        vector<Mat> subNumber;
//        for(int i = 0; i < 10; i++)
//        {
//            Mat subMat = imgIdNumber(Rect(i * 21, 0, 21, 70)).clone();
//            Mat bigSizeSubMat(960, 1280, CV_8UC3, Scalar::all(0));//用成大圖片 較好用OCR
//            subMat.copyTo(bigSizeSubMat(Rect(50, 50, subMat.cols, subMat.rows)));
//            subNumber.push_back(bigSizeSubMat);//切割成一個一個的數字
//        }
//
//        //顯示各個數字＆儲存
//        char imgSubIdNumberPath[50];
//        for(int i = 0; i < 10; i++)
//        {
//            char charNum[1];
//            sprintf(charNum, "%d", i);
//            //imshow(charNum, subNumber[i]);
//
//            char fileName[] = "/subNum";
//            strcat(fileName, charNum);
//            char type[] = ".png";
//            strcat(fileName, type);
//
//            strcpy(imgSubIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, fileName));
//
//            cout << "imgSubIdNumberPath = " << imgSubIdNumberPath << endl;
//            imwrite(imgSubIdNumberPath, subNumber[i]);
//        }

//meanshift
//        pyrMeanShiftFiltering( imgIdNumber, imgIdNumber, 10, 10, 3);
//        char imgGrayIdNumberName[] =  "/imgIdNumber_gray.png";
//        cvtColor(imgIdNumber, imgIdNumber, CV_BGR2GRAY);
//        strcpy(imgGrayIdNumberPath, _TF.ImageOutputPath(imageBasePath, folderIndexChar, imgGrayIdNumberName));
//        imwrite(imgGrayIdNumberPath, imgIdNumber);
