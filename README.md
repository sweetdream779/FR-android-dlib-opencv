[![Build Status](https://travis-ci.com/hoopoe/FR-android-dlib-opencv.svg?branch=master)](https://travis-ci.com/hoopoe/FR-android-dlib-opencv)

LPR with openALPR library.
============

This app demonstrate semi realtime license plates detection, recognition and tracking based on openalpr.

Notes:

1. To run:
  * download jniLibs from [this link](https://drive.google.com/open?id=1cFe2ZsTcLpimlh39E-VFJZc3MHTouIIm). The folder contains precompiled libs for `ANDROID_PLATFORM="android-21"` and `ANDROID_ABI="armeabi-v7a", "arm64-v8a", "x86_64"`. For compling with another `ANDROID_PLATFORM` or `ANDROID_ABI` use [this script](https://gist.github.com/jav974/072425f14927e6ca2c7a4439d8ac5457).
  * set env variable OPENALPR_LIBS
`export OPENALPR_LIBS=/home/irina/jniLibs`
