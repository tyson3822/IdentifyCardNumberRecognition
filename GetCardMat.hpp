#include <opencv2/opencv.hpp>
#include "ChannelProcess.hpp"

using namespace cv;
using namespace std;

Mat imgScene;
Mat imgSceneGray;

//vector順序變為0左上 1右上 2右下 3左下，適用於橫向的長方形四點
template <class T>
void SortRectPoint(T& inputVector, T& outputVector)
{
    //前置
    T inputTemp = inputVector;
    T output;
    Point2f temp;

    int maxIndex = 0;
    int minIndex = 0;
    double maxVal = 0;
    double minVal = 10000;

    //以距離為條件 由進到遠進行泡沫排序
    for(int i = 0; i < inputTemp.size(); i++)
    {
        for(int j = i + 1; j < inputTemp.size(); j++)
        {
            if(sqrt(inputTemp[i].x * inputTemp[i].x + inputTemp[i].y * inputTemp[i].y) > sqrt(inputTemp[j].x * inputTemp[j].x + inputTemp[j].y * inputTemp[j].y))
            {
                temp = inputTemp[i];
                inputTemp[i] = inputTemp[j];
                inputTemp[j] = temp;
            }
        }
    }

    //找最近的點 並存起來
    for(int i = 0; i < inputTemp.size(); i++)
    {
        if(minVal > inputTemp[i].x + inputTemp[i].y)
        {
            minVal = inputTemp[i].x + inputTemp[i].y;
            minIndex = i;
        }
    }
    output.push_back(inputTemp[minIndex]);

    //找最遠的點 並存起來
    for(int i = 0; i < inputTemp.size(); i++)
    {
        if(maxVal < inputTemp[i].x + inputTemp[i].y)
        {
            maxVal = inputTemp[i].x + inputTemp[i].y;
            maxIndex = i;
        }
    }
    output.push_back(inputTemp[maxIndex]);

    //將其他的點也存起來
    for(int i = 0; i < inputTemp.size(); i++)
    {
        if(i == minIndex || i == maxIndex)continue;
        output.push_back(inputTemp[i]);
    }

    //此時方形上編號順序為
    ////////////////////////////////////////
    // [0]                                                [3] //
    //                                                            //
    //                                                            //
    //                                                            //
    // [2]                                                [1] //
    ////////////////////////////////////////

    //故先將右上右下對調
    temp = output[1];
    output[1] = output[3];
    output[3] = temp;

    //左下右下對調
    temp = output[2];
    output[2] = output[3];
    output[3] = temp;

    outputVector = output;
}

//為GetCardMat副程式 回傳找到身份證的四個角落點
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

    if(cardRectPossibleIndex > 0)
    {
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
        SortRectPoint(cardCorner, cardCorner);
    }

    //顯示最終畫布
    resize(drawing, drawing, Size(800,600));
    namedWindow( "result Contours", CV_WINDOW_AUTOSIZE );
    imshow( "result Contours", drawing );

    return cardCorner;
}

//為GetCardCorner主程式 從輸入影上切割出身份證
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

    //如果場景中找到最小矩形
    if(!cardCornerInScene.empty())
    {
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
    }
    else
    {
        Mat emptyMat;
        imgOutput = emptyMat.clone();
    }

    output = imgOutput.clone();
}

