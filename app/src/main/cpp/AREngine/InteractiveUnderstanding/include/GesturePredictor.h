#ifndef ARTEST_GESTUREPREDICTOR_H
#define ARTEST_GESTUREPREDICTOR_H


#include "BasicData.h"
#include "HandJoint.h"
class GesturePredictor {
public:
    GesturePredictor();
    ~GesturePredictor();
    static Gesture predict(HandPose& handPose);
};


#endif //ARTEST_GESTUREPREDICTOR_H
