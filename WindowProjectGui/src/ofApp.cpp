#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    ofSetWindowShape(1200, 500);
//    ofSetFullscreen(true);
//    ofSetWindowShape(768 * 2, 72 * 2); // real aspect ratio
    ofBackground(0);
    
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
    
    // gui
    gui = new ofxUIScrollableCanvas();
    gui->setScrollAreaToScreen();
    gui->setScrollableDirections(false, true);

    gui->setColorBack(ofColor(255, 50));
    
    std::vector<std::string> names;
    names.push_back("EASY CAM");
    names.push_back("FIXED CAM");
    
//    gui->addSpacer();
//    gui->addLabel("CAMERA");
//	  gui->addRadio("CURRENT CAMERA", names, OFX_UI_ORIENTATION_VERTICAL);
//    gui->addSlider("CAMERA FOV", 0.0, 180.0, camera.getFov());
//    gui->addSlider("CAMERA ASPECT RATIO", 0.0, 15.0, camera.getAspectRatio());
//    gui->addSlider("CAMERA NEAR CLIP", 0.0, 1000.0, camera.getNearClip());
//    gui->addSlider("CAMERA FAR CLIP", 0.0, 5000.0, camera.getFarClip());
    
    gui->addSpacer();
    gui->addLabel("PARTICLES");
    gui->addRangeSlider("SPEED", 0.2, 7, 1.5, 2.5);
    gui->addRangeSlider("ROTATION SPEED", 0.0, 3.0, 0.5, 1.5);
    
    gui->addSpacer();
    gui->addLabel("BOUNDING BOX");
    gui->addToggle("DRAW BOX", &bShowBoundingBox);
    gui->addSlider("BOX WIDTH", 1.0, modelDistance, modelDistance - 200.0);
    gui->addSlider("BOX HEIGHT", 1.0, 1000.0, 300);
    gui->addSlider("BOX DEPTH", 1.0, 4000.0, 200.0);
    gui->addSlider("BOX X", - modelDistance/2, modelDistance/2, 0.0);
    gui->addSlider("BOX Y", - modelDistance/2, modelDistance/2, 100.0);
    gui->addSlider("BOX Z", - modelDistance/2, modelDistance/2, -155.0);
    gui->addButton("CENTER BOX", false);
    
    gui->autoSizeToFitWidgets();
    gui->setPosition(ofGetWidth() - gui->getRect()->getWidth(), 0);
    
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
    gui->loadSettings("settings.xml");

    // Particle bounding box
    float boxX = ((ofxUISlider *) gui->getWidget("BOX X"))->getScaledValue();
    float boxY = ((ofxUISlider *) gui->getWidget("BOX Y"))->getScaledValue();
    float boxZ = ((ofxUISlider *) gui->getWidget("BOX Z"))->getScaledValue();
    boundingBox.set(boxX, boxY, boxZ);
    
    float boxWidth = ((ofxUISlider *) gui->getWidget("BOX WIDTH"))->getScaledValue();
    float boxHeight = ((ofxUISlider *) gui->getWidget("BOX HEIGHT"))->getScaledValue();
    float boxDepth = ((ofxUISlider *) gui->getWidget("BOX DEPTH"))->getScaledValue();
    boundingBox.setWidth(boxWidth);
    boundingBox.setHeight(boxHeight);
    boundingBox.setDepth(boxDepth);
    
    boundingBox.setResolution(1);
    
    // misc
    isPaused = false;
    bShowBoundingBox = false;
    bBoundingBoxChanged = false;
    
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
    
    if (bBoundingBoxChanged) {
        
        for (int i = 0; i < model1Faces.size(); i++) {

            ModelFace& model1Face = model1Faces[i];
            ModelFace& model2Face = model2Faces[i];
            
            if (!model1Face.isDislodged()) {
                model1Face.setWaitPosition(getPointInBoundingBox());
            }
            
            if (!model2Face.isDislodged()) {
                model2Face.setWaitPosition(getPointInBoundingBox());
            }
        }
        
        bBoundingBoxChanged = false;
    }
}

