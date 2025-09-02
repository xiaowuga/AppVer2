#include <vector>
#include "BasicData.h"
#include "ConfigLoader.h"

// 注册工厂类的宏
#define REGISTER_SCENE_OBJECT(type, class_name) \
    static bool class_name##Registered = []() { \
        SceneObjectFactory::instance().registerClass(type, []() { return std::make_unique<class_name>(); }); \
        return true; \
    }()
void registerSceneObjects() {
    REGISTER_SCENE_OBJECT("Camera", Camera);
    REGISTER_SCENE_OBJECT("SceneModel", SceneModel);
    REGISTER_SCENE_OBJECT("RealObject", RealObject);
    REGISTER_SCENE_OBJECT("VirtualObject", VirtualObject);
    REGISTER_SCENE_OBJECT("Anchor", Anchor);
    REGISTER_SCENE_OBJECT("Plane", Plane);
}

struct Link_pair{
    std::string realObjName;
    std::string virtualObjName;
    Link_pair(const std::string& rObjName, const std::string& vObjName): realObjName(rObjName), virtualObjName(vObjName) {}
};

class ObjManager : public ARModule {
    private:
        std::vector<Link_pair> objname_pairs;
    public:
        int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override {
            std::cout << "ObjManager Init ..." << std::endl;
            registerSceneObjects();
            // 加载配置并创建对象，保存到 SceneData
            ConfigLoader cfg(appData.sceneObjConfig);
            auto sceneObjectList = cfg.jsonData["obj_list"];
            for (auto& objParam : sceneObjectList) {
                std::string nameParam = objParam["name"];
                std::string typeParam = objParam["type"];
                std::string pathParam = objParam["path"];
                /*
                SceneObjectPtr obj(new VirtualObject);
                obj->filePath = appData.dataDir+"norm_model/squirrel_demo_low.obj";
                obj->initTransform.setPose(ModelInitPose);
                obj->transform.setPose(FirstFramePose);  // for test  [Object2World]-[Object2Camera]
                sceneData.setObject("virtualObj1", obj);
                */
                // 创建SceneObjectPtr对象并保存到SceneData
                SceneObjectPtr obj = SceneObjectFactory::instance().create(typeParam);
                if (obj) {
                    std::cout << "Create object: " << obj->name << ", type: " << typeParam << std::endl;
                    std::cout << "Path:" << pathParam << std::endl;
                    // obj->name = nameParam;
                    obj->filePath = appData.rootDir + pathParam;
                    obj->initTransform.setPose(cfg.get44Matrix(objParam, "initTransform"));
                    obj->transform.setPose(cfg.get44Matrix(objParam, "transform"));
                    obj->Init();
                    sceneData.setObject(nameParam, std::move(obj));
                }
            }
            return STATE_OK;
        }
        int Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) override {
            return STATE_OK;
        }

        int ShutDown(AppData& appData,  SceneData& sceneData) override {
            return STATE_OK;
        }

        /*搜集远程过程调用（RPC）所需的帧数据和命令
        * 对输入模块不会执行该操作，其它模块的该操作在帧数据输入完成之后（Update之前）执行。
        * 函数在主线程中执行，默认该函数的耗时非常短。
        */
        int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override {
            return STATE_OK;
        }
        
        /*处理RPC的返回结果。
        * 注意该函数将在子线程中执行，注意与其它函数的线程同步
        */
        int ProRemoteReturn(RemoteProcPtr proc) override {
            return STATE_OK;
        }
};



class VFR : public ARModule {
private:
    std::vector<Link_pair> objname_pairs;
public:
    int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override {
        ConfigLoader cfg(appData.sceneObjConfig);
        // VFR
        auto linkList = cfg.jsonData["VFR"];
        for (auto& linkPair : linkList){
            std::string realObjName = linkPair[0];
            std::string virtualObjName = linkPair[1];
            objname_pairs.push_back(Link_pair(realObjName, virtualObjName));
        }
        return STATE_OK;
    };

    int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override {
        for(const auto& link_pair:this->objname_pairs){
            auto robj = sceneData.getObject(link_pair.realObjName);
            auto vobj = sceneData.getObject(link_pair.virtualObjName);
            std::cout << "VFR:" << std::endl;
            std::cout << robj->name << std::endl;
            std::cout << vobj->name << std::endl;
            vobj->transform = robj->transform; // 根据真实物体的位姿更新虚拟物体
            // vobj->transform.setPose(robj->transform.GetMatrix());
            std::cout << robj->transform.GetMatrix() << std::endl;
            std::cout << vobj->transform.GetMatrix() << std::endl;
        }
        return STATE_OK;
    }

    int ShutDown(AppData& appData, SceneData& sceneData) override {
        // 实现关闭逻辑
        std::cout << "VFR module shutting down." << std::endl;
        return STATE_OK;
    }
};