#pragma once
#include "glm/glm.hpp"

struct pbrMaterial
{
    // 对应albedo参数
    glm::vec3 albedoValue{1.0f, 1.0f, 1.0f};
    bool useAlbedoMap{false};
    unsigned int albedoMapId{0};

    // 对应normal参数
    glm::vec3 normalValue{0.0f, 0.0f, 1.0f};
    bool useNormalMap{false};
    unsigned int normalMapId{0};

    // 对应metallic参数
    float metallicValue{1.0f};
    bool useMetallicMap{false};
    unsigned int metallicMapId{0};

    // 对应roughness参数
    float roughnessValue{1.0f};
    bool useRoughnessMap{false};
    unsigned int roughnessMapId{0};

    // 对应ao参数
    float aoValue{1.0f};
    bool useAoMap{false};
    unsigned int aoMapId{0};

    // 保留一些原有的参数
    glm::vec4 emissiveFactor{0.0f, 0.0f, 0.0f, 1.0f};
    float alphaMask{1.0f};
    float alphaMaskCutoff{0.5f};

    pbrMaterial() {
        // 默认构造函数
    }
};
struct TinyModelVertex
{
    TinyModelVertex()
    {
        pos = glm::vec3(0.0f, 0.0f, 0.0f);
        uv = glm::vec2(0.0f, 0.0f);
        normal = glm::vec3(0.0f, 0.0f, 0.0f);
        tangent = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    };
    TinyModelVertex(glm::vec3 _pos, glm::vec2 _uv, glm::vec3 _normal, glm::vec4 _tangent)
    {
        pos = _pos;
        uv = _uv;
        normal = _normal;
        tangent = _tangent;
    }
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec4 tangent;

    bool operator==(const TinyModelVertex& other) const
    {
        return pos == other.pos && uv == other.uv && normal == other.normal;
    }
};
namespace std {
    // GLM vec2哈希特化
    template<typename T>
    struct hash<glm::vec<2, T>> {
        size_t operator()(const glm::vec<2, T>& vec) const noexcept {
            hash<T> hasher;
            size_t seed = 0;
            seed ^= hasher(vec.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    // GLM vec3哈希特化
    template<typename T>
    struct hash<glm::vec<3, T>> {
        size_t operator()(const glm::vec<3, T>& vec) const noexcept {
            hash<T> hasher;
            size_t seed = 0;
            seed ^= hasher(vec.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    // GLM vec4哈希特化
    template<typename T>
    struct hash<glm::vec<4, T>> {
        size_t operator()(const glm::vec<4, T>& vec) const noexcept {
            hash<T> hasher;
            size_t seed = 0;
            seed ^= hasher(vec.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    // TinyModelVertex哈希特化（使用GLM类型）
    template<>
    struct hash<TinyModelVertex> {
        size_t operator()(const TinyModelVertex& vertex) const noexcept {
            return ((hash<glm::vec3>()(vertex.pos) ^
                     (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
} // namespace std