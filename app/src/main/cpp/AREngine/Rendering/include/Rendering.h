#ifndef ARENGINE_RENDERING_H
#define ARENGINE_RENDERING_H

#include "Basic/include/ARModule.h"
#include "vsgRenderer.h"

namespace MFW {
    class Renderer;
};

class Rendering : public ARModule {
private:
    vsgRenderer renderer;
    int width = 640;
    int height = 480;
    bool use_png = false;
    bool trackingViewMatrix = true;
    std::string* real_color1;
    std::string* real_depth1;
    std::vector<double> lookAtVector = {0.000588, 0.739846, -0.903124, 0.087284, 1.51642, -0.279094, 0.163545, -0.628984, 0.760021};
    std::vector<vsg::dmat4> model_transforms;
    std::vector<vsg::dmat4> init_model_transforms;
    std::vector<std::string> instance_names;
    std::vector<std::string> model_paths;
    std::string renderingDir;
    bool objtracking_shader = true;
    double upsample_scale = 2;

public:

    Rendering();

    ~Rendering();

    void PreCompute(std::string configPath) override;

    int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;

    int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;

    int ShutDown(AppData& appData, SceneData& sceneData) override;

    vsg::dmat4 CvMatrixToVsg(cv::Matx44f cv_matrix);
};

#endif //ARENGINE_RENDERING_H