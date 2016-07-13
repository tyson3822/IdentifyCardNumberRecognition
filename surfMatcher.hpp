#include <iostream>
#include <stdio.h>
#include "opencv2/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace cv;
using namespace cv::xfeatures2d;
using namespace std;

const int LOOP_NUM = 10;
const int GOOD_PTS_MAX = 50;
const float GOOD_PORTION = 0.15f;

int64 work_begin = 0;
int64 work_end = 0;

static void workBegin()
{
    work_begin = getTickCount();
}

static void workEnd()
{
    work_end = getTickCount() - work_begin;
}

static double getTime()
{
    return work_end /((double)getTickFrequency() )* 1000.;
}

struct SURFDetector
{
    Ptr<Feature2D> surf;
    SURFDetector(double hessian = 800.0)
    {
        surf = SURF::create(hessian);
    }
    template<class T>
    void operator()(const T& in, const T& mask, vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        surf->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};

template<class KPMatcher>
struct SURFMatcher
{
    KPMatcher matcher;
    template<class T>
    void match(const T& in1, const T& in2, vector<cv::DMatch>& matches)
    {
        matcher.match(in1, in2, matches);
    }
};

static Mat drawGoodMatches(
    const Mat& img1,
    const Mat& img2,
    const vector<KeyPoint>& keypoints1,
    const vector<KeyPoint>& keypoints2,
    vector<DMatch>& matches,
    vector<Point2f>& scene_corners_
    )
{
    //-- Sort matches and preserve top 10% matches
    sort(matches.begin(), matches.end());
    vector< DMatch > good_matches;
    double minDist = matches.front().distance;
    double maxDist = matches.back().distance;

    const int ptsPairs = min(GOOD_PTS_MAX, (int)(matches.size() * GOOD_PORTION));
    for( int i = 0; i < ptsPairs; i++ )
    {
        good_matches.push_back( matches[i] );
    }
    cout << "\nMax distance: " << maxDist << endl;
    cout << "Min distance: " << minDist << endl;

    cout << "Calculating homography using " << ptsPairs << " point pairs." << endl;

    // drawing the results
    Mat img_matches;

    drawMatches( img1, keypoints1, img2, keypoints2,
                 good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                 vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS  );

    //-- Localize the object
    vector<Point2f> obj;
    vector<Point2f> scene;

    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints1[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints2[ good_matches[i].trainIdx ].pt );
    }
    //-- Get the corners from the image_1 ( the object to be "detected" )
    vector<Point2f> obj_corners(4);
    obj_corners[0] = Point(0,0);
    obj_corners[1] = Point( img1.cols, 0 );
    obj_corners[2] = Point( img1.cols, img1.rows );
    obj_corners[3] = Point( 0, img1.rows );
    vector<Point2f> scene_corners(4);

    Mat H = findHomography( obj, scene );
    Mat H2 = findHomography( scene,  obj);
    perspectiveTransform( obj_corners, scene_corners, H);

    //imshow("image1", img1);
    //imshow("image2", img2);
    Size size(img1.cols,img1.rows);
    Mat tempMat = Mat::zeros(size,CV_8UC3);
    warpPerspective(img2, tempMat, H2, size);
    imshow("temp_image", tempMat);

    scene_corners_ = scene_corners;

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    line( img_matches,
          scene_corners[0] + Point2f( (float)img1.cols, 0), scene_corners[1] + Point2f( (float)img1.cols, 0),
          Scalar( 0, 255, 0), 2, LINE_AA );
    line( img_matches,
          scene_corners[1] + Point2f( (float)img1.cols, 0), scene_corners[2] + Point2f( (float)img1.cols, 0),
          Scalar( 0, 255, 0), 2, LINE_AA );
    line( img_matches,
          scene_corners[2] + Point2f( (float)img1.cols, 0), scene_corners[3] + Point2f( (float)img1.cols, 0),
          Scalar( 0, 255, 0), 2, LINE_AA );
    line( img_matches,
          scene_corners[3] + Point2f( (float)img1.cols, 0), scene_corners[0] + Point2f( (float)img1.cols, 0),
          Scalar( 0, 255, 0), 2, LINE_AA );
    return img_matches;
}

