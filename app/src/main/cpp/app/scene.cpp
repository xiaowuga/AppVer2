
#include "scene.h"


std::shared_ptr<IScene> createScene(const std::string &name, IApplication *app){
    std::shared_ptr<IScene> _createScene_empty();
    std::shared_ptr<IScene> _createScene_marker_test();
    std::shared_ptr<IScene> _createScene_engine_test_rpc();
    std::shared_ptr<IScene> _createScene_engine_test_aruco();
    std::shared_ptr<IScene> _createScene_marker_test_rpc();
    std::shared_ptr<IScene> _createScene_3dtracking_test();

    struct DFunc{
        std::string name;
        typedef std::shared_ptr<IScene> (*createFuncT)();
        createFuncT  create;
    };

    DFunc funcs[]= {
            {"empty", _createScene_empty},
            {"marker_test", _createScene_marker_test},
            {"engine_test_rpc", _createScene_engine_test_rpc},
            {"engine_test_aruco", _createScene_engine_test_aruco},
            {"marker_test_rpc", _createScene_marker_test_rpc},
            {"3dtracking_test",_createScene_3dtracking_test},
            //list other scenes
    };

    std::shared_ptr<IScene> ptr=nullptr;
    for(auto &f : funcs){
        if(f.name==name) {
            ptr = f.create();
            break;
        }
    }
    if(ptr){
        ptr->m_app=app;
        ptr->m_program=app? app->m_program : nullptr;
    }

    return ptr; //原来是return nullptr,可能是写错了
}


class SceneEmpty
        :public IScene
{
public:
    virtual bool initialize(const XrInstance instance, const XrSession session) {
        return true;
    }
};

std::shared_ptr<IScene> _createScene_empty()
{
    return std::make_shared<SceneEmpty>();
}

