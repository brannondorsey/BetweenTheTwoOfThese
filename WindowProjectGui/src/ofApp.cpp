#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    ofSetWindowShape(1200, 500);
//    ofSetFullscreen(true);
//    ofSetWindowShape(768 * 2, 72 * 2); // real aspect ratio
    ofBackground(0);
    
    // gui
//    gui = new ofxUICanvas();
//    
//    gui->addLabel("Window Project", OFX_UI_FONT_LARGE);
//    gui->addRangeSlider("PARTICLE SPEED", 1.5, 20, 1.5, 2.5);
//    gui->addToggle("FULLSCREEN", false);
//    gui->autoSizeToFitWidgets();
//    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
//    gui->loadSettings("settings.xml");
    
    // camera
    camera.setDistance(1800);
    
    // lights
    light.setup();
    light.setDirectional();
    light.setAmbientColor(ofFloatColor(0.3));
    
    // materials
    material.setShininess(60);
    material.setSpecularColor(ofFloatColor(0.3));
    
    // model
    model.loadModel("model.dae");
    modelDistance = ofGetWidth() * 1.7;
    
    // Particle bounding box
    boundingBox.set(modelDistance - 200, 300, 200);
    boundingBox.move(0, 100, -155);
    boundingBox.setResolution(1);
    
    // misc
    isPaused = false;
    bShowBoundingBox = false;
    
    initMeshFaces();
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (!isPaused) {
        
        int vertCounter = 0;
        for (int i = 0; i < model1Faces.size(); i++) {
            
            std::vector<ofVec3f>& model1MeshVerts = model1Mesh.getVertices();
            model1Faces[i].update(model1MeshVerts[vertCounter],
                                  model1MeshVerts[vertCounter + 1],
                                  model1MeshVerts[vertCounter + 2]);
            
            std::vector<ofVec3f>& model2MeshVerts = model2Mesh.getVertices();
            model2Faces[i].update(model2MeshVerts[vertCounter],
                                  model2MeshVerts[vertCounter + 1],
                                  model2MeshVerts[vertCounter + 2]);
            
            vertCounter += 3;
        }
        
        float nearestDistance = 0;
        int nearestIndex = 0;
        ofVec2f mouse(mouseX, mouseY);
        
        float model1Dist = camera.worldToScreen(ofVec3f(modelDistance/2, 0, 0)).distance(mouse);
        float model2Dist = camera.worldToScreen(ofVec3f(-(modelDistance/2), 0, 0)).distance(mouse);
        
        std::vector<ModelFace>& modelFaces = (model1Dist < model2Dist) ? model1Faces : model2Faces;
        std::vector<ModelFace>& otherModelFaces = (model1Dist > model2Dist) ? model1Faces : model2Faces;
        
        for (int i = 0; i < model1Faces.size(); i++) {
            
            ModelFace& modelFace = modelFaces[i];
            
            ofVec3f cur = camera.worldToScreen(modelFace.getCentroid());
            float distance = cur.distance(mouse);
            
            if (i == 0 || (distance < nearestDistance && !modelFace.isDislodged())) {
                nearestDistance = distance;
                nearestVertex = cur;
                nearestFaceIndex = i;
            }
        }
        
        if(ofGetKeyPressed(OF_KEY_SHIFT)) {
            
            ModelFace& modelFace = modelFaces[nearestFaceIndex];
            ModelFace& otherModelFace = otherModelFaces[nearestFaceIndex];
            
            modelFace.dislodge();
            otherModelFace.onPartnerDislodged();
            modelFace.setWaiting(!otherModelFace.isDislodged());
        }
    }
}

void ofApp::exit() {
    
//    gui->saveSettings("settings.xml");
//    delete gui;
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofEnableDepthTest();
    
//    light.enable();
    camera.begin();
    material.begin();
    
    ofSetColor(255);
    model1Mesh.draw();
    model2Mesh.draw();
    
    if (bShowBoundingBox) {
        boundingBox.drawWireframe();
    }
    
    ofSetColor(0);
    model1Mesh.drawWireframe();
    model2Mesh.drawWireframe();
    
    material.end();
    camera.end();
    
//    light.disable();
    
    if (!isPaused) {
        
        ofSetColor(ofColor::gray);
        ofLine(nearestVertex.x, nearestVertex.y, mouseX, mouseY);
        
        ofNoFill();
        ofSetColor(ofColor::yellow);
        ofSetLineWidth(2);
        ofCircle(nearestVertex, 4);
        ofSetLineWidth(1);
    }
    
    std::string message = ofToString(ofGetFrameRate()) += " fps\n";
    message += "Hold SHIFT to remove faces\n";
    message += "Press SPACE to pause\n";
    message += "Press 'r' to reset\n";
    message += "Press 'f' to toggle fullscreen\n";
    message += "Press 'b' to toggle particle bounding box\n";
    
    ofSetColor(255);
    ofDrawBitmapString(message,  15, 30);
}

