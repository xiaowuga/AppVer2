#pragma once

#include "Basic/include/ARModule.h"

namespace re3d {
    class ModelSet;
}

class ObjectTracking2 
    : public ARModule 
{
public:

    enum
    {
        CMD_SET_MODEL = MODULE_CMD_BEG + 1
    };
    struct Result
    {
        struct ModelPose
        {
            int  modelIndex =-1;
            std::string modelName;

            cv::Matx33f R;
            cv::Vec3f   t;

            std::vector<cv::Point>  contourProjected;
        };

        cv::Matx33f              camK;
        std::vector<ModelPose>   objPoses;
        std::shared_ptr<re3d::ModelSet> modelSet;
    public:
        void  showResult(cv::Mat& dimg);
    };

    static std::shared_ptr<ObjectTracking2> create();
};


