#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofEnableAntiAliasing();
//    ofSetWindowShape(1280 * 6, 720); // real aspect ratio
    ofSetBackgroundAuto(false);
    ofBackground(0);
    ofSeedRandom(RANDOM_SEED);
    
    // MPE Client
    client.setup("mpe/client_" + ofToString(CLIENT_ID) + "_settings.xml", true); //false means you can use backthread
    client.start();
    
//    cout << "client local width: " << client.getLWidth() << endl;
//    cout << "client local height: " << client.getLHeight() << endl;
//    cout << "client master width: " << client.getMWidth() << endl;
//    cout << "client master height: " << client.getMHeight() << endl;
//    cout << "client x offset: " << client.getXoffset() << endl;

    ofxMPERegisterEvents(this);
    
    ofSetWindowShape(client.getLWidth(), client.getLHeight());
    ofSetWindowPosition(client.getXoffset(), 0); // comment out for release
    
    // fbo
    // cout << fbo.maxSamples() << endl;
    fbo.allocate(client.getMWidth(), client.getMHeight(), GL_RGBA, 4);
    ofClear(0, 255);
    
    // lights
    light.setup();
    light.setDirectional();
    light.setAmbientColor(ofFloatColor(0.01));
    light.setDiffuseColor(ofFloatColor(1.0));
    
    // materials
    material.setShininess(120);
    material.setSpecularColor(ofColor(255));
    
    // model
    model.loadModel("model.dae");
    modelDistance = client.getMWidth() * 1.7;
    
    // camera
    camera.setAspectRatio(float(client.getMWidth())/float(client.getMHeight()));
    camera.setForceAspectRatio(true);
    
    startCameraFOV = camera.getFov();
    startCameraAspectRatio = camera.getAspectRatio();
    startCameraNearClip = camera.getNearClip();
    startCameraFarClip = camera.getFarClip();
    
    // motion detectors (Kinects)
    srandom(RANDOM_SEED);
    mD1.setup();
    mD1.setUseLiveVideo(false);
    bMD1UseLiveVideo = mD1.usingLiveVideo();
    
    mD2.setup();
    mD2.setUseLiveVideo(false);
    bMD2UseLiveVideo = mD2.usingLiveVideo();
    
    // misc before gui
    
    // these must be defined here because gui
    // radio button can't have a value on start
    bDestroyTop1 = true;
    bDestroyTop2 = false;
    
    // gui
    gui = new ofxUIScrollableCanvas();
    gui->setScrollAreaToScreen();
    gui->setScrollableDirections(false, true);

    gui->setColorBack(ofColor(50, 200));

    int maxModelDistance = 10000;
    
    gui->addSpacer();
    gui->addButton("RESET", false);
    
    gui->addSpacer();
    gui->addLabel("CAMERA");
    gui->addButton("RESET CAMERA", false);
    gui->addButton("RESET ASPECT TO WINDOW", false);
    gui->addSpacer();
    gui->addSlider("CAMERA FOV", 0.0, 180.0, camera.getFov());
    gui->addSlider("CAMERA ASPECT RATIO", 0.0, 15.0, camera.getAspectRatio());
    gui->addSlider("CAMERA NEAR CLIP", 0.0, 1000.0, camera.getNearClip());
    gui->addSlider("CAMERA FAR CLIP", 0.0, 5000.0, camera.getFarClip());
    gui->addSpacer();
    gui->addSlider("CAMERA DISTANCE", 100, 6000, 1800);
    gui->addSlider("CAMERA X ORBIT", 0, 360, 0.0);
    gui->addSlider("CAMERA Y ORBIT", -90, 90, 0.0);
    
    gui->addSpacer();
    gui->addLabel("DOF");
    gui->addToggle("ENABLE DOF", &bDOFEnabled);
    gui->addToggle("DOF FOCUS ASSISTANCE", &bDrawDOFFocusAssist);
    gui->addSpacer();
    gui->addSlider("DOF FOCAL DISTANCE", 0, 6000, cameraDistance);
    gui->addSlider("DOF FOCAL RANGE", 0, 200, 50);
    gui->addSlider("DOF BLUR AMOUNT", 0, 3, 1);
    
    std::vector<std::string> names;
    names.push_back("TOP");
    names.push_back("BOTTOM");
    names.push_back("OPPOSITE");
    
    gui->addSpacer();
    gui->addLabel("MODELS");
    gui->addSlider("MODEL DISTANCE", 0, maxModelDistance, modelDistance);
    gui->addIntSlider("MODELS Y", -150.0, 150.0, 0.0);
    gui->addRadio("MODEL DESTRUCT MODE", names);
    
    gui->addSpacer();
    gui->addLabel("MATERIAL");
    gui->addSlider("MATERIAL SHINYNESS", 0, maxModelDistance, modelDistance);
    
    gui->addSpacer();
    gui->addLabel("PARTICLES");
    gui->addRangeSlider("SPEED", 0.0, 7, 1.5, 2.5);
    gui->addRangeSlider("ROTATION SPEED", 0.0, 3.0, 0.5, 1.5);
    
    gui->addSpacer();
    gui->addLabel("BOUNDING BOX");
    gui->addToggle("DRAW BOX", &bShowBoundingBox);
    gui->addSpacer();
    gui->addSlider("BOX WIDTH", 1.0, maxModelDistance, modelDistance - 200.0);
    gui->addSlider("BOX HEIGHT", 1.0, 1000.0, 300);
    gui->addSlider("BOX DEPTH", 1.0, 4000.0, 200.0);
    gui->addSpacer();
    gui->addSlider("BOX X", - modelDistance/2, modelDistance/2, 0.0);
    gui->addSlider("BOX Y", - 800, 800, 100.0);
    gui->addSlider("BOX Z", - modelDistance/2, modelDistance/2, -155.0);
    gui->addSpacer();
    gui->addButton("CENTER BOX", false);
    
    ofImage& mD1Image = mD1.getImage();
    float imageWidth = (gui->getGlobalCanvasWidth() - gui->getPadding()*7.0);
    float scale = mD1Image.getWidth() / imageWidth;
    gui->addSpacer();
    gui->addLabel("KINECT 1");
    gui->addToggle("KINECT 1 ENABLED", mD1.isEnabled());
    gui->addToggle("KINECT 1 LIVE VIDEO", &bMD1UseLiveVideo);
    gui->addImage("KINECT 1 IMAGE", &mD1Image, imageWidth, mD1Image.getHeight()/scale);
    gui->addSlider("KINECT 1 FRAME DIFF", 0.0, 1.0, mD1.getFrameDifference()); // read only
    gui->addLabelButton("KINECT 1 MOTION DETECTED", false); // read only
    gui->addSpacer();
    gui->addSlider("KINECT 1 THRESHOLD", 0.0, 0.1, mD1.getThreshold());
    gui->addIntSlider("KINECT 1 INTERVAL", 0, 3000, mD1.getInterval());
    gui->addIntSlider("KINECT 1 NEAR CLIP", 0, 255, mD1.getNearClip());
    gui->addIntSlider("KINECT 1 FAR CLIP", 0, 255, mD1.getNearClip());
    gui->addIntSlider("KINECT 1 TILT", -30, 30, mD1.getTiltAngle());
    
    ofImage& mD2Image = mD2.getImage();
    scale = mD2Image.getWidth() / imageWidth;
    gui->addSpacer();
    gui->addLabel("KINECT 2");
    gui->addToggle("KINECT 2 ENABLED", mD2.isEnabled());
    gui->addToggle("KINECT 2 LIVE VIDEO", &bMD2UseLiveVideo);
    gui->addImage("KINECT 2 IMAGE", &mD2Image, imageWidth, mD2Image.getHeight()/scale);
    gui->addSlider("KINECT 2 FRAME DIFF", 0.0, 1.0, mD2.getFrameDifference()); // read only
    gui->addLabelButton("KINECT 2 MOTION DETECTED", false); // read only
    gui->addSpacer();
    gui->addSlider("KINECT 2 THRESHOLD", 0.0, 0.1, mD2.getThreshold());
    gui->addIntSlider("KINECT 2 INTERVAL", 0, 3000, mD2.getInterval());
    gui->addIntSlider("KINECT 2 NEAR CLIP", 0, 255, mD2.getNearClip());
    gui->addIntSlider("KINECT 2 FAR CLIP", 0, 255, mD2.getNearClip());
    gui->addIntSlider("KINECT 2 TILT", -30, 30, mD2.getTiltAngle());
    
    gui->autoSizeToFitWidgets();
    gui->setPosition((client.getMWidth() - client.getXoffset()) - gui->getRect()->getWidth(), 0);
    gui->setup();
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
    
    cameraXOrbit = ((ofxUISlider *) gui->getWidget("CAMERA X ORBIT"))->getScaledValue();
    cameraYOrbit = ((ofxUISlider *) gui->getWidget("CAMERA Y ORBIT"))->getScaledValue();
    cameraDistance = ((ofxUISlider *) gui->getWidget("CAMERA DISTANCE"))->getScaledValue();
    
    // camera
    camera.disableMouseInput();
    camera.setDistance(cameraDistance);
    camera.orbit(cameraXOrbit, cameraYOrbit, camera.getPosition().distance(ofVec3f(0, 0, 0)));
    camera.lookAt(ofVec3f(0, 0, 0));
    
    // dof
    depthOfField.setup();
    
    // misc
    viewport = ofRectangle(-client.getXoffset(), -client.getYoffset(), client.getMWidth(), client.getLHeight());
    isPaused = false;
    bShowBoundingBox = false;
    bBoundingBoxChanged = false;
    bAllFacesDislodged1 = false;
    bAllFacesDislodged2 = false;
    bFacesWaiting = true;
    modelY = 0;

    initMeshFaces();
    
    // these have to be retriggered after initMeshFaces();
    gui->getWidget("SPEED")->triggerSelf();
    gui->getWidget("ROTATION SPEED")->triggerSelf();
    
}

