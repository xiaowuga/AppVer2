#ifndef ARTEST_COLLISIONDETECTION_H
#define ARTEST_COLLISIONDETECTION_H

//#include "Basic/include/ARModule.h"
//#include "Basic/include/BasicData.h"
//#include "Basic/include/ConfigLoader.h"
//#include "Basic/include/App.h"
#include "ARModule.h"
#include "BasicData.h"
#include "ConfigLoader.h"
#include "App.h"

#include "communication/dataInterface.h"
#include "ExampleCollisionHandler.h"
#include "InteractionConfigLoader.h"

class CollisionDetection : public ARModule {
public:
    CollisionDetection();
    ~CollisionDetection() override;
    void PreCompute(std::string configPath) override;
    int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameData) override;
    int Update(AppData &appData, SceneData& sceneData, FrameDataPtr frameData) override;
    int ShutDown(AppData& appData,  SceneData& sceneData) override;
    int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override;
    int ProRemoteReturn(RemoteProcPtr proc) override;
    void AddCollisionHandler(CollisionHandler* handler);
    void RemoveCollisionHandler(CollisionHandler* handler);
private:
    std::unordered_map<std::string,CollisionData::Ptr> _collisionDatas;
    bool _isColliding(std::shared_ptr<CollisionData> obj1, std::shared_ptr<CollisionData> obj2, CollisionType type);
    bool _AABB2AABB(std::shared_ptr<CollisionData> obj1, std::shared_ptr<CollisionData> obj2);
    bool _Ray2AABB(std::shared_ptr<CollisionData> obj1, std::shared_ptr<CollisionData> obj2);
    bool _Sphere2AABB(std::shared_ptr<CollisionData> obj1, std::shared_ptr<CollisionData> obj2);

};


#endif //ARTEST_COLLISIONDETECTION_H