void ofApp::initMeshFaces() {
    
    ofMesh mesh1 = model.getMesh(0);
    ofMesh mesh2 = mesh1;
    
    std::vector<ofVec3f>& mesh1Vertices = mesh1.getVertices();
    std::vector<ofVec3f>& mesh2Vertices = mesh2.getVertices();
    
    ofVec3f centroid = mesh1.getCentroid();
    
    float xOffset = modelDistance/2;
    
    for (int i = 0; i < mesh1Vertices.size(); i++) {
        
        mesh1Vertices[i] = mesh1Vertices[i].rotate(90, centroid, ofVec3f(0, 1, 0));
        mesh1Vertices[i].x += xOffset;
        
        mesh2Vertices[i] = mesh2Vertices[i].rotate(-90, centroid, ofVec3f(0, 1, 0));
        mesh2Vertices[i].x -= xOffset;
    }
    
    model1Faces.clear();
    model2Faces.clear();
    model1Mesh.clear();
    model2Mesh.clear();
    
    std::vector<ofMeshFace> mesh1Faces = mesh1.getUniqueFaces();
    std::vector<ofMeshFace> mesh2Faces = mesh2.getUniqueFaces();
    
    model1Faces.resize(mesh1Faces.size());
    model2Faces.resize(mesh2Faces.size());
    
    ofVec3f bBoxPosition = boundingBox.getPosition();
    float bBoxWidth = boundingBox.getWidth();
    float bBoxHeight = boundingBox.getHeight();
    float bBoxDepth = boundingBox.getDepth();
    
    for (int i = 0; i < model1Faces.size(); i++) {
        
        model1Faces[i] = ModelFace(mesh1Faces[i], i);
        model2Faces[i] = ModelFace(mesh2Faces[i], i);
        
        model1Faces[i].setTarget(model2Faces[i].getPosition(), ofVec3f(0, 180, 0));
        model2Faces[i].setTarget(model1Faces[i].getPosition(), ofVec3f(0, -180, 0));
        
        model1Faces[i].setWaitPosition(ofVec3f(ofRandom(bBoxPosition.x - bBoxWidth/2, bBoxPosition.x + bBoxWidth/2),
                                               ofRandom(bBoxPosition.y - bBoxHeight/2, bBoxPosition.y + bBoxHeight/2),
                                               ofRandom(bBoxPosition.z - bBoxDepth/2, bBoxPosition.z + bBoxDepth/2)));
        
        model2Faces[i].setWaitPosition(ofVec3f(ofRandom(bBoxPosition.x - bBoxWidth/2, bBoxPosition.x + bBoxWidth/2),
                                               ofRandom(bBoxPosition.y - bBoxHeight/2, bBoxPosition.y + bBoxHeight/2),
                                               ofRandom(bBoxPosition.z - bBoxDepth/2, bBoxPosition.z + bBoxDepth/2)));
        
        
//        cout << ofToString(bBoxPosition.x - bBoxWidth/2) << "," << ofToString(bBoxPosition.x + bBoxWidth/2) << endl;
//        cout << ofToString(bBoxPosition.y - bBoxHeight/2) << "," << ofToString(bBoxPosition.y + bBoxHeight/2) << endl;
//        cout << ofToString(bBoxPosition.z - bBoxDepth/2) << "," << ofToString(bBoxPosition.z + bBoxDepth/2) << endl;
//        cout << endl;
        
        std::vector<ofVec3f>& model1FaceVerts = model1Faces[i].getVertices();
        model1Mesh.addVertex(model1FaceVerts[0]);
        model1Mesh.addVertex(model1FaceVerts[1]);
        model1Mesh.addVertex(model1FaceVerts[2]);
        
        std::vector<ofVec3f>& model2FaceVerts = model2Faces[i].getVertices();
        model2Mesh.addVertex(model2FaceVerts[0]);
        model2Mesh.addVertex(model2FaceVerts[1]);
        model2Mesh.addVertex(model2FaceVerts[2]);
    }
}

void ofApp::guiEvent(ofxUIEventArgs &e) {
    
//    if(e.getName() == "PARTICLE SPEED")
//    {
//        ofxUISlider *slider = e.getSlider();
//        cout << slider->getScaledValue() << endl;
//    }
//    else if(e.getName() == "FULLSCREEN")
//    {
//        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
//        ofSetFullscreen(toggle->getValue());
//    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if (key == ' ') {
        isPaused = !isPaused;
    } else if (key == 'f') {
        ofToggleFullscreen();
    } else if (key == 'r') {
        initMeshFaces();
    } else if (key == 'b') {
        bShowBoundingBox = !bShowBoundingBox;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