//--------------------------------------------------------------
void ofApp::update(){
    // leave this empty...
}

//--------------------------------------------------------------
void ofApp::draw(){
    // leave this empty...
}

void ofApp::customUpdate() {
    
    // update Kinect GUI stuff
    ofxUIButton* mD1MotionDetectedbutton = (ofxUIButton* ) gui->getWidget("KINECT 1 MOTION DETECTED");
    mD1MotionDetectedbutton->setValue(false);
    
    mD1.update();
    ofxUIImage* image1Widget = (ofxUIImage*) gui->getWidget("KINECT 1 LIVE VIDEO");
    image1Widget->setImage(&mD1.getImage());
    ofxUISlider* frameDifferenceSlider1 = (ofxUISlider*) gui->getWidget("KINECT 1 FRAME DIFF");
    frameDifferenceSlider1->setValue(mD1.getFrameDifference());
    
    ofxUIButton* mD2MotionDetectedbutton = (ofxUIButton* ) gui->getWidget("KINECT 2 MOTION DETECTED");
    mD2MotionDetectedbutton->setValue(false);
    
    mD2.update();
    ofxUIImage* image2Widget = (ofxUIImage*) gui->getWidget("KINECT 2 LIVE VIDEO");
    image2Widget->setImage(&mD2.getImage());
    ofxUISlider* frameDifferenceSlider2 = (ofxUISlider*) gui->getWidget("KINECT 2 FRAME DIFF");
    frameDifferenceSlider2->setValue(mD2.getFrameDifference());
    
    if (!isPaused) {
        
        int vertCounter = 0;
        float deltaTime = ofGetElapsedTimef() - lastFrameTime;
        for (int i = 0; i < model1Faces.size(); i++) {
            
            std::vector<ofVec3f>& model1MeshVerts = model1Mesh.getVertices();
            std::vector<ofVec3f>& model1MeshNorms = model1Mesh.getNormals();
            model1Faces[i].update(model1MeshVerts[vertCounter],
                                  model1MeshVerts[vertCounter + 1],
                                  model1MeshVerts[vertCounter + 2],
                                  model1MeshNorms[vertCounter],
                                  model1MeshNorms[vertCounter + 1],
                                  model1MeshNorms[vertCounter + 2],
                                  deltaTime);
            
            std::vector<ofVec3f>& model2MeshVerts = model2Mesh.getVertices();
            std::vector<ofVec3f>& model2MeshNorms = model2Mesh.getNormals();
            model2Faces[i].update(model2MeshVerts[vertCounter],
                                  model2MeshVerts[vertCounter + 1],
                                  model2MeshVerts[vertCounter + 2],
                                  model2MeshNorms[vertCounter],
                                  model2MeshNorms[vertCounter + 1],
                                  model2MeshNorms[vertCounter + 2],
                                  deltaTime);
            
            vertCounter += 3;
        }
        
        // if not all faces had been detached
        if (bAllFacesDislodged1 && bAllFacesDislodged2) {
            
            if (bFacesWaiting) {
                
                for (int i = 0; i < model1Faces.size(); i++) {
                    model1Faces[i].settle();
                    model2Faces[i].settle();
                }
                
                bFacesWaiting = false;
            }
            
            if(allFacesSettled()){
                
                bAllFacesDislodged1 = false;
                bAllFacesDislodged2 = false;
                
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
                
            }
            
        } else {
            
            if (CLIENT_ID == 1) {
                
                if (mD1.motionDetected()) {
                    client.broadcast("mD1MotionDetected");
                    
                }
                
                if (mD2.motionDetected()) {
                    client.broadcast("mD2MotionDetected");
                }
            }
            
            bFacesWaiting = true;
            
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

void ofApp::customDraw() {
    
//    fbo.begin();
    ofClear(0, 255);
    
    ofEnableDepthTest();
    ofSetColor(255);
    
    if (bDOFEnabled) {
        depthOfField.begin();
    }
    
    ofEnableLighting();
    camera.begin(viewport);
    light.enable();
    material.begin();
    ofPushStyle();
    
//    ofSetColor(255, 0, 0);
    model1Mesh.draw();
//    ofSetColor(0, 0, 255);
    model2Mesh.draw();
    
    ofPopStyle();
    material.end();
    light.disable();
    camera.end();
    
    ofDisableLighting();
    
    if (bDOFEnabled) {
        depthOfField.end();
        if (bDrawDOFFocusAssist) depthOfField.drawFocusAssist(0, 0);
        else depthOfField.getFbo().draw(0, 0);
    }
    
    if (bShowBoundingBox) {
        camera.begin(viewport);
        boundingBox.drawWireframe();
        camera.end();
    }
    
    if (gui->isVisible()) {
        std::string message = ofToString(client.getFPS()) += " fps\n";
        message += "Hold SHIFT to remove faces\n";
        message += "Press SPACE to pause\n";
        message += "Press 'r' to reset\n";
        message += "Press 'f' to toggle fullscreen\n";
        message += "Press 'h' to toggle GUI\n";
        ofSetColor(255);
        ofDrawBitmapString(message,  15 - client.getXoffset(), 15);
    }
    
//    fbo.end();
}

void ofApp::mpeFrameEvent(ofxMPEEventArgs& event) {
    
    if (event.frame == 1) {
        lastFrameTime = ofGetElapsedTimef();
    }
    
    ofBackground(0);
    customUpdate();
    customDraw();
    lastFrameTime = ofGetElapsedTimef();
    
//    fbo.getTextureReference().drawSubsection(0,
//                                             0,
//                                             client.getLWidth(),
//                                             client.getLHeight(),
//                                             client.getXoffset(),
//                                             client.getYoffset());
//    
}

void ofApp::mpeMessageEvent(ofxMPEEventArgs& event) {
    
    if (event.message == "mD1MotionDetected") {
        dislodge(mD1, model1Faces, model2Faces, bDestroyTop1, true);
    }
    else if (event.message == "mD2MotionDetected") {
        dislodge(mD2, model2Faces, model1Faces, bDestroyTop2, false);
    }
}

void ofApp::mpeResetEvent(ofxMPEEventArgs& event) {
    
}

void ofApp::exit() {
    
    gui->saveSettings("settings.xml");
    delete gui;
}

bool ofApp::dislodge(MotionDetector& mD,
                     std::vector<ModelFace>& modelFaces,
                     std::vector<ModelFace>& otherModelFaces,
                     bool bRemoveFromTop,
                     bool bModel1) {
    
    if ((bModel1 && bAllFacesDislodged1) || (!bModel1 && bAllFacesDislodged2)) return false;
    else {
        
        int faceIndex = -1;
        bool usingModel1Face = true; // not the first mesh necessarily, just the first one passed into this function
        
        // find an index that hasn't been dislodged to start with
        for (int i = 0; i < modelFaces.size(); i++) {
        
            if ((bModel1 && (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x >= 0)) ||
                (!bModel1 && (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x <= 0))) {
                
                faceIndex = i;
                break;
            }
            
            if ((bModel1 && (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x >= 0)) ||
                (!bModel1 && (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x <= 0))) {
                
                faceIndex = i;
                usingModel1Face = false;
                break;
            }
            
        }
    
        // if one was found...
        if (faceIndex != -1) {
            
            // initial highest y can be from either mesh
            float y = usingModel1Face ? modelFaces[faceIndex].getPosition().y : otherModelFaces[faceIndex].getPosition().y;
            
            for (int i = 0; i < modelFaces.size(); i++) {
            
                if ((bModel1 && (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x >= 0)) ||
                    (!bModel1 && (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x <= 0))) {
                    float curY = bRemoveFromTop ? max(modelFaces[i].getPosition().y, y) : min(modelFaces[i].getPosition().y, y);
                    if ((bRemoveFromTop && curY > y) ||
                        (!bRemoveFromTop && curY < y)) {
                        y = curY;
                        faceIndex = i;
                        usingModel1Face = true;
                    }
                }
                
                if ((bModel1 && (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x > 0)) ||
                    (!bModel1 && (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x < 0))) {
                    float curY = bRemoveFromTop ? max(otherModelFaces[i].getPosition().y, y) : min(otherModelFaces[i].getPosition().y, y);
                    if ((bRemoveFromTop && curY > y) ||
                        (!bRemoveFromTop && curY < y)) {
                        y = curY;
                        faceIndex = i;
                        usingModel1Face = false;
                    }
                }
                    
            }
            
            ModelFace& face = usingModel1Face ? modelFaces[faceIndex] : otherModelFaces[faceIndex];
            face.dislodge();
            
            return true;
        
        } else {
            if (bModel1) bAllFacesDislodged1 = true;
            else bAllFacesDislodged2 = true;
            return false;
        }
    }
    
    return false;
}

bool ofApp::allFacesSettled() {
    
    for (int i = 0; i < model1Faces.size(); i++) {
        if (model1Faces[i].isDislodged() || model2Faces[i].isDislodged()) return false;
    }
    
    return true;
}

void ofApp::initMeshFaces() {
    
    // this must go in here for some reason
    ofSeedRandom(RANDOM_SEED);
    
    ofMesh mesh1 = model.getMesh(0);
    ofMesh mesh2 = mesh1;
    
    std::vector<ofVec3f>& mesh1Vertices = mesh1.getVertices();
    std::vector<ofVec3f>& mesh2Vertices = mesh2.getVertices();
    
    std::vector<ofVec3f>& mesh1Normals = mesh1.getNormals();
    std::vector<ofVec3f>& mesh2Normals = mesh2.getNormals();
    
    ofVec3f centroid = mesh1.getCentroid();
    
    float xOffset = modelDistance/2;
    
    for (int i = 0; i < mesh1Vertices.size(); i++) {
        
        mesh1Normals[i].rotate(90, ofVec3f(0, 1, 0));
        mesh1Vertices[i].rotate(90, centroid, ofVec3f(0, 1, 0));
        mesh1Vertices[i].x += xOffset;
        mesh1Vertices[i].y += modelY;
        
        mesh2Normals[i].rotate(-90, ofVec3f(0, 1, 0));
        mesh2Vertices[i].rotate(-90, centroid, ofVec3f(0, 1, 0));
        mesh2Vertices[i].x -= xOffset;
        mesh2Vertices[i].y += modelY;
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
        std::vector<ofVec3f>& model1FaceNorms = model1Faces[i].getNormals();
        model1Mesh.addVertex(model1FaceVerts[0]);
        model1Mesh.addNormal(model1FaceNorms[0]);
        model1Mesh.addVertex(model1FaceVerts[1]);
        model1Mesh.addNormal(model1FaceNorms[1]);
        model1Mesh.addVertex(model1FaceVerts[2]);
        model1Mesh.addNormal(model1FaceNorms[2]);
        
        std::vector<ofVec3f>& model2FaceVerts = model2Faces[i].getVertices();
        std::vector<ofVec3f>& model2FaceNorms = model2Faces[i].getNormals();
        model2Mesh.addVertex(model2FaceVerts[0]);
        model2Mesh.addNormal(model2FaceNorms[0]);
        model2Mesh.addVertex(model2FaceVerts[1]);
        model2Mesh.addNormal(model2FaceNorms[1]);
        model2Mesh.addVertex(model2FaceVerts[2]);
        model2Mesh.addNormal(model2FaceNorms[2]);
    }

}

void ofApp::resetCamera() {
    
    camera.setFov(startCameraFOV);
    camera.setAspectRatio(startCameraAspectRatio);
    camera.setNearClip(startCameraNearClip);
    camera.setFarClip(startCameraFarClip);
    
    ofxUISlider* fov = (ofxUISlider*) gui->getWidget("CAMERA FOV");
    fov->setValue(startCameraFOV);
    
    ofxUISlider* aspect = (ofxUISlider*) gui->getWidget("CAMERA ASPECT RATIO");
    if (aspect != NULL) aspect->setValue(startCameraAspectRatio);
    
    ofxUISlider* near = (ofxUISlider*) gui->getWidget("CAMERA NEAR CLIP");
    near->setValue(startCameraNearClip);
    
    ofxUISlider* far = (ofxUISlider*) gui->getWidget("CAMERA FAR CLIP");
    far->setValue(startCameraFarClip);
}

ofVec3f ofApp::getPointInBoundingBox() {
    
    ofVec3f bBoxPosition = boundingBox.getPosition();
    float bBoxWidth = boundingBox.getWidth();
    float bBoxHeight = boundingBox.getHeight();
    float bBoxDepth = boundingBox.getDepth();
    ofVec3f point(ofRandom(bBoxPosition.x - bBoxWidth/2, bBoxPosition.x + bBoxWidth/2),
                  ofRandom(bBoxPosition.y - bBoxHeight/2, bBoxPosition.y + bBoxHeight/2),
                  ofRandom(bBoxPosition.z - bBoxDepth/2, bBoxPosition.z + bBoxDepth/2));
    cout << point << endl;
    return point;
}

void ofApp::guiEvent(ofxUIEventArgs &e) {
    
    if (e.getName() == "RESET") {
        initMeshFaces();
    }
    
    // MODEL POSITIONS
    if (e.getName() == "MODEL DISTANCE") {
        
        modelDistance = (int) e.getSlider()->getValue();
        ofxUISlider* boxWidthSlider = (ofxUISlider *) gui->getWidget("BOX WIDTH");
        boxWidthSlider->setValue(modelDistance);
        boundingBox.setWidth(modelDistance - 200);
        initMeshFaces();
    }
    
    if (e.getName() == "MODELS Y") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider *) e.getSlider();
        modelY = slider->getValue();
        initMeshFaces();
    }
    
    if (e.getName() == "MODEL DESTRUCT MODE") {
        ofxUIRadio* radio = (ofxUIRadio*) e.widget;
        destructMode = (int) radio->getValue();
        
        cout << "destruct mode: " << destructMode << endl;
        switch (destructMode) {
                
            case MODEL_DESTRUCT_TOP:
                bDestroyTop1 = true;
                bDestroyTop2 = true;
                break;
                
            case MODEL_DESTRUCT_BOTTOM:
                bDestroyTop1 = false;
                bDestroyTop2 = false;
                break;
                
            case MODEL_DESTRUCT_OPPOSITE:
                bDestroyTop1 = true;
                bDestroyTop2 = false;
                break;
        }
    }
    
    // CAMERA
    if (e.getName() == "RESET CAMERA") {
        resetCamera();
    }
    
    if (e.getName() == "RESET ASPECT TO WINDOW") {
        
        camera.setAspectRatio(float(client.getMWidth())/float(client.getMHeight()));
        ofxUISlider * slider = (ofxUISlider *) gui->getWidget("CAMERA ASPECT RATIO");
        slider->setValue(camera.getAspectRatio());
        startCameraAspectRatio = camera.getAspectRatio();
    }
    
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
    
    if (e.getName() == "CAMERA DISTANCE") {
        
        cameraDistance = e.getSlider()->getScaledValue();
        
        ofVec3f dir = camera.getPosition() - ofVec3f(0, 0, 0);
        dir.normalize();
        dir *= cameraDistance;
        camera.setPosition(dir);
    }
    
    if (e.getName() == "CAMERA X ORBIT") {
        
        cameraXOrbit = e.getSlider()->getScaledValue();
        camera.orbit(cameraXOrbit, cameraYOrbit, camera.getPosition().distance(ofVec3f(0, 0, 0)));
        camera.lookAt(ofVec3f(0, 0, 0));
    }
    
    if (e.getName() == "CAMERA Y ORBIT") {
        
        cameraYOrbit = e.getSlider()->getScaledValue();
        camera.orbit(cameraXOrbit, cameraYOrbit, camera.getPosition().distance(ofVec3f(0, 0, 0)));
        camera.lookAt(ofVec3f(0, 0, 0));
    }
    
    
    // DOF
    if (e.getName() == "DOF FOCAL DISTANCE") {
        depthOfField.setFocalDistance(e.getSlider()->getScaledValue());
    }
    
    if (e.getName() == "DOF FOCAL RANGE") {
        depthOfField.setFocalRange(e.getSlider()->getScaledValue());
    }
    
    if (e.getName() == "DOF BLUR AMOUNT") {
        depthOfField.setBlurAmount(e.getSlider()->getScaledValue());
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
    
    // MOTION DETECTORS
    
    if (e.getName() == "KINECT 1 ENABLED") {
        mD1.setEnabled(e.getButton()->getValue());
    }
    
    if (e.getName() == "KINECT 1 THRESHOLD") {
        mD1.setDifferenceThreshold(e.getSlider()->getScaledValue());
    }
    
    if (e.getName() == "KINECT 1 INTERVAL") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD1.setInterval(slider->getScaledValue());
    }
    
    if (e.getName() == "KINECT 1 NEAR CLIP") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD1.setNearClip(slider->getScaledValue());
    }
    
    if (e.getName() == "KINECT 1 FAR CLIP") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD1.setFarClip(slider->getScaledValue());
    }
    
    if (e.getName() == "KINECT 1 TILT") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD1.setTiltAngle(slider->getScaledValue());
    }
    
    if (e.getName() == "KINECT 2 ENABLED") {
        mD2.setEnabled(e.getButton()->getValue());
    }
    
    if (e.getName() == "KINECT 2 THRESHOLD") {
        mD2.setDifferenceThreshold(e.getSlider()->getScaledValue());
    }
    
    if (e.getName() == "KINECT 2 INTERVAL") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD2.setInterval(slider->getScaledValue());
    }
    
    if (e.getName() == "KINECT 2 NEAR CLIP") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD2.setNearClip(slider->getScaledValue());
    }
    
    if (e.getName() == "KINECT 2 FAR CLIP") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD2.setFarClip(slider->getScaledValue());
    }
    
    if (e.getName() == "KINECT 2 TILT") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider*) e.getSlider();
        mD2.setTiltAngle(slider->getScaledValue());
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
    } else if (key == 'x') {
        for (int i = 0; i < model1Faces.size(); i++) {
            
            model1Faces[i].dislodge();
            model2Faces[i].settle();
            
            model2Faces[i].dislodge();
            model1Faces[i].settle();
        }
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
    
    depthOfField.setup(w, h);
    if (gui != NULL) {
        gui->setPosition(w - gui->getRect()->getWidth(), 0);
        gui->autoSizeToFitWidgets();
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
