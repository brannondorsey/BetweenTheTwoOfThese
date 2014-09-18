#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofSetWindowShape(1200, 500);
    ofEnableAntiAliasing();
    // ofSetWindowShape(768 * 2, 72 * 2); // real aspect ratio
    // ofSetWindowShape(7680 * 2, 720 * 2); // real aspect ratio
    ofBackground(0);
    
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
    modelDistance = ofGetWidth() * 1.7;
    
    // camera
    camera.setAspectRatio(float(ofGetWidth())/float(ofGetHeight()));
    camera.setForceAspectRatio(true);
    
    startCameraFOV = camera.getFov();
    startCameraAspectRatio = camera.getAspectRatio();
    startCameraNearClip = camera.getNearClip();
    startCameraFarClip = camera.getFarClip();
    
    // motion detectors (Kinects)
    mD1.setup();
    mD1.setUseLiveVideo(false);
    bMD1UseLiveVideo = true;
    
    // gui
    gui = new ofxUIScrollableCanvas();
    gui->setScrollAreaToScreen();
    gui->setScrollableDirections(false, true);

    gui->setColorBack(ofColor(50, 200));

    int maxModelDistance = 10000;
    
    gui->addSpacer();
    gui->addButton("RESET", false);
    
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
    gui->addSlider("CAMERA DISTANCE", 100, 4000, 1800);
    gui->addSlider("CAMERA X ORBIT", 0, 360, 0.0);
    gui->addSlider("CAMERA Y ORBIT", -90, 90, 0.0);
    
    gui->addSpacer();
    gui->addLabel("DOF");
    gui->addToggle("ENABLE DOF", &bDOFEnabled);
    gui->addToggle("DOF FOCUS ASSISTANCE", &bDrawDOFFocusAssist);
    gui->addSpacer();
    gui->addSlider("DOF FOCAL DISTANCE", 0, 10000, cameraDistance);
    gui->addSlider("DOF FOCAL RANGE", 0, 2000, 50);
    gui->addSlider("DOF BLUR AMOUNT", 0, 3, 1);
    
    gui->addSpacer();
    gui->addLabel("MODEL");
    gui->addSlider("MODEL DISTANCE", 0, maxModelDistance, modelDistance);
    gui->addIntSlider("MODEL Y", -100.0, 100.0, 0.0);
    
    gui->addSpacer();
    gui->addLabel("MATERIAL");
    gui->addSlider("MATERIAL SHINYNESS", 0, maxModelDistance, modelDistance);
    
    gui->addSpacer();
    gui->addLabel("PARTICLES");
    gui->addRangeSlider("SPEED", 0.2, 7, 1.5, 2.5);
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
    gui->addSlider("BOX Y", - modelDistance/2, modelDistance/2, 100.0);
    gui->addSlider("BOX Z", - modelDistance/2, modelDistance/2, -155.0);
    gui->addSpacer();
    gui->addButton("CENTER BOX", false);
    
    gui->autoSizeToFitWidgets();
    gui->setPosition(ofGetWidth() - gui->getRect()->getWidth(), 0);
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
    isPaused = false;
    bShowBoundingBox = false;
    bBoundingBoxChanged = false;
    bModel1FacesReturning = false;
    modelY = 0;

    initMeshFaces();
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    ofxUIButton* mD1MotionDetectedbutton = (ofxUIButton* ) gui->getWidget("KINECT 1 MOTION DETECTED");
    mD1MotionDetectedbutton->setValue(false);
    
    mD1.update();
    ofxUIImage* imageWidget = (ofxUIImage*) gui->getWidget("KINECT 1 LIVE VIDEO");
    imageWidget->setImage(&mD1.getImage());
    ofxUISlider* frameDifferenceSlider = (ofxUISlider*) gui->getWidget("KINECT 1 FRAME DIFF");
    frameDifferenceSlider->setValue(mD1.getFrameDifference());
    
    if (!isPaused) {
        
        int vertCounter = 0;
        for (int i = 0; i < model1Faces.size(); i++) {
            
            std::vector<ofVec3f>& model1MeshVerts = model1Mesh.getVertices();
            std::vector<ofVec3f>& model1MeshNorms = model1Mesh.getNormals();
            model1Faces[i].update(model1MeshVerts[vertCounter],
                                  model1MeshVerts[vertCounter + 1],
                                  model1MeshVerts[vertCounter + 2],
                                  model1MeshNorms[vertCounter],
                                  model1MeshNorms[vertCounter + 1],
                                  model1MeshNorms[vertCounter + 2]);
            
            std::vector<ofVec3f>& model2MeshVerts = model2Mesh.getVertices();
            std::vector<ofVec3f>& model2MeshNorms = model2Mesh.getNormals();
            model2Faces[i].update(model2MeshVerts[vertCounter],
                                  model2MeshVerts[vertCounter + 1],
                                  model2MeshVerts[vertCounter + 2],
                                  model2MeshNorms[vertCounter],
                                  model2MeshNorms[vertCounter + 1],
                                  model2MeshNorms[vertCounter + 2]);
            
            vertCounter += 3;
        }
        
//        bool result = dislodge(mD1, model1Faces, model2Faces, true);
//        mD1MotionDetectedbutton->setValue(result);
        
        
        if (ofGetKeyPressed('x')) dislodge(mD1, model1Faces, model2Faces, false);
        else dislodge(mD1, model2Faces, model1Faces, true);
        
        
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
           
            int randomIndex = (int) ofRandom(0, model1Faces.size());
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
    ofSetColor(255);
    
    if (bDOFEnabled) {
        depthOfField.begin();
    }
    
    ofEnableLighting();
    
    camera.begin(depthOfField.getDimensions());
    light.enable();
    material.begin();
    ofPushStyle();
    
    ofSetColor(255);
    model1Mesh.draw();
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
        camera.begin();
        boundingBox.drawWireframe();
        camera.end();
    }
    
    if (gui->isVisible()) {
        
        std::string message = ofToString(ofGetFrameRate()) += " fps\n";
        message += "Hold SHIFT to remove faces\n";
        message += "Press SPACE to pause\n";
        message += "Press 'r' to reset\n";
        message += "Press 'f' to toggle fullscreen\n";
        message += "Press 'h' to toggle GUI\n";
        ofSetColor(255);
        ofDrawBitmapString(message,  15, 15);
    }
}

