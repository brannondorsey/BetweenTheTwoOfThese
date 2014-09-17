//
//  MotionDetector.h
//  ofApp
//
//  Created by bdorse on 9/16/14.
//
// This class wraps functionality for using the kinect
// as a night time motion sensor. It uses the kinect's
// depth image and frame differencing to detect percentage
// of movement in the live stream, and compare against a target
// threshold set in the app. It does NOT track.


#ifndef __ofApp__MotionDetector__
#define __ofApp__MotionDetector__

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxCv.h"

class MotionDetector {
public:
    
    MotionDetector();
    
    void setup(int deviceId=-1);
    void update();
    void setInterval(int millis);
    void setUseLiveVideo(bool b);
    void setEnabled(bool b);
    void setDifferenceThreshold(float threshold);
    void drawPreview(int x, int y);
    void drawDiffPreview(int x, int y);
    void setTiltAngle(int angle);
    void setNearClip(int nearClip); // often refered to as threshold
    void setFarClip(int farClip);
    
    bool motionDetected();
    bool usingLiveVideo();
    bool isEnabled();
    bool isConnected();
    
    int getTiltAngle();
    int getNearClip();
    int getFarClip();
    
    float getFrameDifference();
    float getThreshold();
    
    ofPixels& getPixelsRef();
    
protected:
    
    bool _bUseLiveVideo;
    bool _bEnabled;
    bool _bThresholdCrossed;
    bool _bIntervalEllapsedThisFrame;
    
    int _interval; // millis
    int _millisSinceLastInterval;
    int _tiltAngle;
    int _nearClip;
	int _farClip;
    
    unsigned long _millisAtLastInterval;
    
    float _diffThreshold; // float from 0.0 - 1.0
    float _frameDiff; // holds the 0.0 - 1.0 difference between the last frame
    
    ofPixels _previous;
    
    ofImage _displayImage;
	ofImage _diff;
    
    ofVideoPlayer _video;
    
    ofxKinect _kinect;
};

#endif /* defined(__ofApp__MotionDetector__) */
