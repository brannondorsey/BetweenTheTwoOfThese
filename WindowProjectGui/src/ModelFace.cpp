//
//  ModelFace.cpp
//  ModelFaceTest
//
//  Created by bdorse on 9/3/14.
//
//

#include "ModelFace.h"

ModelFace::ModelFace(const ofMeshFace& face, int id):
_id(id),
_maxSpeed(0),
_rotationDir(ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1))),
_thresholdDistance(200),
_isDislodged(false),
_thresholdCrossed(false),
_isReturning(false),
_isWaiting(true)
{
    
    ofMesh mesh;
    mesh.enableNormals();
    mesh.addVertex(face.getVertex(0));
    mesh.addNormal(face.getNormal(0));
    mesh.addIndex(0);
    mesh.addVertex(face.getVertex(1));
    mesh.addNormal(face.getNormal(1));
    mesh.addIndex(1);
    mesh.addVertex(face.getVertex(2));
    mesh.addNormal(face.getNormal(2));
    mesh.addIndex(2);
    
    ofVec3f centroid = mesh.getCentroid();
    
    ofVec3f v1 = (mesh.getVertex(0) - centroid);
    ofVec3f v2 = (mesh.getVertex(1) - centroid);
    ofVec3f v3 = (mesh.getVertex(2) - centroid);
    
    mesh.setVertex(0, v1);
    mesh.setVertex(1, v2);
    mesh.setVertex(2, v3);
    
    getMesh() = mesh;
    
    ofNode::setPosition(centroid);
    _startPosition = ofNode::getPosition();
    _startRotation = ofNode::getOrientationEuler();
    
    _rotationDir.normalize();

    setRotationSpeed(0.1, 0.5);
    setSpeed(2, 3);
    setWaitPosition(ofVec3f(ofRandom(-500, 500),
                            ofRandom(-300, 300),
                            ofRandom(-2000, 2000)));
}

void ModelFace::update(ofVec3f& v1,
                       ofVec3f& v2,
                       ofVec3f& v3,
                       ofVec3f& v1N,
                       ofVec3f& v2N,
                       ofVec3f& v3N) {
    
    ofVec3f position;
    
    if (_isDislodged) {
        
        // movement
        position = ofNode::getPosition();
        ofVec3f desired = _currentTargetPosition - position;
        
        float d = desired.length();
        desired.normalize();
        
        float minSpeed = _isWaiting ? 0 : 2;
        float m = ofMap(d, 0, _thresholdDistance, minSpeed, _maxSpeed, true);
        desired *= m;
        
        // Steering = Desired minus Velocity
        ofVec3f steer = desired - _velocity;
        applyForce(steer);
        
        _velocity += _acceleration;
        _velocity.limit(_maxSpeed);
        _acceleration *= 0;
        
        position += _velocity;
        ofNode::setPosition(position);
        
        // rotation
        if (d < _thresholdDistance &&
            !_isWaiting) {
            
            if (!_thresholdCrossed) {
                _rotationAtThreshold = ofNode::getOrientationEuler();;
                _positionAtThreshold = ofNode::getPosition();
                _thresholdCrossed = true;
            }
            
            float rotX, rotY, rotZ;
            
            if (d < 2) {
                
                rotX = _currentTargetRotation.x;
                rotY = _currentTargetRotation.y;
                rotZ = _currentTargetRotation.z;
                ofNode::setPosition(_currentTargetPosition);
                _isDislodged = false;
                _thresholdCrossed = false;
                _isReturning = !_isReturning;
            
            } else {
                
                float distanceTraveled = position.distance(_positionAtThreshold);
                float percent = distanceTraveled/_thresholdDistance;
                
                rotX = ofLerpDegrees(_rotationAtThreshold.x, _currentTargetRotation.x, percent);
                rotY = ofLerpDegrees(_rotationAtThreshold.y, _currentTargetRotation.y, percent);
                rotZ = ofLerpDegrees(_rotationAtThreshold.z, _currentTargetRotation.z, percent);
            }
            
            ofQuaternion q(rotX, ofVec3f(1, 0, 0), rotY, ofVec3f(0, 1, 0), rotZ, ofVec3f(0, 0, 1));
            ofNode::setOrientation(q);

        } else {
            
            ofVec3f rotation = _rotationDir * _rotationSpeed;
            
            float rotX = ofNode::getPitch() + rotation.x;
            float rotY = ofNode::getHeading() + rotation.y;
            float rotZ = ofNode::getRoll() + rotation.z;
            ofQuaternion q(rotX,
                           ofVec3f(1, 0, 0),
                           rotY,
                           ofVec3f(0, 1, 0),
                           rotZ,
                           ofVec3f(0, 0, 1));
            
            ofNode::setOrientation(q);
            
        }
    }
    
    // update the actual vertex positions
    // using the references from the mesh
    position = ofNode::getPosition();
    
    float pitch = of3dPrimitive::getPitch();
    float heading = of3dPrimitive::getHeading();
    float roll = of3dPrimitive::getRoll();
    
    v1 = of3dPrimitive::getMesh().getVertex(0) + position;
    v2 = of3dPrimitive::getMesh().getVertex(1) + position;
    v3 = of3dPrimitive::getMesh().getVertex(2) + position;
    
    v1.rotate(pitch,   position, ofVec3f(1, 0, 0));
    v1.rotate(heading, position, ofVec3f(0, 1, 0));
    v1.rotate(roll,    position, ofVec3f(0, 0, 1));
    
    v2.rotate(pitch,   position, ofVec3f(1, 0, 0));
    v2.rotate(heading, position, ofVec3f(0, 1, 0));
    v2.rotate(roll,    position, ofVec3f(0, 0, 1));
    
    v3.rotate(pitch,   position, ofVec3f(1, 0, 0));
    v3.rotate(heading, position, ofVec3f(0, 1, 0));
    v3.rotate(roll,    position, ofVec3f(0, 0, 1));
    
    v1N = of3dPrimitive::getMesh().getNormal(0);
    v2N = of3dPrimitive::getMesh().getNormal(1);
    v3N = of3dPrimitive::getMesh().getNormal(2);
    
    v1N.rotate(pitch,   ofVec3f(1, 0, 0));
    v1N.rotate(heading, ofVec3f(0, 1, 0));
    v1N.rotate(roll,    ofVec3f(0, 0, 1));
    
    v2N.rotate(pitch,   ofVec3f(1, 0, 0));
    v2N.rotate(heading, ofVec3f(0, 1, 0));
    v2N.rotate(roll,    ofVec3f(0, 0, 1));
    
    v3N.rotate(pitch,   ofVec3f(1, 0, 0));
    v3N.rotate(heading, ofVec3f(0, 1, 0));
    v3N.rotate(roll,    ofVec3f(0, 0, 1));
}

