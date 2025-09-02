// #ifndef CONFIG_LOADER_H
// #define CONFIG_LOADER_H

// #include <string>
// #include <vector>
// #include <fstream>
// #include <sstream>
// #include <opencv2/core/matx.hpp>  // 包含cv::Matx

// class ConfigLoader {
// public:
//     explicit ConfigLoader(const std::string& filePath);  // 使用文件路径作为参数

//     template<typename T>
//     T getValue(const std::string& key) const;

//     cv::Matx44f get44Matrix(const std::string& key) const;
//     cv::Matx33f getCamIntrinsicMatrix() const;  // 新增
//     cv::Matx14f getCamDistCoeff() const;       // 新增

// private:
//     std::string jsonText;
//     std::string::size_type findKey(const std::string& key) const;
// };

// #include "ConfigLoader.tpp"  // 包含模板实现

// #endif // CONFIG_LOADER_H

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <string>
#include "json.hpp"
#include <opencv2/core/matx.hpp>
#include <glm/glm.hpp>

class ConfigLoader {
public:
    explicit ConfigLoader(const std::string& filePath);  // 使用文件路径作为参数

    template<typename T>
    T getValue(const std::string& key) const;

    cv::Matx44f get44Matrix(const std::string& key) const;
    cv::Matx44f get44Matrix(const nlohmann::json jsonParam, const std::string& key) const;
    glm::mat4 getglmmat4Matrix(const std::string& key) const;
    cv::Matx33f getCamIntrinsicMatrix() const;  // 新增
    cv::Matx14f getCamDistCoeff() const;       // 新增
    nlohmann::json jsonData;
};

#include "ConfigLoader.tpp"  // 包含模板实现

#endif // CONFIG_LOADER_H