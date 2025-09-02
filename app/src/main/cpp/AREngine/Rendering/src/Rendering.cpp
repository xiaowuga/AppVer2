#include "Rendering.h"

Rendering::Rendering() = default;

Rendering::~Rendering() = default;

int Rendering::Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr){
    std::cout << "Rendering init" << std::endl;
    // width = frameData.image.size();
    // height = frameData.image.size();
    // std::cout << "width" << "height" << std::endl;
    // std::cout << width << height << std::endl;
    auto camera = sceneData.getMainCamera();
	this->width = camera.width;
	this->height = camera.height;

    std::cout << "width: " << width << " height:" << height;
	auto K = camera.IntrinsicMatrix;
    std::cout << "K" << K;
    std::cout << "render-K:" << camera.IntrinsicMatrix << std::endl;
    renderer.setWidthAndHeight(width, height, upsample_scale);
    renderer.setKParameters(K(0, 0), K(1, 1), K(0, 2), K(1, 2));

    std::cout << "1" << std::endl;
    renderingDir = appData.engineDir + "Rendering/";
    std::cout << "2" << std::endl;
    std::vector<VirtualObject*> scene_virtualObjects = sceneData.getAllObjectsOfType<VirtualObject>();
    for(int i = 0; i < scene_virtualObjects.size(); i ++){
        model_paths.push_back(scene_virtualObjects[i]->filePath);
        model_transforms.push_back(CvMatrixToVsg(scene_virtualObjects[i]->transform.GetMatrix() * scene_virtualObjects[i]->initTransform.GetMatrix()));
        instance_names.push_back(scene_virtualObjects[i]->name);
    }
    std::cout << "3" << std::endl;
    renderer.setUpShader(renderingDir, objtracking_shader);
    // vsg::dmat4 plane_transform = CvMatrixToVsg(scene_virtualObjects[0]->transform.GetMatrix() * scene_virtualObjects[0]->initTransform.GetMatrix());
    vsg::dmat4 plane_transform = vsg::translate(0.0, 1.5, 0.0) * vsg::rotate(90.0, 1.0, 0.0, 0.0) * vsg::translate(0.0, 0.0, 1.0);
    renderer.initRenderer(renderingDir, model_transforms, model_paths, instance_names, plane_transform);
    
    if(trackingViewMatrix){
        vsg::dmat4 left_hand_view = CvMatrixToVsg(camera.transform.GetMatrix() * camera.initTransform.GetMatrix());
        vsg::dmat4 left_to_right = vsg::dmat4(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
        vsg::dmat4 right_hand_view = left_to_right * left_hand_view;
        renderer.updateCamera(right_hand_view);
    }

    std::cout << "Rendering Init successed" << std::endl;
    return 0;
}

int Rendering::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr){
    // std::cout << "Rendering Update Start" << std::endl;
    if(trackingViewMatrix){
        auto camera = sceneData.getMainCamera();
        auto T = camera.transform.GetMatrix();
        auto T_t = camera.transform.GetMatrix();
     
        // for(int i = 0; i < 3; i++){
        //     for(int j = 0; j < 3; j++){
        //         T_t(i, j) = T(j, i);
        //     }
        // }
        // T_t(0, 0) = -T(0, 0); T_t(0, 1) = -T(0, 1); T_t(0, 2) = T(0, 2);
        // T_t(1, 0) = -T(1, 0); T_t(1, 1) = -T(1, 1); T_t(1, 2) = T(1, 2);
        // T_t(2, 0) = T(2, 0); T_t(2, 1) = T(2, 1); T_t(2, 2) = T(2, 2);
        // auto x = T(0, 3);
        // auto y = T(1, 3);
        // auto z = T(2, 3);
        // T_t(0, 3) = -x;
        // T_t(1, 3) = z;
        // T_t(2, 3) = -y;
        vsg::dmat4 left_hand_view = CvMatrixToVsg(T_t * camera.initTransform.GetMatrix());
        std::cout << "Rendering Camera: " << camera.transform.GetMatrix() * camera.initTransform.GetMatrix() << std::endl;
        // vsg::dmat4 left_hand_view = CvMatrixToVsg(camera.transform.GetMatrix());
        // std::cout << "camera.transform:" << camera.transform.GetMatrix() << std::endl;

        vsg::dmat4 left_to_right = vsg::dmat4(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
        vsg::dmat4 right_hand_view = left_to_right * left_hand_view;
        // std::cout << "render view";
        // for(int i = 0; i < 4; i ++)
        //     for(int j = 0; j < 4; j++)
        //         std::cout << right_hand_view[j][i];
        renderer.updateCamera(right_hand_view);
    }
    // else{
    //     vsg::dvec3 centre = {lookAtVector[0], lookAtVector[1], lookAtVector[2]};                    // 固定观察点
    //     vsg::dvec3 eye = {lookAtVector[3], lookAtVector[4], lookAtVector[5]};// 固定相机位置
    //     vsg::dvec3 up = {lookAtVector[6], lookAtVector[7], lookAtVector[8]};                       // 固定观察方向
    //     renderer.updateCamera(centre, eye, up);
    // }
    std::vector<VirtualObject*> scene_virtualObjects = sceneData.getAllObjectsOfType<VirtualObject>();
    for(int i = 0; i < scene_virtualObjects.size(); i ++){
        renderer.updateObjectPose(scene_virtualObjects[i]->name, CvMatrixToVsg(scene_virtualObjects[i]->transform.GetMatrix() * scene_virtualObjects[i]->initTransform.GetMatrix()));
        std::cout << "Rendering "<< i <<" pose: " << scene_virtualObjects[i]->transform.GetMatrix() << std::endl;
        std::cout << "Rendering "<< i <<" normalization: " << scene_virtualObjects[i]->initTransform.GetMatrix() << std::endl;
        std::cout << "Rendering "<< i <<" pose*normalization: " << scene_virtualObjects[i]->transform.GetMatrix() * scene_virtualObjects[i]->initTransform.GetMatrix() << std::endl;
    }
    // std::cout << "ObjectTracking modelViewMatrix: " << modelViewMatrix << std::endl;
    // std::string colorPath = "/home/lab/workspace/wgy/cadvsg_intg/asset/data/slamData/color/1711699313.948925.png";
    // std::string depthPath = "/home/lab/workspace/wgy/cadvsg_intg/asset/data/slamData/depth/1711699313.948925.png";
    // std::ifstream color_file(colorPath, std::ios::binary | std::ios::app);
    // std::vector<uint8_t> color_buffer((std::istreambuf_iterator<char>(color_file)), std::istreambuf_iterator<char>());
    // std::ifstream depth_file(depthPath, std::ios::binary | std::ios::app);
    // std::vector<uint8_t> depth_buffer((std::istreambuf_iterator<char>(depth_file)), std::istreambuf_iterator<char>());

    // std::string real_color1(color_buffer.begin(), color_buffer.end());
    // std::string real_depth1(depth_buffer.begin(), depth_buffer.end());

    // const std::string& real_color = real_color1;
    // const std::string& real_depth = real_depth1;
    if(! use_png){
        unsigned short* depth_data = reinterpret_cast<unsigned short*>(frameDataPtr->imgDepth.data);
        cv::Mat BGR;
        frameDataPtr->convertRGB2BGR(BGR);

        renderer.setRealColorAndImage(BGR.data, depth_data);
    }
    // else{
    //     renderer.setRealColorAndImage(real_color, real_depth);
    // }
    
    uint8_t* color_image;
    renderer.render(color_image);
    // std::cout << "Rendering Update successed" << std::endl;
    return 0;
}

void Rendering::PreCompute(std::string configPath) {
    return;
}

int Rendering::ShutDown(AppData& appData, SceneData& sceneData){
    // 实现关闭逻辑
    std::cout << "Rendering module shutting down." << std::endl;
    return STATE_OK;
}

vsg::dmat4 Rendering::CvMatrixToVsg(cv::Matx44f cv_matrix){
    vsg::dmat4 vsg_matrix = vsg::dmat4();
    for(int i = 0; i < 4; i ++)
        for(int j = 0; j < 4; j ++)
            vsg_matrix[i][j] = cv_matrix(j ,i);
    return vsg_matrix;
}
