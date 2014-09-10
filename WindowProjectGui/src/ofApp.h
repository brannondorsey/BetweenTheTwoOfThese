#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ModelFace.h"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void initMeshFaces();
    
    int nearestFaceIndex;
    int modalDistance;
    bool isPaused;
    
    ofVec3f nearestVertex;
    
    std::vector<ModelFace> model1Faces;
    std::vector<ModelFace> model2Faces;
    
    ofMesh model1Mesh;
    ofMesh model2Mesh;
    
    ofEasyCam camera;
    ofMaterial material;
    ofLight light;
    ofxAssimpModelLoader model;
    
};
