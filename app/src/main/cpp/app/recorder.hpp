#pragma once
#ifndef ROKIDOPENXRANDROIDDEMO_RECORDER_HPP
#define ROKIDOPENXRANDROIDDEMO_RECORDER_HPP

#include<opencv2/opencv.hpp>
#include"utilsmym.hpp"

class Recorder{
public:
    std::string start_recording(){recording_state=true; return {};}
    std::string end_recording(){recording_state=false; return {};}
    std::string set_recorder_save_dir(const std::string &dir){
        if(!MakeDir(dir)){
            return (("Set Recording Dir: '"+dir+"' Fail!"));
        }
        save_dir=MakeSdcardPath(dir);
        return {};
    }
    std::string record_image(const cv::Mat &image,const std::string &image_name={}){
        if(!recording_state) return "Not Recording";
        auto file_name=image_name;
        if(file_name.empty()) file_name=std::to_string(CurrentMSecsSinceEpoch())+".png";
        if(!save_dir.empty()&&save_dir[save_dir.size()-1]!='/'&&save_dir[save_dir.size()-1]!='\\') file_name='/'+file_name;
        file_name=save_dir+file_name;
        if(!cv::imwrite(file_name, image)) {
            return (("Failed to Save Record Image: "+file_name));
        }
//        infof(std::string("Record Image Success: "+file_name).c_str());
        return {};
    }
    bool is_recording(){return recording_state;}

private:
    bool recording_state{false};
    std::string save_dir;
};


#endif //ROKIDOPENXRANDROIDDEMO_RECORDER_HPP
