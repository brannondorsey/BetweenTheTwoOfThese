//
//  MotionDetector.cpp
//  ofApp
//
//  Created by bdorse on 9/16/14.
//
//
#include "MotionDetector.h"

MotionDetector::MotionDetector():
_bUseLiveVideo(false),
_bEnabled(true),
_interval(100),
_tiltAngle(0),
_nearClip(70),
_farClip(230),
_millisAtLastInterval(0),
_bIntervalEllapsedThisFrame(false),
_bThresholdCrossed(false),
_diffThreshold(0),
_frameDiff(0)
{
    
}

void MotionDetector::setup(int deviceId) {

    _kinect.setRegistration(true);
    _kinect.init(true);
    _kinect.open(deviceId);
    
    _video.loadMovie(ofToDataPath("kinect_recording.mov"));
    _video.setLoopState(OF_LOOP_NORMAL);
    
    _displayImage.allocate(_kinect.getWidth(), _kinect.getHeight(), OF_IMAGE_GRAYSCALE);
    _displayImage.setFromPixels(_video.getPixelsRef());
    
    setUseLiveVideo(false);
    
    if (!usingLiveVideo()) {
        _video.play();
    }
}

void MotionDetector::update() {

    _video.update();
    if (isConnected()) _kinect.update();
    
    ofPixels& pixelsRef = (usingLiveVideo() && isConnected()) ? _kinect.getDepthPixelsRef() : _video.getPixelsRef();
    
    if ((!usingLiveVideo() && _video.isFrameNew()) ||
        (usingLiveVideo() && _kinect.isFrameNew())) {

//        for(int i = 0; i < pixelsRef.size(); i++) {
//            if(pixelsRef[i] > _nearClip && pixelsRef[i] < _farClip) {
//                pixelsRef[i] = 255;
//            } else {
//                pixelsRef[i] = 0;
//            }
//        }
        _displayImage.setFromPixels(pixelsRef);
        
        // take the absolute difference of prev and cam and save it inside diff
        ofxCv::absdiff(_previous, pixelsRef, _diff);
		
		// like ofSetPixels, but more concise and cross-toolkit
        ofxCv::copy(pixelsRef, _previous);
		
		// mean() returns a Scalar. it's a cv:: function so we have to pass a Mat
        ofxCv::Scalar diffMean = ofxCv::mean(ofxCv::toCv(_diff));
        // calculate percentage of image that is different this frame
        _frameDiff = ofMap(diffMean[0], 0, 255, 0, 1);
        
        if (_frameDiff >= _diffThreshold) {
            // threshold was crossed during this interval
            _bThresholdCrossed = true;
        }

    }
    
    if (ofGetElapsedTimeMillis() - _millisAtLastInterval >= _interval) {
        _bIntervalEllapsedThisFrame = true;
        _millisAtLastInterval = ofGetElapsedTimeMillis();
    } else {
        _bIntervalEllapsedThisFrame = false;
    }
}

void MotionDetector::drawPreview(int x, int y) {
    _displayImage.draw(x, y);
}

void MotionDetector::drawDiffPreview(int x, int y) {
    _diff.draw(x, y);
}

void MotionDetector::setInterval(int millis) {
    _interval = millis;
}

void MotionDetector::setDifferenceThreshold(float threshold) {
    _diffThreshold = ofClamp(threshold, 0.0, 1.0);
}

void MotionDetector::setUseLiveVideo(bool b) {
    
    _bUseLiveVideo = b;
    
    ofPixels pixels;
    if (usingLiveVideo() &&
        isConnected()) {
        _video.stop();
        pixels = _kinect.getDepthPixelsRef();
    } else {
        _video.play();
        pixels = _video.getPixelsRef();
    }

    ofxCv::imitate(_previous, pixels);
    ofxCv::imitate(_diff, pixels);
}

void MotionDetector::setEnabled(bool b) {
    _bEnabled = b;
}

void MotionDetector::setTiltAngle(int angle) {
    
    if (isConnected()) {
        if (angle > 30) angle = 30;
        if (angle < -30) angle = -30;
        _tiltAngle = angle;
        _kinect.setCameraTiltAngle(_tiltAngle);
    }
}

void MotionDetector::setNearClip(int clip) {
    _nearClip = clip;
}

void MotionDetector::setFarClip(int clip) {
    _farClip = clip;
}

bool MotionDetector::motionDetected() {
    
    if (_bIntervalEllapsedThisFrame &&
        _bThresholdCrossed) {
        _bThresholdCrossed = false; // reset for next interval
        if (isEnabled()) return true;
    }
    
    return false;
}

bool MotionDetector::usingLiveVideo() {
    return _bUseLiveVideo;
}

bool MotionDetector::isEnabled() {
    return _bEnabled;
}

bool MotionDetector::isConnected() {
    return _kinect.isConnected();
}

float MotionDetector::getThreshold() {
    return _diffThreshold;
}

int MotionDetector::getInterval() {
    return _interval;
}

int MotionDetector::getTiltAngle() {
    return _tiltAngle;
}

int MotionDetector::getNearClip() {
    return _nearClip;
}

int MotionDetector::getFarClip() {
    return _farClip;
}

float MotionDetector::getFrameDifference() {
    return _frameDiff;
}

ofPixels& MotionDetector::getPixelsRef() {
    return usingLiveVideo() ? _kinect.getDepthPixelsRef() : _video.getPixelsRef();
}

ofImage& MotionDetector::getImage() {
    return _displayImage;
}