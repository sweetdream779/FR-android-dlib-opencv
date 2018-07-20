#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>

#include <android/log.h>
#include <math.h>
#include "tracking/tracker.hpp" //MLtracker


using namespace std;
using namespace cv;


#define  LOG_TAG    "OCV-Native"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

extern "C"
{

JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_OpenCVdetector(JNIEnv *env, jclass instance,
                                                                jlong inputAddrMat, jlong matRects);

/*JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_OpenCVdetector(JNIEnv *env, jclass instance,
                                                                jlong inputAddrMat);*/

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

struct byArea {
    bool operator()(const Rect &a, const Rect &b) {
        return a.width * a.height < b.width * b.height;
    }
};

//New HAAR detection function to reduce false detection
std::vector<Rect> detectRF(Mat &gray) {

    //new part----------------------------------------
    Mat img_hsv,s_img;
    cvtColor(gray,img_hsv,CV_RGB2HSV);
    std::vector<cv::Mat> channels;
    cv::split(img_hsv, channels);//[0] = H;[1] = S;[2] = V;
    s_img=channels[2];//value channel: 2
    //end new part------------------------------------

    double const TH_weight=4.0;//Good empirical threshold values: 5-7 (INDOOR) - 4.0 (outdoor)
                               //NOTE: The detection range depends on this threshold value.
                               //      By reducing this value, you increase the possibility
                               //      to detect a smaller face even at a longer distance,
                               // but also to have false detections.
    std::vector<int> reject_levels;
    std::vector<double> weights;

    std::vector<Rect> faces = {};
    std::vector<Rect> realfaces = {};
    //face_cascade.detectMultiScale( gray, faces, reject_levels, weights, 1.1, 5, 0|CV_HAAR_SCALE_IMAGE, Size(), Size(1000,1000), true );
    face_cascade.detectMultiScale(s_img, faces, reject_levels, weights, 1.1, 5, 0|CV_HAAR_SCALE_IMAGE, Size(), Size(1000,1000), true );
    int i=0;
    for(vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ ) {
        LOGI("weights[%i]:%f, sizeFace[i]: %i", i, weights[i], faces[i].width);
        if (weights[i] >= TH_weight)//Good empirical threshold values: 5-7
        {
            //LOGI("weightsACCEPTED[i]:%f", weights[i]);
            realfaces.push_back(*r);
            //for debugging
            rectangle(gray, realfaces[i], Scalar(255, 255, 0), 2, 8, 0);//4, 8, 0
        }
    }
    LOGI("#realFaces: %i (TH_weight= %.2f)", (int)faces.size(), TH_weight);
    sort( realfaces.begin(), realfaces.end(), byArea() );


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


/*
JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_OpenCVdetector(JNIEnv *env, jclass instance,
                                                                jlong inputAddrMat, jlong matRects) {

    vector<Rect> faces;

    Mat &origImg = *((Mat *) inputAddrMat);
    Mat mGray;
    cv::cvtColor(origImg, mGray, CV_BGR2GRAY);

    faces = detectRF(mGray);
    for (int i = 0; i < faces.size(); i++) {
        rectangle(origImg, faces[i], Scalar(255, 255, 0), 4, 8, 0);
    }

    //vector_Rect_to_Mat(faces, *((Mat*)matRects));
  ////  vector_Rect_to_Mat(faces, *((Mat*)matRects));
    *((Mat*)matRects) = Mat(faces, true);
}
*/



// draw the tracked object (using multitracker_alt class)
void DrawTrackedOBJ(MultiTracker &trackers, cv::Mat &Rgb, cv::Scalar &color) {
    LOGI("Inside DrawTrackedOBJ fn");
    for (size_t i = 0; i < trackers.getObjects().size(); i++) {
        rectangle(Rgb, trackers.getObjects().at(i), color, 4, 8, 0);
    }
}
//Face Tracking
// create a tracker object
Ptr<Tracker> tracker= TrackerMedianFlow::create();

//Opencv Tracker
// create multitracker
MultiTracker trackers;//This class is used to track multiple objects using the specified tracker algorithm.
MultiTracker* mytrackers=NULL;

std::vector<Ptr<Tracker> > algorithms;//tracking algorithm
// set the default tracking algorithm
String trackingAlg = "MEDIAN_FLOW"; //default tracking Algorithm
// container of the tracked objects
vector<Rect2d> trackedFaces;
bool firstTime = true;
bool initOK=false;
bool updateOK;
int frame_num = 0;
bool foundFaces=false;

inline cv::Ptr<cv::Tracker> createTrackerByName(cv::String name)
{
    cv::Ptr<cv::Tracker> tracker;

    if (name == "KCF")
        tracker = cv::TrackerKCF::create();
    else if (name == "TLD")
        tracker = cv::TrackerTLD::create();
    else if (name == "BOOSTING")
        tracker = cv::TrackerBoosting::create();
    else if (name == "MEDIAN_FLOW")
        tracker = cv::TrackerMedianFlow::create();
    else if (name == "MIL")
        tracker = cv::TrackerMIL::create();
    else if (name == "GOTURN")
        tracker = cv::TrackerGOTURN::create();
    else
        CV_Error(cv::Error::StsBadArg, "Invalid tracking algorithm name\n");

    return tracker;
}

JNIEXPORT void JNICALL
Java_org_opencv_android_facetracker_HaarDetector_OpenCVdetector(JNIEnv *env, jclass instance,
                                                                jlong inputAddrMat, jlong matRects) {

      Mat &origImg = *((Mat *) inputAddrMat);
      Mat mGray;
      cv::cvtColor(origImg, mGray, CV_BGR2GRAY);
      vector<Rect> BBfaces, oldFaces, tfaces;

      int currNumFaces=0;
      int oldNumFaces=0;
      ////BBfaces = detectRF(mGray);//new_version
      BBfaces = detectRF(origImg);//new_version with HSV conversion

    /*  for (int i = 0; i < BBfaces.size(); i++) {
          rectangle(origImg, BBfaces[i], Scalar(255, 255, 0), 2, 8, 0);//4, 8, 0
      }*/

      //Face tracking
      if(BBfaces.size()>0) {

          if(firstTime)
          {
              for (size_t i = 0; i <BBfaces.size(); i++)
              {
                  foundFaces =true;

                  //Tracker initialization
                  algorithms.push_back(createTrackerByName(trackingAlg));//trackers creation
                  trackedFaces.push_back(BBfaces[i]);
                  LOGI("#trackedFaces:%i", (int)trackedFaces.size());
                  tfaces.push_back(trackedFaces.at(i));//new part

                  //create faces history
                  oldFaces.push_back(BBfaces.at(i));//create history faces
              } // end for
              firstTime = false;
              oldNumFaces = (int)oldFaces.size();
              LOGI("#oldNumFaces:%i", oldNumFaces);
              trackers.add(algorithms,origImg,trackedFaces);

          } // end if first time
          else
          {

              //check for the variation of the detected faces number
              if(currNumFaces!=trackers.getObjects().size())
              {
                  if(currNumFaces>oldNumFaces)
                  {
                      vector<Rect2d> newTrackedFaces;

                      algorithms.clear();
                      trackedFaces.clear();
                      mytrackers=trackers.create();
                      trackers=*mytrackers;

                      for (size_t i = 0; i <currNumFaces; i++)
                      {
                          //Tracker initialization
                          algorithms.push_back(createTrackerByName(trackingAlg));//trackers creation
                          newTrackedFaces.push_back(BBfaces[BBfaces.size()-1-i]);//add last detected faces
                          tfaces.push_back(newTrackedFaces.at(i));//new part
                      } // end for
                      trackers.add(algorithms,origImg,newTrackedFaces);

                  }
                  else if (currNumFaces<oldNumFaces)
                  {

                      if(currNumFaces<trackers.getObjects().size())
                      {
                          __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "REMOVE tracker(s)_Num: %i",(int)trackers.getObjects().size());
                      }
                  }
              }//if(currNumFaces!=trackers.targetNum)

              for(int i=0;i<BBfaces.size();i++)
              {

                  vector<Rect2d> roi;
                  roi.push_back(BBfaces[BBfaces.size()-1+i]);
                  updateOK = trackers.update(origImg,roi);
                  LOGI("updateOK: %i",(int)updateOK);
                  //Draw the Bounding Boxes of the tracked faces
                  if(updateOK)
                  {
                      //new part-----------------------------
                      for (size_t i = 0; i < trackers.getObjects().size(); i++) {
                          tfaces.push_back(trackers.getObjects().at(i));
                      }
                      //end new part-------------------------------
                      LOGI("updateOK: %i -> DrawTrackedFaces",(int)updateOK);
                     /* cv::Scalar blue=cv::Scalar(0,0,255) ;
                      DrawTrackedOBJ(trackers,origImg, blue);*/
                  }
                  else
                  {
                      LOGI(" if(updateOK==0), #algorithms: %i", (int)algorithms.size());
                      algorithms.clear();
                      trackedFaces.clear();
                      mytrackers=trackers.create();
                      trackers=*mytrackers;
                      trackers.create();
                      firstTime=true;

                  }
              }


              //clear faces history & update it
              oldFaces.clear();
              for (size_t i = 0; i < BBfaces.size(); i++)
              {
                  oldFaces.push_back(BBfaces.at(i));
              }
              oldNumFaces = (int)oldFaces.size();
          }//else firstTime
      }
      else // there are no faces
      {
          if (oldNumFaces >0)
          {
              //continue to track the old faces (only trackers updating)
              updateOK=trackers.update(origImg);
              if(updateOK)
              {
                  /*cv::Scalar blue=cv::Scalar(255,0,0) ;
                  DrawTrackedOBJ(trackers, origImg, blue);*/
                  //new part-----------------------------
                  for (size_t i = 0; i < trackers.getObjects().size(); i++) {
                      tfaces.push_back(trackers.getObjects().at(i));
                  }
                  //end new part-------------------------------
              }
              else
              {
                  foundFaces=false;
              }
          }//if (oldNumFaces !=0)
          else
          {
              __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "No faces history");
          }
      }
    //--------------------------------------------
////    *((Mat*)matRects) = Mat(BBfaces, true);

    *((Mat*)matRects) = Mat(tfaces, true);
}



}