bool ofApp::dislodge(MotionDetector& mD,
                     std::vector<ModelFace>& modelFaces,
                     std::vector<ModelFace>& otherModelFaces,
                     bool bModel1) {
    
//    if (true) {
    if (mD.motionDetected()) {
        
        // find an index that hasn't been dislodged
        // to start with
        int faceIndex = -1;
        bool usingModel1Face = true;
        
        for (int i = 0; i < modelFaces.size(); i++) {
            
            if (bModel1) {
                
                if (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x > 0) {
                    
                    faceIndex = i;
                    break;
                }
                
                if (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x > 0) {
                    
                    faceIndex = i;
                    usingModel1Face = false;
                    break;
                }

            } else {
                
                if (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x < 0) {
                    
                    faceIndex = i;
                    break;
                }
                
                if (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x < 0) {
                    
                    faceIndex = i;
                    usingModel1Face = false;
                    break;
                }
                
            }
            
        }
        
        // loop through all
    
        if (faceIndex != -1) {
            
            // initial highest y can be from either mesh
            float highestY = usingModel1Face ? model1Faces[faceIndex].getPosition().y : model2Faces[faceIndex].getPosition().y;
            
            for (int i = 0; i < modelFaces.size(); i++) {
                
                if (bModel1) {
                    
                    if (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x > 0) {
                        int maxY = max(modelFaces[i].getPosition().y, highestY);
                        if (maxY > highestY) {
                            highestY = maxY;
                            faceIndex = i;
                            usingModel1Face = true;
                        }
                    }
                    
                    if (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x > 0) {
                        int maxY = max(otherModelFaces[i].getPosition().y, highestY);
                        if (maxY > highestY) {
                            highestY = maxY;
                            faceIndex = i;
                            usingModel1Face = false;
                        }
                    }
                    
                } else {
                 
                    if (!modelFaces[i].isDislodged() && modelFaces[i].getPosition().x < 0) {
                        int maxY = max(modelFaces[i].getPosition().y, highestY);
                        if (maxY > highestY) {
                            highestY = maxY;
                            faceIndex = i;
                            usingModel1Face = true;
                        }
                    }
                    
                    if (!otherModelFaces[i].isDislodged() && otherModelFaces[i].getPosition().x < 0) {
                        int maxY = max(otherModelFaces[i].getPosition().y, highestY);
                        if (maxY > highestY) {
                            highestY = maxY;
                            faceIndex = i;
                            usingModel1Face = false;
                        }
                    }
                }
                
//            float highestY = usingModel1Face ? model1Faces[faceIndex].getPosition().y : model2Faces[faceIndex].getPosition().y;
//            
//            for (int i = 0; i < model1Faces.size(); i++) {
//                
//                ModelFace& modelFace = usingModel1Face ? model1Faces[i] : model2Faces[i];
//                ModelFace& otherModelFace = usingModel1Face ? model2Faces[i] : model1Faces[i];
//                
//                if (usingModel1Face && !modelFace.isDislodged()) {
//                    
//                    float lastHighestY = highestY;
//                    highestY = max(modelFace.getPosition().y, highestY);
//
//                    if (highestY != lastHighestY) {
//                        usingModel1Face = true;
//                        faceIndex = i;
//                    }
//                }
//                
//                if (!usingModel1Face && !otherModelFace.isDislodged()) {
//                    
//                    float lastHighestY = highestY;
//                    highestY = max(otherModelFace.getPosition().y, highestY);
//                    
//                    if (highestY != lastHighestY) {
//                        usingModel1Face = false;
//                        faceIndex = i;
//                    }
//                }
            }
            
            ModelFace& face = usingModel1Face ? modelFaces[faceIndex] : otherModelFaces[faceIndex];
            ModelFace& correspondingFace = usingModel1Face ? otherModelFaces[faceIndex] : modelFaces[faceIndex];
        
            face.dislodge();
            correspondingFace.onPartnerDislodged();
            face.setWaiting(!correspondingFace.isDislodged());
        }
        
        return true;
    }
    
    return false;
}

