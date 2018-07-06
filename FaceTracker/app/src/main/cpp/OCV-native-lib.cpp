#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>

#include <android/log.h>

using namespace std;
using namespace cv;


#define  LOG_TAG    "OCV-Native"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


extern "C"
{

JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_OpenCVdetector(JNIEnv *env, jclass instance,
                                                                jlong inputAddrMat, jlong matRects);

JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_loadResources(JNIEnv *env, jobject instance);

inline void vector_Rect_to_Mat(std::vector<Rect>& v_rect, Mat& mat)
{
    mat = Mat(v_rect, true);
}


CascadeClassifier face_cascade;

vector<Rect> detect(Mat &gray) {

    std::vector<Rect> faces = {};
    face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, Size(20, 20), Size(1000, 1000));

    return faces;
}


//New HAAR detection function to reduce false detection
std::vector<Rect> detectRF(Mat &gray) {

    double const TH_weight=5.0;//Good empirical threshold values: 5-7
    std::vector<int> reject_levels;
    std::vector<double> weights;

    std::vector<Rect> faces = {};
    std::vector<Rect> realfaces = {};
    face_cascade.detectMultiScale( gray, faces, reject_levels, weights, 1.1, 5, 0|CV_HAAR_SCALE_IMAGE, Size(), Size(1000,1000), true );

    int i=0;
    for(vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ ) {
        LOGI("weights[i]:%f", weights[i]);
        if (weights[i] >= TH_weight)//Good empirical threshold values: 5-7
        {
            //LOGI("weightsACCEPTED[i]:%f", weights[i]);
            realfaces.push_back(*r);
        }
    }
    LOGI("#realFaces: %i", (int)faces.size());
    return realfaces;
}


JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_loadResources(JNIEnv *env, jobject instance) {

    String face_cascade_name = "/sdcard/Download/haarcascade_frontalface_default.xml";

    if (!face_cascade.load(face_cascade_name)) {
        LOGE("OCV resources NOT loaded");
        return;
    } else {
        LOGI("OCV resources loaded");
    }
}


JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_OpenCVdetector(JNIEnv *env, jclass instance,
                                                                jlong inputAddrMat, jlong matRects) {

    vector<Rect> faces;

    Mat &origImg = *((Mat *) inputAddrMat);
    Mat mGray;
    cv::cvtColor(origImg, mGray, CV_BGR2GRAY);

    //faces = detect (mGray);

    /* for (int i = 0; i < faces.size(); i++)
         rectangle(origImg, Point(faces[i].x,faces[i].y),
                            Point(faces[i].x+faces[i].width,faces[i].y+faces[i].height),
                            Scalar(255, 255, 255), 3);*/

    faces = detectRF(mGray);
    for (int i = 0; i < faces.size(); i++) {
    rectangle(origImg, faces[i], Scalar(255, 255, 0), 4, 8, 0);
    }

    //vector_Rect_to_Mat(faces, *((Mat*)matRects));
    vector_Rect_to_Mat(faces, *((Mat*)matRects));
}
}
