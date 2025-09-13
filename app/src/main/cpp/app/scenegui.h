#ifndef ROKIDOPENXRANDROIDDEMO_SCENEGUI_H
#define ROKIDOPENXRANDROIDDEMO_SCENEGUI_H

#include"demos/gui.h"
#include"demos/text.h"
#include"opencv2/core.hpp"

class SceneGui {
public:
    struct ButtonItem{
        int width,height;
        std::string text;  //显示在按钮上的文本
        glm::mat4 translate_model{1.0f};
        float scale{1.0f};
        glm::mat4 layout_on_camera;
        bool useView{true};
    };
    struct TextItem{
        std::string text;
        glm::vec3 color{1.0f,1.0f,1.0f}; //默认为白色
        glm::mat4 translate_model{1.0f};
        float scale{1.0f};
        bool useView{true};
    };
    struct ImageItem{
        cv::Mat image;
        glm::mat4 translate_model{1.0f};
        float scale{1.0f};
        bool useView{true};
    };
public:
    SceneGui();
    ~SceneGui();

    void render(const glm::mat4 &p, const glm::mat4 &v, int32_t eye);


    bool add_button(const ButtonItem &item,const std::string &id={});  //往渲染列表里添加一个按钮.最后的string表示该Item的id,若不为空,则不能与其它的id重复
    bool add_text(const TextItem &item,const std::string &id={});      //往渲染列表里添加一段文字.最后的string表示该Item的id,若不为空,则不能与其它的id重复
    bool add_image(const ImageItem &item,const std::string &id={});    //往渲染列表里添加一张图片.最后的string表示该Item的id,若不为空,则不能与其它的id重复


private:
    void render_button(const glm::mat4 &p,const glm::mat4 &v,int32_t eye);
    void render_text(const glm::mat4 &p,const glm::mat4 &v,int32_t eye);
    void render_image(const glm::mat4 &p,const glm::mat4 &v,int32_t eye);

//    struct WindowItem{
//        std::shared_ptr<Gui> ptr;
//        int width,height;
//        glm::mat4 translate_model;
//        std::vector<std::string> informationList;
//        glm::mat4 layout_on_camera;
//    };

    std::vector<std::tuple<ButtonItem,std::shared_ptr<Gui>,std::string>>        mButtonList; //最后的string表示该Item的id,若不为空,则不能与其它的id重复
    std::vector<std::tuple<TextItem,Text,std::string>>                          mTextList;
    std::vector<std::tuple<ImageItem,std::shared_ptr<Gui>,GLuint,std::string>>  mImageList;  //GLuint表示图像的TextureID
};


#endif //ROKIDOPENXRANDROIDDEMO_SCENEGUI_H
