#include <opencv2/opencv.hpp>
#include "ColorBalance.hpp"

using namespace cv;
using namespace std;

//struct userdata{
//    Mat im;
//    vector<Point2f> points;
//};

Mat imgScene;
Mat imgSceneGray;
//int thresh = 50;
//int max_thresh = 255;
//RNG rng(12345);

//void mouseHandler(int event, int x, int y, int flags, void* data_ptr)
//{
//    if  ( event == EVENT_LBUTTONDOWN )
//    {
//        userdata *data = ((userdata *) data_ptr);
//        circle(data->im, Point(x,y),3,Scalar(0,255,255), 5, CV_AA);
//        imshow("Image", data->im);
//        if (data->points.size() < 4)
//        {
//            data->points.push_back(Point2f(x,y));
//        }
//    }
//}

//vector順序變為0左上 1右上 2右下 3左下
vector<Point2f> SortRectPoint(vector<Point2f> input)
{
    vector<Point2f> inputTemp = input;
    vector<Point2f> output;
    Point2f temp;

    int maxIndex = 0;
    int minIndex = 0;
    double maxVal = 0;
    double minVal = 10000;

    for(int i = 0; i < inputTemp.size(); i++)
        for(int j = i + 1; j < inputTemp.size(); j++)
            if(sqrt(inputTemp[i].x * inputTemp[i].x + inputTemp[i].y * inputTemp[i].y) > sqrt(inputTemp[j].x * inputTemp[j].x + inputTemp[j].y * inputTemp[j].y))
            {
                temp = inputTemp[i];
                inputTemp[i] = inputTemp[j];
                inputTemp[j] = temp;
            }


    for(int i = 0; i < inputTemp.size(); i++)
    {
        if(minVal > inputTemp[i].x + inputTemp[i].y)
        {
            minVal = inputTemp[i].x + inputTemp[i].y;
            minIndex = i;
        }
    }

    output.push_back(inputTemp[minIndex]);

    for(int i = 0; i < inputTemp.size(); i++)
    {
        if(maxVal < inputTemp[i].x + inputTemp[i].y)
        {
            maxVal = inputTemp[i].x + inputTemp[i].y;
            maxIndex = i;
        }
    }

    for(int i = 0; i < inputTemp.size(); i++)
    {
        if(i == minIndex || i == maxIndex)continue;
        output.push_back(inputTemp[i]);
    }

    output.push_back(inputTemp[maxIndex]);

    temp = output[1];
    output[1] = output[2];
    output[2] = temp;

    temp = output[2];
    output[2] = output[3];
    output[3] = temp;

    //做好排序的放到vector
//    for(int i = 0; i < inputTemp.size(); i++)
//        output.push_back(inputTemp[i]);

    //把順序變為左上 右上 左下 右下
//    temp = output[3];
//    output[3] = output[1];
//    output[1] = temp;

    return output;
}

