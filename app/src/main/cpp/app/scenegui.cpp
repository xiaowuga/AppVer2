#include"scenegui.h"
#include"demos/model.h"
#include"demos/utils.h"
#include"utilsmym.hpp"
#include<GLES3/gl3.h>

//GLuint MatToTexture(const cv::Mat &image, GLuint &textureID, bool createNew = true) {
//    if (image.empty()) return 0;
//
//    GLenum format = GL_RGB;
//    if (image.channels() == 1) format = GL_LUMINANCE;
//    else if (image.channels() == 3) format = GL_RGB;
//    else if (image.channels() == 4) format = GL_RGBA;
//
//    if (createNew || textureID == 0) {
//        glGenTextures(1, &textureID);
//    }
//
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//    glTexImage2D(
//            GL_TEXTURE_2D, 0, format,
//            image.cols, image.rows,
//            0, format, GL_UNSIGNED_BYTE, image.data
//    );
//
//    glBindTexture(GL_TEXTURE_2D, 0);
//    return textureID;
//}
GLuint MatToTexture(const cv::Mat &image, GLuint &textureID, bool createNew = true) {
    if (image.empty()) return 0;

    GLenum format = GL_RGB;
    if (image.channels() == 1) format = GL_LUMINANCE;
    else if (image.channels() == 3) format = GL_RGB;
    else if (image.channels() == 4) format = GL_RGBA;

    int width = image.cols; int height = image.rows;
    int channel = image.channels();
    int pixellength = width * height * channel;    // 获取图像指针
    GLubyte *pixels = new GLubyte[pixellength];
    memcpy(pixels, image.data, pixellength * sizeof(char));

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    glTexImage2D(
            GL_TEXTURE_2D, 0, format,
            image.cols, image.rows,
            0, format, GL_UNSIGNED_BYTE, image.data
    );

    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

SceneGui::SceneGui() {

}
SceneGui::~SceneGui() {

}

void SceneGui::render(const glm::mat4 &p, const glm::mat4 &v, int32_t eye) {
    render_button(p,v,eye);
    render_text(p,v,eye);
    render_image(p,v,eye);
}

bool SceneGui::add_button(const SceneGui::ButtonItem &item,const std::string &id){
    if(!id.empty()){
        for(const auto &i:mButtonList) if(std::get<2>(i)==id) return false; //id 重复
    }
    //由于ImGui的name不能为空,所以当id为空时自动生成一个不重复的字符串作为构造的name参数
    std::string gui_name=id;
    if(gui_name.empty()) gui_name=std::to_string(CurrentMSecsSinceEpoch());
    auto ptr=std::make_shared<Gui>(gui_name);
    ptr->initialize(item.width+15,item.height+35); //为边框留出距离
    ptr->setModel(item.translate_model);
    ptr->setGuiFlags(ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse);
    mButtonList.emplace_back(item,ptr,id);
    return true;
}
bool SceneGui::add_text(const SceneGui::TextItem &item,const std::string &id){
    if(!id.empty()){
        for(const auto &i:mTextList) if(std::get<2>(i)==id) return false; //id 重复
    }
    Text text_render;
    text_render.initialize();
    mTextList.emplace_back(item,text_render,id);
    return true;
}
bool SceneGui::add_image(const SceneGui::ImageItem &item,const std::string &id){
    if(!id.empty()){
        for(const auto &i:mButtonList) if(std::get<2>(i)==id) return false; //id 重复
    }
    //由于ImGui的name不能为空,所以当id为空时自动生成一个不重复的字符串作为构造的name参数
    std::string gui_name=id;
    if(gui_name.empty()) gui_name=std::to_string(CurrentMSecsSinceEpoch());
    auto ptr=std::make_shared<Gui>(gui_name);
    ptr->initialize(item.image.cols+15,item.image.rows+35); //为边框留出距离
    ptr->setModel(item.translate_model);
    ptr->setGuiFlags(ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse);
    mImageList.emplace_back(item,ptr,0,id);
    return true;
}
void SceneGui::render_button(const glm::mat4 &p,const glm::mat4 &v,int32_t eye){
    for(const auto &[item,gui_ptr,id]:mButtonList){
//        mButton->isIntersectWithLine(linePoint, lineDirection);
        gui_ptr->begin();
        if(ImGui::Button(item.text.c_str(),ImVec2(float(item.width),float(item.height)))) {
            infof("IMGUI: Button Clicked");
        }
        gui_ptr->end();
        gui_ptr->setModel(glm::scale(item.translate_model,glm::vec3(item.scale)));
        gui_ptr->render(p,item.useView?v:glm::mat4(1.0f));
    }
}
void SceneGui::render_text(const glm::mat4 &p,const glm::mat4 &v,int32_t eye){
    static constexpr int TextBufferLength=1024;
    wchar_t text[TextBufferLength]={0};
    for(auto &[item,text_render,id]:mTextList){
        //split with \n TODO
        swprintf(text,TextBufferLength,L"%s",item.text.c_str());
        text_render.render(p,item.useView?v:glm::mat4(1.0f),glm::scale(item.translate_model,glm::vec3(item.scale)),text,(int)wcslen(text),item.color);
    }
}
void SceneGui::render_image(const glm::mat4 &p,const glm::mat4 &v,int32_t eye){
    for(auto &[item,gui_ptr,texture,id]:mImageList){
//        mButton->isIntersectWithLine(linePoint, lineDirection);
        gui_ptr->begin();
        ImVec2 size(400,300); // 控制显示大小
        cv::Mat rgbFrame;
        cv::cvtColor(item.image, rgbFrame, cv::COLOR_BGR2RGB);
        MatToTexture(rgbFrame,texture,texture == 0);
        ImGui::Image((ImTextureID)(intptr_t)(texture), size);
//        ImVec2 pos(ImGui::GetCursorScreenPos()); // 左上角位置
//        ImGui::GetWindowDrawList()->AddImage(
//                (ImTextureID)(intptr_t)texture,
//                pos,
//                ImVec2(pos.x+item.image.cols,pos.y+item.image.rows),// + ImGui::GetWindowSize(),
//                ImVec2(0, 0), ImVec2(1, 1));
        glDeleteTextures(1, &texture);
        gui_ptr->end();
        gui_ptr->setModel(glm::scale(item.translate_model,glm::vec3(item.scale)));
        gui_ptr->render(p,item.useView?v:glm::mat4(1.0f));
    }
}
