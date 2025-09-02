//
// Created by zhang on 23-9-15.
//

#include "RbotTracking.h"
#include <typeinfo>
RbotTracking::RbotTracking(){
    std::cout << "RBOT Tracker";
};

RbotTracking::~RbotTracking() {

    RenderingEngine::Instance()->doneCurrent();
    // clean up
    RenderingEngine::Instance()->destroy();

    // for(int i = 0; i < objects.size(); i++)
    // {
    //     delete objects[i];
    // }
    // objects.clear();
    for(int i = 0; i < this->realObjects.size(); i++)
    {
        delete this->realObjects[i];
    }
    this->realObjects.clear();
        for(int i = 0; i < this->virtualObjects.size(); i++)
    {
        delete this->virtualObjects[i];
    }
    this->virtualObjects.clear();

    delete poseEstimator;
};


void RbotTracking::Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) 
{
    // vfr = appData.getData("mode");
    auto camera = sceneData.getMainCamera();
	width = camera.width;
	height = camera.height;
    // width = frameDataPtr->image[0].cols;
    // height = frameDataPtr->image[0].rows;

    // near and far plane of the OpenGL view frustum
    zNear = 10.0;
    zFar = 10000.0;

    timeout =1;

    showHelp = true;

    K=appData.params.intrinsicMatrices[0];
    distCoeffs=appData.params.distCoeffs[0];

    shader_path=appData.params.shaderPath;

    std::vector<RealObject*> scene_realObjects = sceneData.getAllObjectsOfType<RealObject>();
    // std::vector<VirtualObject*> scene_virtualObjects = sceneData.getAllObjectsOfType<VirtualObject>();
    std::cout<< "[ObjectTracking] If core dumped happened -> check .obj file path exist or not" <<std::endl;
    for(auto realptr : scene_realObjects){
        std::cout<< "loading real model:" << realptr->filePath <<std::endl;
        auto T = realptr->transform.GetMatrix();
        T(0, 3) *= 1000.0;
        T(1, 3) *= 1000.0;
        T(2, 3) *= 1000.0;
        this->realObjects.push_back(new Object3D(realptr->filePath,  T, 1.0, 0.55f, distances));
    }
    if(static_cast<int>(this->realObjects.size())>1){
        std::cout << "Tracking Module Alert! realObject number > 1 !!!";
    }

    // for(auto virtualptr : scene_virtualObjects){
    //     std::cout<< "loading virtual model:" << virtualptr->filePath <<std::endl;
    //     auto T = virtualptr->transform.GetMatrix();
    //     T(0, 3) *= 1000.0;
    //     T(1, 3) *= 1000.0;
    //     T(2, 3) *= 1000.0;
    //     this->virtualObjects.push_back(new Object3D(virtualptr->filePath, T, 1.0, 0.55f, distances));
    // }

    
    // for(auto realptr : scene_realObjects){
    //     std::cout<< "loading real model:" << realptr->filePath <<std::endl;
    //     this->realObjects.push_back(new Object3D(realptr->filePath, realptr->transform.GetMatrix(), 1.0, 0.55f, distances));
    // }
    // if(static_cast<int>(this->realObjects.size())>1){
    //     std::cout << "Tracking Module Alert! realObject number > 1 !!!";
    // }

    // for(auto virtualptr : scene_virtualObjects){
    //     std::cout<< "loading virtual model:" << virtualptr->filePath <<std::endl;
    //     this->virtualObjects.push_back(new Object3D(virtualptr->filePath, virtualptr->transform.GetMatrix(), 1.0, 0.55f, distances));
    // }
    std::cout << "loading objects" << std::endl;
    this->objects.insert(this->objects.end(), this->realObjects.begin(), this->realObjects.end());
    // this->objects.insert(this->objects.end(), this->virtualObjects.begin(), this->virtualObjects.end());

    renderingEngine= RenderingEngine::Instance();
    // create the pose estimator
    zNear = 1.0f;
    zFar = 3000.0f;
    poseEstimator = new PoseEstimator6D(width, height, zNear, zFar, K, distCoeffs, this->objects, shader_path);
    RenderingEngine::Instance()->makeCurrent();
}