//vector<Point2f> FindCardCorner()
//{
//    vector<Point2f> sceneCorner;
//    Mat threshold_output;
//    vector<vector<Point> > contours;
//    vector<Vec4i> hierarchy;
//
//    //
//    Canny( imgSceneGray, threshold_output, 0, 40, 3 );
//
//    imshow("Canny", threshold_output);
//
//    Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
//    Mat closeImg;
//    morphologyEx( threshold_output, threshold_output, MORPH_CLOSE, element);
//
//    imshow("Close", threshold_output);
//
//    // Detect edges using Threshold
//    //threshold( imgSceneGray, threshold_output, thresh, 255, THRESH_BINARY );
//    // Find contours
//    findContours( threshold_output, contours, hierarchy, CV_RETR_CCOMP  , CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
//
//    // Approximate contours to polygons + get bounding rects and circles
//    vector<vector<Point> > contours_poly( contours.size() );
//    vector<Rect> boundRect( contours.size() );
//    vector<Point2f>center( contours.size() );
//    vector<float>radius( contours.size() );
//
//    for( int i = 0; i < contours.size(); i++ )
//    {
//        //cout << "i = " << i << ", and fabs(contourArea(contours[" << i << "]) = " << fabs(contourArea(contours[i])) << endl;
//        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
//        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
//        minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
//    }
//
//
//    // Draw polygonal contour + bonding rects + circles
//    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
//    for( int i = 0; i< contours.size(); i++ )
//    {
//        if(fabs(contourArea(contours_poly[i])) > 11200 || fabs(contourArea(contours_poly[i])) < 5000 || !isContourConvex(contours_poly[i]) )
//            continue;
//        cout << "i = " << i << ", and fabs(contourArea(contours_poly[" << i << "]) = " << fabs(contourArea(contours_poly[i])) << endl;
//        Scalar color = Scalar(255, 255, 255);
//        drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
//        cout << "contours_poly[i]  = " << contours_poly[i] << endl;
//
//        vector<Point2f> contoursTemp;
//        for(int index = 0; index < contours_poly[i].size(); index++)
//            contoursTemp.push_back(contours_poly[i][index]);
//        contoursTemp = SortVectorPoint(contoursTemp);
//        vector<Point2f> resizeCorner;
//        Point2f shiftDis;
//
//        for(int index = 0; index < 4; index++)
//        {
//            Point2f corner = contoursTemp[index];
//            corner.x *= 6.25;
//            corner.y *= 5.658;
//            resizeCorner.push_back(corner);
//        }
//
//        resizeCorner = SortVectorPoint(resizeCorner);
//
//        cout << "resizeCorner  = " << resizeCorner << endl;
//
//        shiftDis.x = resizeCorner[0].x -  contoursTemp[0].x + 0.21875 * sqrt((contoursTemp[0].x - contoursTemp[1].x) * (contoursTemp[0].x - contoursTemp[1].x) + (contoursTemp[0].y - contoursTemp[1].y) * (contoursTemp[0].y - contoursTemp[1].y));// + 0.035 * sqrt((resizeCorner[0].x - resizeCorner[1].x) * (resizeCorner[0].x - resizeCorner[1].x) + (resizeCorner[0].y - resizeCorner[1].y) * (resizeCorner[0].y - resizeCorner[1].y));
//        shiftDis.y = resizeCorner[0].y -  contoursTemp[0].y + 0.45882 * sqrt((contoursTemp[0].x - contoursTemp[3].x) * (contoursTemp[0].x - contoursTemp[3].x) + (contoursTemp[0].y - contoursTemp[3].y) * (contoursTemp[0].y - contoursTemp[3].y));// + 0.081 * sqrt((resizeCorner[0].x - resizeCorner[3].x) * (resizeCorner[0].x - resizeCorner[3].x) + (resizeCorner[0].y - resizeCorner[3].y) * (resizeCorner[0].y - resizeCorner[3].y));
//
//        for(int index = 0; index < 4; index++)
//        {
//            resizeCorner[index].x -= shiftDis.x;
//            resizeCorner[index].y -= shiftDis.y;
//        }
//
//        for(int index = 0; index < 4; index++)
//            line(drawing, resizeCorner[index % 4], resizeCorner[(index + 1) % 4], Scalar(0, 255, 0), 2, LINE_AA);
//
//        for(int index = 0; index < 4; index++)
//            sceneCorner.push_back(resizeCorner[index]);
//        //rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 1, 1, 0 );
//        //circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );
//    }
//
//    // Show in a window
//    resize(drawing, drawing, Size(800,600));
//    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
//    imshow( "Contours", drawing );
//
////    sceneCorner.push_back(Point2f(boundRect[0].x,boundRect[0].y));
////    sceneCorner.push_back(Point2f(boundRect[0].x + boundRect[0].width, boundRect[0].y));
////    sceneCorner.push_back(Point2f(boundRect[0].x + boundRect[0].width, boundRect[0].y + boundRect[0].height));
////    sceneCorner.push_back(Point2f(boundRect[0].x, boundRect[0].y + boundRect[0].height));
//
//    return sceneCorner;
//}