void ofApp::initMeshFaces() {
    
    ofMesh mesh1 = model.getMesh(0);
    ofMesh mesh2 = mesh1;
    
    std::vector<ofVec3f>& mesh1Vertices = mesh1.getVertices();
    std::vector<ofVec3f>& mesh2Vertices = mesh2.getVertices();
    
    std::vector<ofVec3f>& mesh1Normals = mesh1.getNormals();
    std::vector<ofVec3f>& mesh2Normals = mesh2.getNormals();
    
    ofVec3f centroid = mesh1.getCentroid();
    
    float xOffset = modelDistance/2;
    
    meshFaces1MinY = mesh1Vertices[0].y;
    meshFaces1MaxY = mesh1Vertices[0].y;
    
    for (int i = 0; i < mesh1Vertices.size(); i++) {
        
        mesh1Normals[i].rotate(90, ofVec3f(0, 1, 0));
        mesh1Vertices[i].rotate(90, centroid, ofVec3f(0, 1, 0));
        mesh1Vertices[i].x += xOffset;
        mesh1Vertices[i].y += modelY;
        
        meshFaces1MinY = min(mesh1Vertices[i].y, meshFaces1MinY);
        meshFaces1MaxY = max(mesh1Vertices[i].y, meshFaces1MaxY);
        
        mesh2Normals[i].rotate(-90, ofVec3f(0, 1, 0));
        mesh2Vertices[i].rotate(-90, centroid, ofVec3f(0, 1, 0));
        mesh2Vertices[i].x -= xOffset;
        mesh2Vertices[i].y += modelY;
    }

//    cout << "The lowest mesh 1 Vertex is: " << meshFaces1MinY << endl;
//    cout << "The highest mesh 1 Vertex is: " << meshFaces1MaxY << endl;
//    cout << endl;
    
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
    
    return ofVec3f(ofRandom(bBoxPosition.x - bBoxWidth/2, bBoxPosition.x + bBoxWidth/2),
                   ofRandom(bBoxPosition.y - bBoxHeight/2, bBoxPosition.y + bBoxHeight/2),
                   ofRandom(bBoxPosition.z - bBoxDepth/2, bBoxPosition.z + bBoxDepth/2));
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
    
    if (e.getName() == "MODEL Y") {
        
        ofxUIIntSlider* slider = (ofxUIIntSlider *) e.getSlider();
        modelY = slider->getValue();
        initMeshFaces();
    }
    
    // CAMERA
    if (e.getName() == "RESET CAMERA") {
        resetCamera();
    }
    
    if (e.getName() == "RESET ASPECT TO WINDOW") {
        
        camera.setAspectRatio(float(ofGetWidth())/float(ofGetHeight()));
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
