#include "GesturePredictor.h"

GesturePredictor::GesturePredictor() {
}

GesturePredictor::~GesturePredictor() {
}

Gesture GesturePredictor::predict(HandPose& handPose) {
    int thumb = 0;
    int index = 0;
    int middle = 0;
    int ring = 0;
    int little = 0;
    std::array<cv::Vec3f, HandPose::jointNum> joints = handPose.getjoints();
    double d1 = cv::norm(joints[(int)HandJoint::THUMB_IP] - joints[(int)HandJoint::INDEX_FINGER_MCP]);
    double d2 = cv::norm(joints[(int)HandJoint::THUMB_IP] - joints[(int)HandJoint::THUMB_MCP]);
    cv::Vec3f v10 = joints[(int)HandJoint::WRIST] - joints[(int)HandJoint::THUMB_CMC];
    cv::Vec3f v12 = joints[(int)HandJoint::THUMB_MCP] - joints[(int)HandJoint::THUMB_CMC];
    cv::Vec3f v21 = joints[(int)HandJoint::THUMB_CMC] - joints[(int)HandJoint::THUMB_MCP];
    cv::Vec3f v23 = joints[(int)HandJoint::THUMB_IP] - joints[(int)HandJoint::THUMB_MCP];
    cv::Vec3f v32 = joints[(int)HandJoint::THUMB_MCP] - joints[(int)HandJoint::THUMB_IP];
    cv::Vec3f v34 = joints[(int)HandJoint::THUMB_TIP] - joints[(int)HandJoint::THUMB_IP];
    cv::Vec3f v14 = joints[(int)HandJoint::THUMB_TIP] - joints[(int)HandJoint::THUMB_CMC];
    cv::Vec3f v15 = joints[(int)HandJoint::INDEX_FINGER_MCP] - joints[(int)HandJoint::THUMB_CMC];
    double angle0 = cv::fastAtan2(cv::norm(v10.cross(v12)), v10.dot(v12));
    double angle1 = cv::fastAtan2(cv::norm(v21.cross(v23)), v21.dot(v23));
    double angle2 = cv::fastAtan2(cv::norm(v32.cross(v34)), v32.dot(v34));
    if(angle0 + angle1 + angle2 > 460 && cv::fastAtan2(cv::norm(v14.cross(v15)), v14.dot(v15)) > 30) {
        thumb = 1;
    } else {
        thumb = 0;
    }
    d1 = cv::norm(joints[(int)HandJoint::INDEX_FINGER_TIP] - joints[(int)HandJoint::INDEX_FINGER_MCP]);
    d2 = cv::norm(joints[(int)HandJoint::INDEX_FINGER_PIP] - joints[(int)HandJoint::INDEX_FINGER_MCP]);

    cv::Vec3f v50 = joints[(int)HandJoint::WRIST] - joints[(int)HandJoint::INDEX_FINGER_MCP];
    cv::Vec3f v56 = joints[(int)HandJoint::INDEX_FINGER_PIP] - joints[(int)HandJoint::INDEX_FINGER_MCP];
    cv::Vec3f v65 = joints[(int)HandJoint::INDEX_FINGER_MCP] - joints[(int)HandJoint::INDEX_FINGER_PIP];
    cv::Vec3f v67 = joints[(int)HandJoint::INDEX_FINGER_DIP] - joints[(int)HandJoint::INDEX_FINGER_PIP];
    cv::Vec3f v76 = joints[(int)HandJoint::INDEX_FINGER_PIP] - joints[(int)HandJoint::INDEX_FINGER_DIP];
    cv::Vec3f v78 = joints[(int)HandJoint::INDEX_FINGER_TIP] - joints[(int)HandJoint::INDEX_FINGER_DIP];
    angle0 = cv::fastAtan2(cv::norm(v50.cross(v56)), v50.dot(v56));
    angle1 = cv::fastAtan2(cv::norm(v65.cross(v67)), v65.dot(v67));
    angle2 = cv::fastAtan2(cv::norm(v76.cross(v78)), v76.dot(v78));
    if(angle0 + angle1 + angle2 > 465) {
        index = 1;
    } else if(d1 / d2 < 3.0) {
        index = 0;
    } else {
        index = -1;
    }
    d1 = cv::norm(joints[(int)HandJoint::MIDDLE_FINGER_TIP] - joints[(int)HandJoint::MIDDLE_FINGER_MCP]);
    d2 = cv::norm(joints[(int)HandJoint::MIDDLE_FINGER_PIP] - joints[(int)HandJoint::MIDDLE_FINGER_MCP]);
    cv::Vec3f v90 = joints[(int)HandJoint::WRIST] - joints[(int)HandJoint::MIDDLE_FINGER_MCP];
    cv::Vec3f v910 = joints[(int)HandJoint::MIDDLE_FINGER_PIP] - joints[(int)HandJoint::MIDDLE_FINGER_MCP];
    cv::Vec3f v109 = joints[(int)HandJoint::MIDDLE_FINGER_MCP] - joints[(int)HandJoint::MIDDLE_FINGER_PIP];
    cv::Vec3f v1011 = joints[(int)HandJoint::MIDDLE_FINGER_DIP] - joints[(int)HandJoint::MIDDLE_FINGER_PIP];
    cv::Vec3f v1110 = joints[(int)HandJoint::MIDDLE_FINGER_PIP] - joints[(int)HandJoint::MIDDLE_FINGER_DIP];
    cv::Vec3f v1112 = joints[(int)HandJoint::MIDDLE_FINGER_TIP] - joints[(int)HandJoint::MIDDLE_FINGER_DIP];
    angle0 = cv::fastAtan2(cv::norm(v90.cross(v910)), v90.dot(v910));
    angle1 = cv::fastAtan2(cv::norm(v109.cross(v1011)), v109.dot(v1011));
    angle2 = cv::fastAtan2(cv::norm(v1110.cross(v1112)), v1110.dot(v1112));
    if(angle0 + angle1 + angle2 > 460) {
        middle = 1;
    } else if(d1 / d2 < 4.2) {
        middle = 0;
    } else {
        middle = -1;
    }
    d1 = cv::norm(joints[(int)HandJoint::RING_FINGER_TIP] - joints[(int)HandJoint::RING_FINGER_MCP]);
    d2 = cv::norm(joints[(int)HandJoint::RING_FINGER_PIP] - joints[(int)HandJoint::RING_FINGER_MCP]);
    cv::Vec3f v130 = joints[(int)HandJoint::WRIST] - joints[(int)HandJoint::RING_FINGER_MCP];
    cv::Vec3f v1314 = joints[(int)HandJoint::RING_FINGER_PIP] - joints[(int)HandJoint::RING_FINGER_MCP];
    cv::Vec3f v1413 = joints[(int)HandJoint::RING_FINGER_MCP] - joints[(int)HandJoint::RING_FINGER_PIP];
    cv::Vec3f v1415 = joints[(int)HandJoint::RING_FINGER_DIP] - joints[(int)HandJoint::RING_FINGER_PIP];
    cv::Vec3f v1514 = joints[(int)HandJoint::RING_FINGER_PIP] - joints[(int)HandJoint::RING_FINGER_DIP];
    cv::Vec3f v1516 = joints[(int)HandJoint::RING_FINGER_TIP] - joints[(int)HandJoint::RING_FINGER_DIP];
    angle0 = cv::fastAtan2(cv::norm(v130.cross(v1314)), v130.dot(v1314));
    angle1 = cv::fastAtan2(cv::norm(v1413.cross(v1415)), v1413.dot(v1415));
    angle2 = cv::fastAtan2(cv::norm(v1514.cross(v1516)), v1514.dot(v1516));
    if(angle0 + angle1 + angle2 > 460) {
        ring = 1;
    } else if(d1 / d2 < 4.2) {
        ring = 0;
    } else {
        ring = -1;
    }
    d1 = cv::norm(joints[(int)HandJoint::PINKY_TIP] - joints[(int)HandJoint::PINKY_MCP]);
    d2 = cv::norm(joints[(int)HandJoint::PINKY_PIP] - joints[(int)HandJoint::PINKY_MCP]);
    cv::Vec3f v170 = joints[(int)HandJoint::WRIST] - joints[(int)HandJoint::PINKY_MCP];
    cv::Vec3f v1718 = joints[(int)HandJoint::PINKY_PIP] - joints[(int)HandJoint::PINKY_MCP];
    cv::Vec3f v1817 = joints[(int)HandJoint::PINKY_MCP] - joints[(int)HandJoint::PINKY_PIP];
    cv::Vec3f v1819 = joints[(int)HandJoint::PINKY_DIP] - joints[(int)HandJoint::PINKY_PIP];
    cv::Vec3f v1918 = joints[(int)HandJoint::PINKY_PIP] - joints[(int)HandJoint::PINKY_DIP];
    cv::Vec3f v1920 = joints[(int)HandJoint::PINKY_TIP] - joints[(int)HandJoint::PINKY_DIP];
    angle0 = cv::fastAtan2(cv::norm(v170.cross(v1718)), v170.dot(v1718));
    angle1 = cv::fastAtan2(cv::norm(v1817.cross(v1819)), v1817.dot(v1819));
    angle2 = cv::fastAtan2(cv::norm(v1918.cross(v1920)), v1918.dot(v1920));
    if(angle0 + angle1 + angle2 > 465) {
        little = 1;
    } else if(d1 / d2 < 4.2) {
        little = 0;
    } else {
        little = -1;
    }
    if(thumb==1&&index==1&&middle==0&&ring==0&&little==0){
        if(cv::fastAtan2(cv::norm(v14.cross(v15)), v14.dot(v15))>10){
            return Gesture::ZOOM;
        }
        return Gesture::CURSOR;
    }
    if(thumb==0&&index==1&&middle==0&&ring==0&&little==0){
        return Gesture::CURSOR;
    }
    if(thumb==0&&index==0&&middle==0&&ring==0&&little==0){
        return Gesture::GRASP;
    }
    return Gesture::UNDEFINED;
}