
#include"ObjectTracking2/include/ObjectTracking2.h"

#include"./base.h"
#include"BFC/argv.h"
#include"CVX/vis.h"
#include"CVRender/cvrm.h"
#include"CVRender/cvrender.h"
#include<fstream>
using namespace re3d;
using namespace cv;


class ObjectTracking2Impl
	:public ObjectTracking2
{
public:
    
public:

    //int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;

    int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override
    {
        Result res;
        if (!frameDataPtr->image.empty() && _tracker)
        {
            cv::Mat img = frameDataPtr->image.front();
            if(img.channels()!=3)
                cvtColor(img,img,cv::COLOR_GRAY2BGR);

            _fd.cameraK = frameDataPtr->colorCameraMatrix;
            _fd.distCoeffs = frameDataPtr->colorDistCoeffs;

            _tracker->pro(img, _fd);

            Result::ModelPose mp;
            for (auto& r : _fd.objs)
            {
                if (r.score < 0.5f || uint(r.modelIndex)>_modelInfos.size())
                    continue;

                auto &pose = r.pose.get<std::vector<RigidPose>>().front();
                mp.modelIndex = r.modelIndex;
                mp.modelName = _modelInfos[mp.modelIndex].name;
                mp.R = pose.R;
                mp.t = pose.t;
                mp.contourProjected = cvtPoint(r.contourProjected);
                res.objPoses.push_back(mp);
            }
            res.camK = _fd.cameraK;
            res.modelSet = _modelSet;
        }
        sceneData.setData("ObjectTracking2Result", res);

        return STATE_OK;
    }

    //int ShutDown(AppData& appData, SceneData& sceneData) override;

    //int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override;

    //int ProRemoteReturn(RemoteProcPtr proc) override;

    int ProCommand(int cmd, std::any& data) override
    {
        switch (cmd)
        {
        case CMD_SET_MODEL:
            this->_setModel(std::any_cast<std::string>(data));
            break;
        }
        return STATE_OK;
    }
private:
    re3d::FrameData   _fd;
    std::vector<ModelInfos>    _modelInfos;
    std::shared_ptr<ModelSet>  _modelSet;
    std::shared_ptr<FrameProc> _tracker;


    void _setModel(const std::string& modelFile)
    {
        std::shared_ptr<ModelSet> modelSet(new ModelSet);

        std::vector<std::string> modelFiles = { modelFile };
        std::vector<ModelInfos> modelInfos = modelInfosFromFiles(modelFiles, "cadar");
        modelSet->set(modelInfos);

        auto tracker = FrameProc::create("v1.Tracker");
        ff::CommandArgSet args;
        args.setArgs("-globalSearch - -usePointMatches + -trackScale 1.0");

        tracker->init(modelSet.get(), &args);
        
        _modelInfos.swap(modelInfos);
        _tracker = tracker;
        _modelSet = modelSet;
    }
};

void ObjectTracking2::Result::showResult(cv::Mat& dimg)
{
    for (auto &pose : this->objPoses)
    {
        CVRMats mats;
        mats.mModel = cvrm::fromR33T(pose.R, pose.t);
        mats.mProjection = cvrm::fromK(camK, dimg.size(), 0.1, 3000);

        auto modelPtr = modelSet->getModel(pose.modelIndex);
        if (modelPtr)
        {
            CVRModel& m3d = modelPtr->get3DModel();
            auto center=m3d.getCenter();
            CVRender render(m3d);
            //auto rr = render.exec(mats, img.size(), CVRM_IMAGE | CVRM_DEPTH, CVRM_DEFAULT, nullptr, r.roi);
            auto rr = render.exec(mats, dimg.size(), CVRM_IMAGE | CVRM_DEPTH, CVRM_DEFAULT, nullptr);
            //cv::imshow("rr", rr.img);
            Mat1b mask = rr.getMaskFromDepth();
            Rect roi = cv::get_mask_roi(DWHS(mask), 127);

            if (roi.empty())
                continue;

            //if (_drawBlend)
            {
                Mat t;
                cv::addWeighted(dimg(roi), 0.5, rr.img(roi), 0.5, 0, t);
                t.copyTo(dimg(roi), mask(roi));
            }
            //if (_drawContour)
            {
                std::vector<std::vector<Point> > cont;
                cv::findContours(mask, cont, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
                drawContours(dimg, cont, -1, Scalar(255, 0, 0), 2, CV_AA);
            }
        }
    }
}



std::shared_ptr<ObjectTracking2> ObjectTracking2::create()
{
    return std::shared_ptr<ObjectTracking2>(new ObjectTracking2Impl);
}