void ModelFace::applyForce(const ofVec3f& force) {
    
    _acceleration += force;
}

void ModelFace::setTarget(const ofVec3f &targetPosition, const ofVec3f &targetRotation) {
    
    _targetPosition = targetPosition;
    _targetRotation = targetRotation;
    _currentTargetPosition = targetPosition;
    _currentTargetRotation = targetRotation;
}

void ModelFace::dislodge() {
    
    _isDislodged = true;
    _currentTargetPosition = _isReturning ? _startPosition : _targetPosition;
    _currentTargetRotation = _isReturning ? _startRotation : _targetRotation;
}

void ModelFace::onPartnerDislodged() {
    _isWaiting = false;
    _currentTargetPosition = _isReturning ? _startPosition : _targetPosition;
    _currentTargetRotation = _isReturning ? _startRotation : _targetRotation;
}

void ModelFace::setWaiting(bool wait) {
    _isWaiting = wait;
    if (_isWaiting) _currentTargetPosition = _waitPosition;
}

void ModelFace::setWaitPosition(const ofVec3f& position) {
    _waitPosition = position;
}

void ModelFace::setSpeed(float min, float max) {
    _maxSpeed = ofRandom(min, max);
}

void ModelFace::setSpeed(float speed) {
    _maxSpeed = speed;
}

void ModelFace::setRotationSpeed(float min, float max) {
    _rotationSpeed = ofRandom(min, max);
}

void ModelFace::setRotationSpeed(float speed) {
    _rotationSpeed = speed;
}

bool ModelFace::isDislodged() const {
    
    return _isDislodged;
}

ofVec3f ModelFace::getCentroid() const {
    
    return ofNode::getPosition();
}

std::vector<ofVec3f>& ModelFace::getVertices() {
    return of3dPrimitive::getMesh().getVertices();
}

std::vector<ofVec3f>& ModelFace::getNormals() {
    return of3dPrimitive::getMesh().getNormals();
}