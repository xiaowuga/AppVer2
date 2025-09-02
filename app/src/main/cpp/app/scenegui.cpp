#include "scenegui.h"


SceneGui::SceneGui() {

}

SceneGui::~SceneGui() {

}
void SceneGui::render(const glm::mat4 &p, const glm::mat4 &v, int32_t eye) {
    for (GuiItem item: this->guiList) {
        item.ptr->begin();

        for(std::string str : item.informationList ){
//            ImGui::BulletText("%s",str.c_str());
            ImGui::Text("%s",str.c_str());

        }
        ImGui::BulletText("test sceneGUI.cpp");

//        ImGui::Text("Please use the hand ray to pinch click.");
        item.ptr->end();
        item.ptr->render(p,v,item.layout_on_camera);
    }
}

int SceneGui::add_gui(int width, int height, const std::string &name,const glm::mat4 &translate_model) {
    GuiItem item;
    item.ptr=std::make_shared<Gui>(name.c_str());
    item.width=width; item.height=height;
    item.translate_model=translate_model;

    item.ptr->initialize(width,height);
    item.ptr->setModel(translate_model);

    //add in guiList
    guiList.push_back(item);
    return guiList.size();

}


int SceneGui::add_gui(int width, int height, const std::string &name,const glm::mat4 &translate_model,const glm::mat4 &layout_on_camera) {
    GuiItem item;
    item.ptr=std::make_shared<Gui>(name.c_str());
    item.width=width; item.height=height;
    item.translate_model=translate_model;
    item.translate_model=translate_model;
    item.layout_on_camera=layout_on_camera;

    item.ptr->initialize(width,height);
    item.ptr->setModel(translate_model);

    //add in guiList
    guiList.push_back(item);
    return guiList.size();
}

int SceneGui::add_gui(int width, int height, const std::string &name,
                      const glm::mat4 &translate_model,const glm::mat4 &layout_on_camera,const std::vector<std::string> &informationList) {

    GuiItem item;
    item.ptr=std::make_shared<Gui>(name.c_str());
    item.width=width; item.height=height;
    item.translate_model=translate_model;
    item.translate_model=translate_model;
    item.layout_on_camera=layout_on_camera;
    item.informationList=informationList;

    item.ptr->initialize(width,height);
    item.ptr->setModel(translate_model);

    //add in guiList
    guiList.push_back(item);
    return guiList.size();
}