vector<Point2f> GetCardCorner()
{
    vector<Point2f> cardCorner;
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //canny取輪廓
    Canny( imgSceneGray, threshold_output, 100, 200, 3 );
    imshow("Canny", threshold_output);

    //將輪廓接近的線給閉合
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat closeImg;
    morphologyEx( threshold_output, threshold_output, MORPH_CLOSE, element);
    imshow("Close", threshold_output);

    //找輪廓存到contours
    findContours( threshold_output, contours, hierarchy, CV_RETR_TREE  , CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    //宣告大約輪廓（減少Point），最小矩形，有可能為身份證的矩形
    vector<vector<Point> > contours_poly( contours.size() );
    vector<RotatedRect> minRect( contours.size() );
    vector<vector<Point> > cardRectPossible( contours.size() );
    int cardRectPossibleIndex = 0;

    //將每個輪廓取出大約的輪廓並將大約的輪廓用最小矩形包起
    for( int i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 5, true );
        minRect[i] = minAreaRect( (Mat)contours_poly[i] );
    }

    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
    for( int i = 0; i < contours_poly.size(); i++ )
    {
        //替除掉大小不合理的大約輪廓
        if(fabs(contourArea(contours_poly[i])) > 400000 || fabs(contourArea(contours_poly[i])) < 80000 || contours_poly[i].size() > 20 )//條件內的不要
            continue;

        //cout << "i = " << i << ", and fabs(contourArea(contours_poly[" << i << "]) = " << fabs(contourArea(contours_poly[i])) << endl;

        //將這些大約輪廓畫在drawing上
        Scalar color = Scalar(255, 255, 255);
        drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );

        //當前大約輪廓的最小矩形
        Point2f rect_points[4];
        minRect[i].points( rect_points );

        //將最小舉行畫在drawing並列進可能為身份證矩形的vector中
        for( int j = 0; j < 4; j++ )
        {
            line( drawing, rect_points[j], rect_points[(j+1)%4], Scalar(0, 255, 0), 3, 8 );
            cardRectPossible[cardRectPossibleIndex].push_back(rect_points[j]);
        }

        cardRectPossibleIndex++;
    }

    //顯示所有的大約輪廓及他們的最小矩形
    imshow( "all possible Contours", drawing );

    //取得這些矩形中最小的那個index（為了濾掉可能是外框）
    int minArea = 1000000;
    int minCardRectIndex = 0;
    for(int i = 0; i < cardRectPossibleIndex; i++)
    {
        int cardArea = fabs(contourArea(cardRectPossible[i]));
        if(cardArea < minArea)
        {
            minArea = cardArea;
            minCardRectIndex = i;
        }
    }

    //將最終的矩形存到輸出矩形上
    for(int i = 0; i < 4; i++)
    {
        cardCorner.push_back(cardRectPossible[minCardRectIndex][i]);
        line( drawing, cardRectPossible[minCardRectIndex][i], cardRectPossible[minCardRectIndex][(i + 1) % 4], Scalar(255, 0, 0), 3, 8 );
    }

    //將矩形的點重新排序成0左上 1右上 2右下 3左下
    cardCorner = SortRectPoint(cardCorner);

    //顯示最終畫布
    resize(drawing, drawing, Size(800,600));
    namedWindow( "result Contours", CV_WINDOW_AUTOSIZE );
    imshow( "result Contours", drawing );

    return cardCorner;
}

