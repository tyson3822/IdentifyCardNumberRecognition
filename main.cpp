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
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "TestFile.hpp"
#include "BlurDetection.hpp"
#include "GetCardMat.hpp"
#include "CreatIDNumRandom.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

#define PRINT_COUNT 0
#define PRINT_RESULT 1

char testPath[100];
char scenePath[100];
char imageBasePath[100];
char  imageBaseFolderPath[100];
char commandLineTemp[100];
TestFile _TF;

void SaveTestFile()
{
    char testFileName[3][20] = {"inputTest.txt", "outputTest.txt",  "testResult.txt"};

    for(int index = 0; index < 3; index++)
    {
        char srcTestFilePath[100];
        char destTestFilePath[100];

        strcpy(srcTestFilePath, testPath);
        strcat(srcTestFilePath, testFileName[index]);

        strcpy(destTestFilePath, imageBasePath);
        strcat(destTestFilePath, testFileName[index]);

        _TF.CopyFile(srcTestFilePath, destTestFilePath);
    }
}

int main(int argc, const char ** argv)
{
    cout << endl << "--if you want to see the output image--" << endl;
    cout << "--you can create folder in the ../imageOutput/(folderIndex)--" << endl;
    cout << "--eg. sample's output image folder path is ../imageOutput/0000/--" << endl;
    cout << "--the folder index is according to it's index in the inputTest.txt--" << endl;

    cout << endl << "--start identity card recognition program--" << endl << endl;

    ///command line input
    strcpy(testPath, argv[1]);//test資料夾路徑
    strcpy(scenePath, argv[2]); //樣本路徑
    strcpy(imageBasePath, argv[3]);//輸出處理圖片資料夾路徑
    int indexMin = atoi(argv[4]);//起始index
    int indexMax = atoi(argv[5]);//結束index
    int varianceOfLaplacianMax = atoi(argv[6]);//設定模糊範圍

    ///command line access
    for(int index = 1; index < 7; index++)
    {
        strcat(commandLineTemp, argv[index]);
        strcat(commandLineTemp, " ");
    }

    ///初始化測試資料
    _TF.InitTestFile(argv[1]);

    ///首先新建由時間命名的處理圖片路經資料夾
    //取得時間
    time_t t = time(0);
    char timeTemp[64];
    strftime( timeTemp, sizeof(timeTemp), "%Y-%m-%d-%X",localtime(&t) );

    //指定路徑
    strcat(imageBasePath, timeTemp);

    //資料夾建立成功與否
    mkdir(imageBasePath, 0);

    //指定到子路徑方便後面程式使用
    strcat(imageBasePath, "/");

    ///進行每一張圖的影像處理
    for(int index = indexMin; index <= indexMax; index++)
    {
        ///讀取影像
        //取得影像資料夾路徑
        char scenePathTemp[100];
        strcpy(scenePathTemp, scenePath);

        //取得影像名稱路徑
        char imgBuffer[50];
        strcpy(imgBuffer, _TF.GetImgByIndex(index).c_str());//暫存影像名稱
        strcat(scenePathTemp, imgBuffer);//連接成影像路徑

         //存取影像
        Mat imgScene = imread(scenePathTemp , IMREAD_COLOR );
        cout << "scenePath = " << scenePathTemp << endl;

        ///建立處理圖片資料夾之子資料夾
        //將處理圖片index 由int轉成char[]並遵循規則,eg.0->0000,1->0001,20->0020
        int folderIndex = index;
        char folderIndexChar[5];
        sprintf(folderIndexChar, "%d", folderIndex);
        strcpy(folderIndexChar, _TF.FillDigit(folderIndexChar));
        cout << "the output image folder index = " << folderIndexChar << endl;

        //連接子路徑名稱
        strcpy(imageBaseFolderPath, imageBasePath);
        strcat(imageBaseFolderPath, folderIndexChar);

        //新建子路徑資料夾,用來存本index影像處理的輸出圖片
        mkdir(imageBaseFolderPath, 0);

        //指定到子路徑以方便使用
        strcat(imageBaseFolderPath, "/");

        ///先確認影像是直向還是橫向，如果是直向一律先擺成橫向
        float widthHeightRate = (float)imgScene.cols / (float)imgScene.rows;

        ///如果是直向的，就先翻轉成橫向
        if(widthHeightRate < 1)
        {
            Mat outputImgScene(Size(imgScene.rows, imgScene.cols), CV_8UC3, Scalar::all(255));

            float rotateAngle = 90;
            float originScale = 1;
            Point imgSceneCenter(imgScene.cols / 2, outputImgScene.rows / 2);

            Mat rotatedImgSceneTranserMat = getRotationMatrix2D(imgSceneCenter, rotateAngle, originScale);
            warpAffine( imgScene, outputImgScene, rotatedImgSceneTranserMat, outputImgScene.size() );
            outputImgScene.copyTo(imgScene);
        }

        ///檢查影像大小有沒有符合需求
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

        ///若模糊值在指定範圍內則是為模糊
        if(varianceOfLaplacian < varianceOfLaplacianMax)//origin = 300
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

        //切割出場景上的身份證
        Mat imgID(480, 800, CV_8UC3, Scalar::all(0));
        GetCardMat(imgScene, imgID);

        //儲存處理圖片
        _TF.SaveOutputImage("1_IdCardWithoutProcess.png", imageBaseFolderPath, imgID);

        ///如果場景上找不到身份證的話
        if(imgID.empty())
        {
            cout << "--can't detect identity card in this image, ignore." << endl << endl;
            //輸出忽略結果到output vector
            _TF.WriteToOutputByIndex("ignore, can't find the identity card.", index);
            continue;
        }

        ///從身份證上面找到國旗
        vector<Point2f> nationalFlagCorner;
        nationalFlagCorner = GetNationalFlagCorner(imgID);

        ///如果找不到國旗，那代表可能影像本身就過度歪斜列入忽略
        if(nationalFlagCorner.empty())
        {
            cout << "--can't find the national flag in the image, maybe it's too crooked to find the national flag area, ignore." << endl << endl;
            //輸出忽略結果到output vector
            _TF.WriteToOutputByIndex("ignore, can't find the national flag.", index);
            continue;
        }

        ///進行校正，利用身份證上的國旗位置來校正身份證需不需要旋轉
        bool isRotates = false;
        isRotates = RotateCardUseNationalFlag(imgID, imgID, nationalFlagCorner);

        ///如果有校正過的影像，需要在重新找一次國旗
        if(isRotates)
        {
            nationalFlagCorner = GetNationalFlagCorner(imgID);
        }

        ///雙重確認，確認身份證上的國旗是不是在正確的區域
        bool isCorrectAreaNationalFlag;
        isCorrectAreaNationalFlag = DoubleCheckUseNationalFlag(imgID, nationalFlagCorner);

        ///雙重確認國旗的位置是不是在合適的位置
        if(isCorrectAreaNationalFlag != 1)
        {
            cout << "--can't detect national flag in the correct area, maybe it's too crooked to effect the national flag area, ignore." << endl << endl;

            //輸出忽略結果到output vector
            _TF.WriteToOutputByIndex("ignore, can't detect national flag in the correct area.", index);
            continue;
        }

        //儲存處理圖片
        _TF.SaveOutputImage("2_IdCardCorrect.png", imageBaseFolderPath, imgID);

        ///割出身份證上的字號樣本
        Mat imgIdNumber = imgID(Rect(565, 400, 225, 70)).clone();
        imshow("IdNumber", imgIdNumber);

        //儲存處理圖片
        _TF.SaveOutputImage("3_IdSegmentNumber.png", imageBaseFolderPath, imgIdNumber);

        ///反光值做計算
        int reflectionValue = CalculateReflectionValue(imgIdNumber);

        ///若反光值超出範圍則忽略
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

        ///宣告存字母和數字的Mat
        Mat singleAlphabet(imgIdNumber.size(), CV_8UC1, Scalar::all(255));
        Mat multiNumbers(imgIdNumber.size(), CV_8UC1, Scalar::all(255));;

        ///把原圖分割成字母和數字
        SeparateIdentityNumber(imgIdNumber, singleAlphabet, multiNumbers);
        //SeparateIdentityNumberMethod2(imgIdNumber, singleAlphabet, multiNumbers);

        ///灰階 二值濾波
        cvtColor(imgIdNumber, imgIdNumber, CV_BGR2GRAY);
        BinaryFilterByThresh(imgIdNumber, imgIdNumber);

        ///將原圖切割下來
        imgIdNumber(Rect(0, 0, singleAlphabet.cols, singleAlphabet.rows)).copyTo(singleAlphabet);
        imgIdNumber(Rect(singleAlphabet.cols, 0, multiNumbers.cols, multiNumbers.rows)).copyTo(multiNumbers);

        ///顯示切割結果
        imshow("singleAlphabet mat", singleAlphabet);
        imshow("multiNumbers mat", multiNumbers);

        //儲存處理圖片
        _TF.SaveOutputImage("4_IdNumberSingleAlphabet.png", imageBaseFolderPath, singleAlphabet);
        _TF.SaveOutputImage("5_IdNumberMultiNumbers.png", imageBaseFolderPath, multiNumbers);

        ///OCR處理
        //初始化
        tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
        api -> Init("../", "kaiu_eng", tesseract::OEM_DEFAULT );

        ///先掃描數字
        //白名單和模式設定
        api -> TessBaseAPI::SetVariable("tessedit_char_whitelist", "0123456789");
        api -> SetPageSegMode(tesseract::PSM_SINGLE_LINE );

        //給定影像，辨識
        api -> SetImage((uchar*)multiNumbers.data, multiNumbers.size().width, multiNumbers.size().height,
            multiNumbers.channels(), multiNumbers.step1());
        api -> Recognize(0);
        const char* num = api -> GetUTF8Text();

        ///再掃描字母
        //白名單和模式設定
        api -> TessBaseAPI::SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        api -> SetPageSegMode(tesseract::PSM_SINGLE_CHAR);

        //給定影像，辨識
        api -> SetImage((uchar*)singleAlphabet.data, singleAlphabet.size().width, singleAlphabet.size().height,
            singleAlphabet.channels(), singleAlphabet.step1());
        api -> Recognize(0);
        const char* alphabet = api -> GetUTF8Text();

        ///輸出字串設定
        char outputString[15] = "";
        strncat(outputString, alphabet, 1);
        strncat(outputString, num, 9);
        api -> End();

        cout << "String:" <<  outputString << endl;

        ///輸出結果到output vector
        _TF.WriteToOutputByIndex(outputString, index);

        cout << endl;
    }

    //把output資料寫進文件中
    _TF.WriteDownOutput();

     cout << endl << "--begin get test result--" << endl << endl;

    ///比對結果
    _TF.MatchResult();

    //把command line記錄下來
    _TF.SaveCommandLine(commandLineTemp);

    //顯示成功失敗忽略的樣本
    _TF.ListSuccessTest(PRINT_RESULT);
    _TF.ListFailureTest(PRINT_RESULT);
    _TF.ListIgnoreTest(PRINT_RESULT);

    cout << endl;

    ///顯示最終數據
    _TF.PrintResultData();
    _TF.WriteResultData();

    cout << endl << "--program end--" << endl << endl;

    ///儲存文件檔
    SaveTestFile();

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