//把輸入的身份證字號影像分割成英文和數字兩個Mat輸出，輸入影像為三通道
void SeparateIdentityNumber(Mat& input, Mat& outputAlphabet, Mat& outputNumber)
{
    //複製原影像進行處理
    Mat inputTemp = input.clone();

    //將影像進行灰階
    cvtColor( inputTemp, inputTemp, CV_BGR2GRAY );

    //將影像進行濾波
    BinaryFilterByThresh(inputTemp, inputTemp);

    //找輪廓前置
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //將影像進行膨脹
//    Mat element = getStructuringElement(MORPH_RECT, Size(1, 10));
//    //dilate(inputTemp, inputTemp, element, Point(-1,-1), 1);//侵蝕
//    erode(inputTemp, inputTemp, element, Point(-1,-1), 1);//膨脹
//    imshow("closeImg", inputTemp);

    //canny取輪廓
    imshow("before Canny", inputTemp);
    Canny( inputTemp, threshold_output, 0, 0, 3 );

    Mat element2 = getStructuringElement(MORPH_RECT, Size(2, 2));
    dilate(threshold_output, threshold_output, element2, Point(-1,-1), 1);//侵蝕
    imshow("after Canny", threshold_output);

    //找輪廓存到contours
    findContours( inputTemp, contours, hierarchy, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    //宣告大約輪廓（減少Point），最小矩形，有可能為身份證的矩形
    vector<vector<Point> > contours_poly( contours.size() );
    vector<RotatedRect> minRect( contours.size() );
    vector<vector<Point> > identityNumber;
    vector<Rect> boundRect;

    //將每個輪廓取出大約的輪廓並將大約的輪廓用最小矩形包起
    for( int i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 5, true );
        minRect[i] = minAreaRect( (Mat)contours[i] );
    }

    //宣告畫布，方便觀察
    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );

    //暫存前置
    int alphabetIndex = 0;
    int identityNumberIndex = 0;
    int identityNumberMinX = 999;

    for( int i = 0; i < contours_poly.size(); i++ )
    {
        //當前大約輪廓的最小矩形
        Point2f rect_points[4];
        minRect[i].points( rect_points );

        //把矩形存進vector<point>才可為後面算面積
        vector<Point> vectorRectPoints;
        for(int index = 0; index < 4; index++)
            vectorRectPoints.push_back(rect_points[index]);

        //篩選掉大小不合理的大約輪廓
        if(fabs(contourArea(contours[i])) > 800 || fabs(contourArea(vectorRectPoints) < 100))//條件內的不要
            continue;

        identityNumber.push_back(vectorRectPoints);

        SortRectPoint(vectorRectPoints, vectorRectPoints);

        //將最小矩形取正矩形存起
        boundRect.push_back(boundingRect(Mat(vectorRectPoints)));
        identityNumberIndex++;

        //cout << "i = " << i << ", and fabs(contourArea(contours[" << i << "]) = " << fabs(contourArea(vectorRectPoints)) << endl;

        //將這些大約輪廓畫在drawing上
        Scalar color = Scalar(255, 255, 255);
        drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );

        //將找到數字的矩形畫在drawing
        rectangle(drawing, boundRect[identityNumberIndex - 1], Scalar(0, 255, 0), 1);
    }

    Mat maskNum(input.size(), CV_8UC1, Scalar::all(255));
    for(int index = 0; index < boundRect.size(); index++)
    {
        rectangle(maskNum, boundRect[index], Scalar(0, 0, 0), -1);
    }
    imshow("mask Num", maskNum);

    //取得最左邊的矩形為字母
    for(int index = 0; index < identityNumber.size(); index++)
    {
        if(identityNumber[index][0].x < identityNumberMinX)
        {
            identityNumberMinX = identityNumber[index][0].x;
            alphabetIndex = index;
        }
    }

    //另外設一個矩形存字母
    Rect alphabetRect(boundRect[alphabetIndex]);

    //找出字母區域與數字區域
    Rect alphabetArea(0, 0, alphabetRect.x + alphabetRect.width, inputTemp.rows);
    Rect numberArea(alphabetRect.x + alphabetRect.width, 0, inputTemp.cols - (alphabetRect.x + alphabetRect.width), inputTemp.rows);

    //將原圖用上面的區域切割成兩個子區域（字母和字號）
    input(Rect(alphabetArea)).copyTo(outputAlphabet);
    input(Rect(numberArea)).copyTo(outputNumber);

    //把字母正矩形劃上畫布
    rectangle(drawing, boundRect[alphabetIndex], Scalar(0, 0, 255), 1);

    //顯示最終畫布
//    imshow( "identity number all", drawing );
//    imshow( "output alphabet", outputAlphabet );
//    imshow( "output number", outputNumber );
}

