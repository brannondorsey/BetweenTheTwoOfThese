#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxUI.h"
#include "ofxDOF.h"
#include "ModelFace.h"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void guiEvent(ofxUIEventArgs &e);
    
    void initMeshFaces();
    void resetCamera();
    ofVec3f getPointInBoundingBox();
    
    int nearestFaceIndex;
    int modelDistance;
    float startCameraFOV;
    float startCameraAspectRatio;
    float startCameraNearClip;
    float startCameraFarClip;
    float cameraDistance;
    float cameraXOrbit;
    float cameraYOrbit;
    bool isPaused;
    bool bShowBoundingBox;
    bool bBoundingBoxChanged;
    bool bDOFEnabled;
    bool bDrawDOFFocusAssist;
    
    ofVec3f nearestVertex;
    
    std::vector<ModelFace> model1Faces;
    std::vector<ModelFace> model2Faces;
    
    ofMesh model1Mesh;
    ofMesh model2Mesh;
    
    ofBoxPrimitive boundingBox;
    
    ofEasyCam camera;
    ofMaterial material;
    ofLight light;
    
    ofxAssimpModelLoader model;
    ofxUIScrollableCanvas *gui;
    ofxDOF depthOfField;
};