static Mat CutGoodArea(
    const Mat& img1,
    const Mat& img2,
    const vector<KeyPoint>& keypoints1,
    const vector<KeyPoint>& keypoints2,
    vector<DMatch>& matches,
    vector<Point2f>& scene_corners_
    )
{
    //-- Sort matches and preserve top 10% matches
    sort(matches.begin(), matches.end());
    vector< DMatch > good_matches;
    double minDist = matches.front().distance;
    double maxDist = matches.back().distance;

    const int ptsPairs = min(GOOD_PTS_MAX, (int)(matches.size() * GOOD_PORTION));
    for( int i = 0; i < ptsPairs; i++ )
    {
        good_matches.push_back( matches[i] );
    }
    cout << "\nMax distance: " << maxDist << endl;
    cout << "Min distance: " << minDist << endl;

    cout << "Calculating homography using " << ptsPairs << " point pairs." << endl;

    // drawing the results
    Mat img_matches;

    drawMatches( img1, keypoints1, img2, keypoints2,
                 good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                 vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS  );

    //-- Localize the object
    vector<Point2f> obj;
    vector<Point2f> scene;

    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints1[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints2[ good_matches[i].trainIdx ].pt );
    }
    //-- Get the corners from the image_1 ( the object to be "detected" )
    vector<Point2f> obj_corners(4);
    obj_corners[0] = Point(0,0);
    obj_corners[1] = Point( img1.cols, 0 );
    obj_corners[2] = Point( img1.cols, img1.rows );
    obj_corners[3] = Point( 0, img1.rows );
    vector<Point2f> scene_corners(4);

    Mat H2 = findHomography( scene,  obj);

    Size size(img1.cols,img1.rows);
    Mat imgGoodArea = Mat::zeros(size,CV_8UC3);
    warpPerspective(img2, imgGoodArea, H2, size);
    imshow("img_GoodArea", imgGoodArea);

    return imgGoodArea;
}

Mat SurfMatch(Mat img1, Mat img2)
{
        double surf_time = 0.;

    //declare input/output
    vector<KeyPoint> keypoints1, keypoints2;
    vector<DMatch> matches;

    UMat _descriptors1, _descriptors2;
    Mat descriptors1 = _descriptors1.getMat(ACCESS_RW),
        descriptors2 = _descriptors2.getMat(ACCESS_RW);

    //instantiate detectors/matchers
    SURFDetector surf;

    SURFMatcher<BFMatcher> matcher;

    //-- start of timing section

    for (int i = 0; i <= LOOP_NUM; i++)
    {
        if(i == 1) workBegin();
        //surf(img1.getMat(ACCESS_READ), Mat(), keypoints1, descriptors1);
        //surf(img2.getMat(ACCESS_READ), Mat(), keypoints2, descriptors2);
        surf(img1, Mat(), keypoints1, descriptors1);
        surf(img2, Mat(), keypoints2, descriptors2);
        matcher.match(descriptors1, descriptors2, matches);
    }
    workEnd();
    cout << "FOUND " << keypoints1.size() << " keypoints on first image" << endl;
    cout << "FOUND " << keypoints2.size() << " keypoints on second image" << endl;

    surf_time = getTime();
    cout << "SURF run time: " << surf_time / LOOP_NUM << " ms" << endl<<"\n";


    vector<Point2f> corner;
    //Mat img_matches = drawGoodMatches(img1.getMat(ACCESS_READ), img2.getMat(ACCESS_READ), keypoints1, keypoints2, matches, corner);
    //Mat img_matches = drawGoodMatches(img1, img2, keypoints1, keypoints2, matches, corner);
    Mat imgIdentifyCard = CutGoodArea(img1, img2, keypoints1, keypoints2, matches, corner);

    //-- Show detected matches

    string outpath = "test.jpg";

    /*namedWindow("surf matches", 0);
    imshow("surf matches", img_matches);
    imwrite(outpath, img_matches);*/

    return imgIdentifyCard;
}
