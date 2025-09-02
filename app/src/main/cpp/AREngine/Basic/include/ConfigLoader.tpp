// #ifndef CONFIG_LOADER_TPP
// #define CONFIG_LOADER_TPP

// #include "ConfigLoader.h"
// #include <stdexcept>
// #include <cctype>
// #include <cstdlib>

// template<typename T>
// T ConfigLoader::getValue(const std::string& key) const {
//     std::string::size_type pos = findKey(key);
//     if (pos == std::string::npos) {
//         throw std::runtime_error("Key not found: " + key);
//     }

//     // 这里需要根据 T 的类型来解析数据
//     // 示例仅展示基本类型，对于复杂类型需要扩展解析方法
//     pos = jsonText.find(':', pos) + 1;
//     std::string::size_type end = jsonText.find(',', pos);
//     if (end == std::string::npos) end = jsonText.find('}', pos);
//     std::string valueStr = jsonText.substr(pos, end - pos);
//     std::istringstream iss(valueStr);
//     T value;
//     iss >> value;
//     if (iss.fail()) throw std::runtime_error("Parse error");
//     // remove " from json
//     if constexpr (std::is_same<T, std::string>::value){
//         if(!value.empty()&&value.front()=='"'&&value.back()=='"'){
//             value = value.substr(1, value.length()-2);
//         }
//     }

//     return value;
// }

// #endif // CONFIG_LOADER_TPP

#ifndef CONFIG_LOADER_TPP
#define CONFIG_LOADER_TPP

#include "ConfigLoader.h"
#include <iostream>
template<typename T>
T ConfigLoader::getValue(const std::string& key) const {
    std::cout << "ConfigLoader getValue:" << key << "|" << jsonData[key] <<std::endl;
    T result = jsonData[key];
    return result;
}

#endif // CONFIG_LOADER_TPP