void ofApp::exit() {
    
    gui->saveSettings("settings.xml");
    delete gui;
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
    message += "Press 'h' to toggle GUI\n";
    
    ofSetColor(255);
    ofDrawBitmapString(message,  15, 15);
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
    
    for (int i = 0; i < model1Faces.size(); i++) {
        
        model1Faces[i] = ModelFace(mesh1Faces[i], i);
        model2Faces[i] = ModelFace(mesh2Faces[i], i);
        
        model1Faces[i].setTarget(model2Faces[i].getPosition(), ofVec3f(0, 180, 0));
        model2Faces[i].setTarget(model1Faces[i].getPosition(), ofVec3f(0, -180, 0));
        
        model1Faces[i].setWaitPosition(getPointInBoundingBox());
        model2Faces[i].setWaitPosition(getPointInBoundingBox());
        
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

ofVec3f ofApp::getPointInBoundingBox() {
    
    ofVec3f bBoxPosition = boundingBox.getPosition();
    float bBoxWidth = boundingBox.getWidth();
    float bBoxHeight = boundingBox.getHeight();
    float bBoxDepth = boundingBox.getDepth();
    
    return ofVec3f(ofRandom(bBoxPosition.x - bBoxWidth/2, bBoxPosition.x + bBoxWidth/2),
                   ofRandom(bBoxPosition.y - bBoxHeight/2, bBoxPosition.y + bBoxHeight/2),
                   ofRandom(bBoxPosition.z - bBoxDepth/2, bBoxPosition.z + bBoxDepth/2));
}

void ofApp::guiEvent(ofxUIEventArgs &e) {
    
    
    //CAMERA
    if (e.getName() == "CAMERA FOV") {
        camera.setFov(e.getSlider()->getScaledValue());
    }
    
    if (e.getName() == "CAMERA ASPECT RATIO") {
        camera.setAspectRatio(e.getSlider()->getScaledValue());
    }
    
    if (e.getName() == "CAMERA NEAR CLIP") {
        camera.setNearClip(e.getSlider()->getScaledValue());
    }
    
    if (e.getName() == "CAMERA FAR CLIP") {
        camera.setFarClip(e.getSlider()->getScaledValue());
    }
    
    
    // BOUNDING BOX
    if (e.getName() == "BOX WIDTH") {
        boundingBox.setWidth(e.getSlider()->getScaledValue());
        bBoundingBoxChanged = true;
    }
    
    if (e.getName() == "BOX HEIGHT") {
        boundingBox.setHeight(e.getSlider()->getScaledValue());
        bBoundingBoxChanged = true;
    }
    
    if (e.getName() == "BOX DEPTH") {
        boundingBox.setDepth(e.getSlider()->getScaledValue());
        bBoundingBoxChanged = true;
    }
    
    if (e.getName() == "BOX X") {
        
        ofVec3f position = boundingBox.getPosition();
        position.x = e.getSlider()->getScaledValue();
        boundingBox.setPosition(position);
        bBoundingBoxChanged = true;
    }
    
    if (e.getName() == "BOX Y") {
        
        ofVec3f position = boundingBox.getPosition();
        position.y = e.getSlider()->getScaledValue();
        boundingBox.setPosition(position);
        bBoundingBoxChanged = true;
    }
    
    if (e.getName() == "BOX Z") {
        
        ofVec3f position = boundingBox.getPosition();
        position.z = e.getSlider()->getScaledValue();
        boundingBox.setPosition(position);
        bBoundingBoxChanged = true;
    }
    
    if (e.getName() == "CENTER BOX") {
        boundingBox.setPosition(0, 0, 0);
        bBoundingBoxChanged = true;
    }
    
    
    // PARTICLES
    if(e.getName() == "SPEED")
    {
        ofxUIRangeSlider *slider = (ofxUIRangeSlider*) e.widget;
        
        float min = slider->getValueLow();
        float max = slider->getValueHigh();
        
        for (int i = 0; i < model1Faces.size(); i++) {
            model1Faces[i].setSpeed(min, max);
            model2Faces[i].setSpeed(min, max);
        }
    }
    
    if(e.getName() == "ROTATION SPEED")
    {
        ofxUIRangeSlider *slider = (ofxUIRangeSlider*) e.widget;
        
        float min = slider->getValueLow();
        float max = slider->getValueHigh();
        
        for (int i = 0; i < model1Faces.size(); i++) {
            model1Faces[i].setRotationSpeed(min, max);
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if (key == ' ') {
        isPaused = !isPaused;
    } else if (key == 'f') {
        ofToggleFullscreen();
    } else if (key == 'r') {
        initMeshFaces();
    } else if (key == 'h') {
        gui->toggleVisible();
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
