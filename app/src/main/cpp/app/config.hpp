#ifndef ROKIDOPENXRANDROIDDEMO_CONFIG_HPP
#define ROKIDOPENXRANDROIDDEMO_CONFIG_HPP
#pragma once

#include<string>
#include<unordered_map>
#include<fstream>
#include<sstream>
#include<iostream>
#include<algorithm>

class Config{
public:
    explicit Config(const std::string &path={}){
        filePath=path;
        if(!path.empty()) load(path);
    }
    bool load(const std::string &path){
        std::ifstream in(path);
        if(!in) return false;

        filePath=path;
        data.clear();
        std::string section,line;

        while(std::getline(in,line)){
            trim(line);
            if(line.empty()||line[0]==';'||line[0]=='#')
                continue;
            if(line.front()=='['&&line.back()==']'){
                section=line.substr(1,line.size()-2);
                trim(section);
            }
            else{
                auto pos=line.find('=');
                if(pos==std::string::npos) continue;
                std::string key=line.substr(0,pos);
                std::string value=line.substr(pos+1);
                trim(key);
                trim(value);
                data[section+"."+key]=value;
            }
        }
        return true;
    }
    bool save(const std::string &path="")const{
        std::string outPath=path.empty()?filePath:path;
        if(outPath.empty()) return false;
        std::ofstream out(outPath);
        if(!out) return false;
        std::string lastSection;
        for(const auto &[fullKey,value]:data){
            auto pos=fullKey.find('.');
            std::string section=fullKey.substr(0,pos);
            std::string key=fullKey.substr(pos+1);
            if(section!=lastSection){
                if(!lastSection.empty()) out<<"\n";
                out<<"["<<section<<"]\n";
                lastSection=section;
            }
            out<<key<<" = "<<value<<"\n";
        }
        return true;
    }
    std::string get(const std::string &section,const std::string &key,const std::string &def="")const{
        auto it=data.find(section+"."+key);
        return it!=data.end()?it->second:def;
    }
    std::string get(const std::string &key,const std::string &def="")const{
        return get("global",key,def);
    }
    void set(const std::string &key,const std::string &value){
        set("global",key,value);
    }
    void set(const std::string &section,const std::string &key,const std::string &value){
        data[section+"."+key]=value;
    }
    static Config& global(){
        static Config GlobalConfig("/storage/emulated/0/Download/config.ini");
        return GlobalConfig;
    }
    // 禁止拷贝和赋值
    Config(const Config&)=delete;
    Config& operator=(const Config&)=delete;

private:
    std::string filePath;
    std::unordered_map<std::string,std::string> data;

    static void trim(std::string &s){
        auto notSpace=[](unsigned char ch){return !std::isspace(ch);};
        s.erase(s.begin(),std::find_if(s.begin(),s.end(),notSpace));
        s.erase(std::find_if(s.rbegin(),s.rend(),notSpace).base(),s.end());
    }
};

/*
 * ** Demo:
    IniFile ini;
    if (ini.load("config.ini")) {
        std::string ss = ini.get("global", "setting_item", "sss_aaa");
        int num = std::stoi(ini.get("global", "num", "23321"));
    }
    ini.set("network", "host", "192.168.1.100");
    ini.set("network", "port", "9090");
    ini.save("config.ini");
 */


#endif //ROKIDOPENXRANDROIDDEMO_CONFIG_HPP
