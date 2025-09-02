#ifndef ROKIDOPENXRANDROIDDEMO_SCENEGUI_H
#define ROKIDOPENXRANDROIDDEMO_SCENEGUI_H

#include"demos/gui.h"
#include"demos/text.h"

class SceneGui {
public:
    SceneGui();
    ~SceneGui();
    void render(const glm::mat4 &p, const glm::mat4 &v, int32_t eye);

    int add_gui(int width,int height,const std::string &name={},const glm::mat4 &translate_model=glm::mat4(1.0f));

    int add_gui(int width, int height, const std::string &name,const glm::mat4 &translate_model,const glm::mat4 &layout_on_camera);

    int add_gui(int width, int height, const std::string &name,const glm::mat4 &translate_model,
                const glm::mat4 &layout_on_camera , const std::vector<std::string> &informationList);



private:

    struct GuiItem{
        std::shared_ptr<Gui> ptr;
        int width,height;
        glm::mat4 translate_model;
        std::vector<std::string> informationList;
        glm::mat4 layout_on_camera;
    };
    struct TextItem{
        std::string text;
        glm::vec3 color;
        glm::mat4 translate_model;
    };
    std::vector<GuiItem>    guiList;
    std::vector<TextItem>   textList;
};


#endif //ROKIDOPENXRANDROIDDEMO_SCENEGUI_H