//把輸入的身份證字號影像分割成英文和數字兩個Mat輸出，輸入影像為三通道
void SeparateIdentityNumberMethod2(Mat& input, Mat& outputAlphabet, Mat& outputNumber)
{
    //複製原影像進行處理
    Mat inputTemp = input.clone();

    //將影像進行灰階
    cvtColor( inputTemp, inputTemp, CV_BGR2GRAY );

    //將影像進行濾波
    BinaryFilterByThresh(inputTemp, inputTemp);

    //找輪廓前置
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //將影像進行膨脹
//    Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
//    //dilate(inputTemp, inputTemp, element, Point(-1,-1), 1);//侵蝕
//    erode(inputTemp, inputTemp, element, Point(-1,-1), 1);//膨脹
//    imshow("closeImg", inputTemp);

    //canny取輪廓
    imshow("before Canny", inputTemp);
    Canny( inputTemp, threshold_output, 0, 0, 3 );
    imshow("after Canny", threshold_output);

    Mat element2 = getStructuringElement(MORPH_RECT, Size(2, 2));
    dilate(threshold_output, threshold_output, element2, Point(-1,-1), 1);//侵蝕
//    imshow("after Canny", threshold_output);

    //找輪廓存到contours
    findContours( threshold_output, contours, hierarchy, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    //宣告大約輪廓（減少Point），最小矩形，有可能為身份證的矩形
    //vector<vector<Point> > contours_poly( contours.size() );
    vector<RotatedRect> minRect( contours.size() );
    vector<vector<Point> > identityNumber;
    vector<Rect> boundRect;

    //將每個輪廓取出大約的輪廓並將大約的輪廓用最小矩形包起
    for( int i = 0; i < contours.size(); i++ )
    {
        //approxPolyDP( Mat(contours[i]), contours_poly[i], 5, true );
        minRect[i] = minAreaRect( (Mat)contours[i] );
    }

    //宣告畫布，方便觀察
    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );

    //暫存前置
    int alphabetIndex = 0;
    int identityNumberIndex = 0;
    int identityNumberMinX = 999;

    for( int i = 0; i < contours.size(); i++ )
    {
        //當前大約輪廓的最小矩形
        Point2f rect_points[4];
        minRect[i].points( rect_points );

        //把矩形存進vector<point>才可為後面算面積
        vector<Point> vectorRectPoints;
        for(int index = 0; index < 4; index++)
            vectorRectPoints.push_back(rect_points[index]);

        //篩選掉大小不合理的大約輪廓
        if(fabs(contourArea(contours[i])) > 800 || fabs(contourArea(vectorRectPoints) < 100))//條件內的不要
            continue;

        SortRectPoint(vectorRectPoints, vectorRectPoints);

        identityNumber.push_back(vectorRectPoints);

        //將最小矩形取正矩形存起
        boundRect.push_back(boundingRect(Mat(vectorRectPoints)));
        identityNumberIndex++;

        //cout << "i = " << i << ", and fabs(contourArea(contours[" << i << "]) = " << fabs(contourArea(vectorRectPoints)) << endl;

        //將這些大約輪廓畫在drawing上
        Scalar color = Scalar(255, 255, 255);
        drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );

        //將找到數字的矩形畫在drawing
        rectangle(drawing, boundRect[identityNumberIndex - 1], Scalar(0, 255, 0), 1);
    }

    Mat maskNum(input.size(), CV_8UC1, Scalar::all(255));
    for(int index = 0; index < boundRect.size(); index++)
    {
        rectangle(maskNum, boundRect[index], Scalar(0, 0, 0), -1);
    }
    imshow("mask Num", maskNum);

    Canny(maskNum, maskNum, 0, 0, 3);

    vector<vector<Point> > maskContours;
    findContours(maskNum, maskContours, hierarchy, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    Mat drawing2 = Mat::zeros( threshold_output.size(), CV_8UC3 );
    vector<Rect> maskRect;
    int rectIndexTemp = 0;
    for(int index = 0; index < maskContours.size(); index++)
    {
        Rect rectTemp = boundingRect(maskContours[index]);
        if(rectTemp.width > 35)continue;
        //drawContours(drawing2, maskContours, index, Scalar(255, 255, 255));
        maskRect.push_back(boundingRect(maskContours[index]));
        rectangle(drawing2, maskRect[rectIndexTemp], Scalar(255, 255, 255), -1);
        //cout <<  "maskRect = " << maskRect[rectIndexTemp] <<endl;
        rectIndexTemp++;
    }
    imshow("drawing2", drawing2);

    //取得最左邊的矩形為字母
    for(int index = 0; index < maskRect.size(); index++)
    {
        if(maskRect[index].x < identityNumberMinX)
        {
            identityNumberMinX = maskRect[index].x;
            alphabetIndex = index;
        }
    }
    //cout << "alphabetIndex = " << alphabetIndex << maskRect[alphabetIndex]  <<endl;

    //另外設一個矩形存字母
    Rect alphabetRect(maskRect[alphabetIndex]);

    //找出字母區域與數字區域
    Rect alphabetArea(0, 0, alphabetRect.x + alphabetRect.width, inputTemp.rows);
    Rect numberArea(alphabetRect.x + alphabetRect.width, 0, inputTemp.cols - (alphabetRect.x + alphabetRect.width), inputTemp.rows);

    //將原圖用上面的區域切割成兩個子區域（字母和字號）
    input(Rect(alphabetArea)).copyTo(outputAlphabet);
    input(Rect(numberArea)).copyTo(outputNumber);

    //把字母正矩形劃上畫布
    rectangle(drawing, maskRect[alphabetIndex], Scalar(0, 0, 255), 1);

    //顯示最終畫布
    imshow( "identity number all", drawing );
    imshow( "output alphabet", outputAlphabet );
    imshow( "output number", outputNumber );
}

//////////////////////////////////////////////////////
//以下是沒使用但日後可以參考或擴增的功能//
//////////////////////////////////////////////////////

//原是sort使用的struct
//struct str{
//    bool operator() ( Point a, Point b ){
//        if ( a.y != b.y )
//            return a.y < b.y;
//        return a.x <= b.x ;
//    }
//} comp;

//原為存使用者指定座標的struct
//struct userdata{
//    Mat im;
//    vector<Point2f> points;
//};

//原為讓使用者自己店選影像上四點座標之handler
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
