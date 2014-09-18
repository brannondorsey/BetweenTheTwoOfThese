/*
This class contains the logic for manipulating a set of 3 vertices 
as if it were an of3dPrimitive (which extends from ofNode). 
It does NOT contain methods to draw the mesh. Rather, you pass in 3 vertice references
inside update(...), and then draw the mesh from the App.
*/

#ifndef __ModelFaceTest__ModelFace__
#define __ModelFaceTest__ModelFace__

#include "ofMain.h"
#include <iostream>

class ModelFace : public of3dPrimitive{
public:
    
    ModelFace(){};
    ModelFace(const ofMeshFace& face, int i);
    
    void update(ofVec3f& v1,
                ofVec3f& v2,
                ofVec3f& v3,
                ofVec3f& v1N,
                ofVec3f& v2N,
                ofVec3f& v3N);
    
    void applyForce(const ofVec3f& force);
    void setTarget(const ofVec3f& position, const ofVec3f& rotation);
    void dislodge();
    void onPartnerDislodged();
    void setWaiting(bool wait);
    void setWaitPosition(const ofVec3f& position);
    void setSpeed(float min, float max);
    void setSpeed(float speed);
    void setRotationSpeed(float min, float max);
    void setRotationSpeed(float speed);
    
    bool isDislodged() const;
    bool isReturning() const;
    
    ofVec3f getCentroid() const;
    std::vector<ofVec3f>& getVertices();
    std::vector<ofVec3f>& getNormals();
    
protected:
    
    int _id;
    int _thresholdDistance;
    
    float _maxSpeed;
    float _rotationSpeed;
    
    bool _isDislodged;
    bool _isWaiting;
    bool _thresholdCrossed;
    bool _isReturning;
    
    ofVec3f _rotationAtThreshold;
    ofVec3f _positionAtThreshold;
    ofVec3f _acceleration;
    ofVec3f _velocity;
    ofVec3f _rotationDir;
    ofVec3f _startPosition;
    ofVec3f _startRotation;
    ofVec3f _targetPosition;
    ofVec3f _targetRotation;
    ofVec3f _currentTargetRotation;
    ofVec3f _currentTargetPosition;
    ofVec3f _waitPosition;
    
};

#endif /* defined(__ModelFaceTest__ModelFace__) */
