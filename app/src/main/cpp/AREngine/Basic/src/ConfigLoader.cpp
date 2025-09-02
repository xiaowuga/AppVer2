#include "ConfigLoader.h"
#include <fstream>
#include <stdexcept>

ConfigLoader::ConfigLoader(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
    file >> jsonData;  // 使用 nlohmann/json 直接解析文件
    file.close();
}

cv::Matx44f ConfigLoader::get44Matrix(const std::string& key) const {
    auto matrixJson = jsonData.at(key);
    cv::Matx44f matrix;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix(i, j) = matrixJson[i][j];
        }
    }
    return matrix;
}

cv::Matx44f ConfigLoader::get44Matrix(const nlohmann::json jsonParam, const std::string& key) const {
    cv::Matx44f matrix = cv::Matx44f::eye();
    auto matrixJson = jsonParam.at(key);
    std::cout << "matrixJson.size() == " << matrixJson.size() << std::endl;
    if(matrixJson.size() == 0){
        return matrix;
    }else{
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                matrix(i, j) = matrixJson[i][j];
            }
        }
    }
    return matrix;
}

glm::mat4 ConfigLoader::getglmmat4Matrix(const std::string& key) const {
    auto matrixJson = jsonData.at(key);
    glm::mat4 matrix = glm::mat4(1.0);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix[j][i] = matrixJson[i][j];
        }
    }
    return matrix;
}

cv::Matx33f ConfigLoader::getCamIntrinsicMatrix() const {
    return cv::Matx33f(
        jsonData["fx"], 0, jsonData["cx"],
        0, jsonData["fy"], jsonData["cy"],
        0, 0, 1);
}

cv::Matx14f ConfigLoader::getCamDistCoeff() const {
    return cv::Matx14f(
        jsonData["k1"], 
        jsonData["k2"], 
        jsonData["p1"], 
        jsonData["p2"]);
}