#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ModelFace.h"
#include "ofxUI.h"

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
    ofVec3f getPointInBoundingBox();
    
    int nearestFaceIndex;
    int modelDistance;
    float cameraDistance;
    float cameraXOrbit;
    float cameraYOrbit;
    bool isPaused;
    bool bShowBoundingBox;
    bool bBoundingBoxChanged;
    
    ofVec3f nearestVertex;
    
    std::vector<ModelFace> model1Faces;
    std::vector<ModelFace> model2Faces;
    
    ofMesh model1Mesh;
    ofMesh model2Mesh;
    
    ofBoxPrimitive boundingBox;
    
    ofCamera camera;
    ofMaterial material;
    ofLight light;
    ofxAssimpModelLoader model;
    ofxUIScrollableCanvas *gui;
};
