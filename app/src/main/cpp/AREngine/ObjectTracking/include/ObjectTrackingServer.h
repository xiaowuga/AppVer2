#pragma once

#include"Basic/include/RPCServer.h"
#include"Basic/include/RPC.h"

class ObjectTrackingServer
    :public ARModuleServer
{
public:
    virtual ARModuleServerPtr create() {
        return std::make_shared<ObjectTrackingServer>();
    }

    virtual int init(RPCServerConnection& con) override;

    struct DetectedObj
    {
        std::string objName;

        cv::Rect     roi;
        cv::Matx34f  pose;

        DEFINE_BFS_IO_3(DetectedObj, objName, roi, pose);
    };

    virtual int call(RemoteProcPtr proc, FrameDataPtr frameDataPtr, RPCServerConnection& con) override;
};