void GetCardMat( Mat& input, Mat& output)
{
    //輸出Mat長480寬800
    Mat imgOutput(480, 800, CV_8UC3, Scalar::all(0));

    //input複製到imgScene，調整到寬度800，長度維持比例，為適合偵測
    imgScene = input.clone();
    double rate = (800 / (double)imgScene.cols);
    resize(imgScene, imgScene, Size(imgScene.cols * rate, imgScene.rows * rate));

    //設定暫存Mat為處理Mat
    Mat imgSceneTemp;
    imgSceneTemp = imgScene.clone();
    imshow("Image SceneTemp", imgSceneTemp);

    //做色彩平衡
    Mat imgSceneColorBalance;
    ColorBalance(imgSceneTemp, imgSceneColorBalance, 1);

    //做MeanShift把顏色區塊化
    Mat imgSceneMeanShift;
    pyrMeanShiftFiltering( imgSceneColorBalance, imgSceneMeanShift, 30, 20, 3);
    imshow("meanShiftImg", imgSceneMeanShift);

    //灰階
    cvtColor( imgSceneMeanShift, imgSceneGray, CV_BGR2GRAY );

    //取得場景中的身份證四角
    vector<Point2f> cardCornerInScene = GetCardCorner();

    //將偵測出的四角還原到原圖大小上
    rate = ((double)input.cols / (double)imgScene.cols);
    for(int i = 0; i < 4; i ++)
        cardCornerInScene[i] *= rate;

    //設定身份證結果圖大小及四角
    Size imgOutputSize = imgOutput.size();
    vector<Point2f> outputCardCorner;
    outputCardCorner.push_back(Point2f(0,0));
    outputCardCorner.push_back(Point2f(imgOutputSize.width - 1, 0));
    outputCardCorner.push_back(Point2f(imgOutputSize.width - 1, imgOutputSize.height -1));
    outputCardCorner.push_back(Point2f(0, imgOutputSize.height - 1 ));

    //找出轉換矩陣並將身份證切割出來到寬800長480的imgOutput上
    Mat H = findHomography(cardCornerInScene, outputCardCorner);
    warpPerspective(input, imgOutput, H, imgOutputSize);

    //顯示輸出身份證切割圖
    imshow("imgOutput", imgOutput);
    output = imgOutput.clone();
}

//原版
//Mat GetCardMat( Mat& img1, Mat& img2)
//{
//    imgObj = img1.clone();
//    imgScene = img2.clone();
//    Mat imgOutput;
//
//    Size size = imgObj.size();
//
//    // Create a vector of points.
//    vector<Point2f> objCorner;
//    objCorner.push_back(Point2f(0,0));
//    objCorner.push_back(Point2f(size.width - 1, 0));
//    objCorner.push_back(Point2f(size.width - 1, size.height -1));
//    objCorner.push_back(Point2f(0, size.height - 1 ));
//
//
//    // Set data for mouse handler
//    imgOutput = imgScene.clone();
////    userdata data;
////    data.im = imgOutput;
//
//    //show the image
//    imshow("Image", imgOutput);
//
//    //cout << "Click on four corners of a billboard and then press ENTER" << endl;
//    //set the callback function for any mouse event
//    Mat colorBalanceImg;
//    ColorBalance(imgScene, colorBalanceImg, 1);
//
//    Mat meanShiftImg;
//    pyrMeanShiftFiltering( colorBalanceImg, meanShiftImg, 30, 20, 3);
//    imshow("meanShiftImg", meanShiftImg);
//
//    cvtColor( meanShiftImg, imgSceneGray, CV_BGR2GRAY );
//    vector<Point2f> sceneCorner = GetCardCorner();
//
//    // Calculate Homography between source and destination points
//    Mat H = findHomography(objCorner, sceneCorner);
//
//    // Warp source image
//    warpPerspective(imgObj, imgOutput, H, imgOutput.size());
//
//    // Extract four points from mouse data
//    Point pts_dst[4];
//    for( int i = 0; i < 4; i++)
//        pts_dst[i] = sceneCorner[i];
//
//    // Black out polygonal area in destination image.
//    fillConvexPoly(imgScene, pts_dst, 4, Scalar(0), CV_AA);
//
//    // Add warped source image to destination image.
//    imgScene = imgScene + imgOutput;
//
//    // Display image.
//    resize(imgScene, imgScene, Size(800, 600));
//    imshow("Image", imgScene);
//    waitKey(0);
//
//    return imgOutput;
//}