void RbotTracking::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) 
{

    // cv::Mat frame = frameDataPtr->image[0];
    cv::Mat frame = frameDataPtr->imgColor;


    poseEstimator->estimatePoses(frame, false, true, vfr);

    // only update real object pose
    std::vector<RealObject*> scene_realObjects = sceneData.getAllObjectsOfType<RealObject>();
    for(int i = 0; i < static_cast<int>(this->realObjects.size()); ++i){
        auto tracking_real_ptr = this->realObjects[i];
        auto T = tracking_real_ptr->getPose();
        T(0, 3) /= 1000.0;
        T(1, 3) /= 1000.0;
        T(2, 3) /= 1000.0;
        scene_realObjects[i]->transform.setPose(T);
        std::cout << "name:" << scene_realObjects[i]->name << std::endl;
        std::cout << "objTracking2 pose:" << T << std::endl;
    }
    
    // just for test
    if(vfr)
    {
        // for(auto &obj : objects)
        // {
        //     int id = obj->getModelID();
        //     // load transforms in this 
        //     cv::Matx44f transform = cv::Matx44f::eye();
        //     cv::Matx44f pose = objects[0]->getPose();
        //     obj->setPose(pose * transform);
        // }
        for(auto &obj : this->objects)
        {
            int id = obj->getModelID();
            // load transforms in this 
            cv::Matx44f transform = cv::Matx44f::eye();
            cv::Matx44f pose = objects[0]->getPose();
            obj->setPose(pose * transform);
            std::cout << "objTracking update pose: " << pose * transform << std::endl;
        }
    } 

    // turn on/off objectTracking render
    // render the models with the resulting pose estimates ontop of the input image
    bool TurnOffRender = false;
    if(!TurnOffRender){
        cv::Mat result = DrawResultOverlay(frame);

        if(showHelp)
        {
            putText(result, "Press '1' to initialize", cv::Point(150, 250), cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
            putText(result, "or 'c' to quit", cv::Point(205, 285), cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        }

        imshow("ObjectTracking", result);
    }else{
        imshow("ObjectTracking", frame);
    }


    int key=0;
    key = cv::waitKey(timeout);

    // start/stop tracking the first object
    if(key == (int)'1')
    {
        poseEstimator->toggleTracking(frame, 0, false);
        poseEstimator->estimatePoses(frame, false, false, vfr);
        timeout = 1;
        showHelp = !showHelp;
    }
    if(key == (int)'2') // the same for a second object
    {
        //poseEstimator->toggleTracking(frame, 1, false);
        //poseEstimator->estimatePoses(frame, false, false);
    }
    // reset the systekeym to the initial state
    if(key == (int)'r')
        poseEstimator->reset();
    // stop the demo
}

void RbotTracking::PreCompute(std::string configPath) {
    return;
}

cv::Mat RbotTracking::DrawResultOverlay(const cv::Mat& frame)
{
    // render the models with phong shading
    RenderingEngine::Instance()->setLevel(0);

    std::vector<cv::Point3f> colors;
    colors.push_back(cv::Point3f(1.0, 0.5, 0.0));
    //colors.push_back(Point3f(0.2, 0.3, 1.0));
    // if(vfr) RenderingEngine::Instance()->renderShaded(std::vector<Model*>(this->virtualObjects.begin(), this->virtualObjects.end()), GL_FILL, colors, true);
    // else RenderingEngine::Instance()->renderShaded(std::vector<Model*>(this->virtualObjects.begin(), this->virtualObjects.begin()+1), GL_FILL, colors, true);
    
    // choose which model be rendered
    // //only virtual
    // RenderingEngine::Instance()->renderShaded(std::vector<Model*>(this->objects.begin()+1, this->objects.end()), GL_FILL, colors, true);
    // only real 
    RenderingEngine::Instance()->renderShaded(std::vector<Model*>(this->objects.begin(), this->objects.begin()+1), GL_FILL, colors, true);

    // download the rendering to the CPU
    cv::Mat rendering = RenderingEngine::Instance()->downloadFrame(RenderingEngine::RGB);

    // download the depth buffer to the CPU
    cv::Mat depth = RenderingEngine::Instance()->downloadFrame(RenderingEngine::DEPTH);

    // compose the rendering with the current camera image for demo purposes (can be done more efficiently directly in OpenGL)
    cv::Mat result = frame.clone();
    for(int y = 0; y < frame.rows; y++)
    {
        for(int x = 0; x < frame.cols; x++)
        {
            cv::Vec3b color = rendering.at<cv::Vec3b>(y,x);
            if(depth.at<float>(y,x) != 0.0f)
            {
                result.at<cv::Vec3b>(y,x)[0] = color[2];
                result.at<cv::Vec3b>(y,x)[1] = color[1];
                result.at<cv::Vec3b>(y,x)[2] = color[0];
            }
        }
    }
    return result;
}

