#pragma once
#include <string>
#include <vector>
#include "common/gfxwrapper_opengl.h"
#include "logger.h"

#define OPENGL_DEBUG
#ifdef OPENGL_DEBUG
#define GL_CALL(_CALL)  do { _CALL; GLenum gl_err = glGetError(); if (gl_err != 0) Log::Write(Log::Level::Error,__FILE__,__LINE__,Fmt("GL error %d returned from '%s'", gl_err,#_CALL)); } while (0)
#else
#define GL_CALL(_CALL)
#endif

#define errorf(...)   Log::Write(Log::Level::Error,   __FILE__, __LINE__, Fmt(__VA_ARGS__));
#define warnf(...)    Log::Write(Log::Level::Warning, __FILE__, __LINE__, Fmt(__VA_ARGS__));
#define infof(...)    Log::Write(Log::Level::Info,    __FILE__, __LINE__, Fmt(__VA_ARGS__));
#define debugf(...)   Log::Write(Log::Level::Debug,   __FILE__, __LINE__, Fmt(__VA_ARGS__));
#define verbosef(...) Log::Write(Log::Level::Verbose, __FILE__, __LINE__, Fmt(__VA_ARGS__));


bool copyFile(const char* src, const char* dst);
unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
unsigned int TextureFromFileAssets(const char* path, const std::string& directory, bool gamma = false);
std::vector<char> readFileFromAssets(const char* file);
void refreshMedia(const std::string& path);
void setJNIEnv(JNIEnv *env);


#define HAND_LEFT  0
#define HAND_RIGHT 1
#define HAND_COUNT 2
#define HAND_JOINT_COUNT 21

#define EYE_LEFT  0
#define EYE_RIGHT 1
#define EYE_COUNT 2

#define GETSTR(str) #str

#define MEMBER_NAME(c, m) member_name<decltype(&c::m), &c::m>(#m)
template<typename T, T t>
constexpr const char *member_name(const char *name) noexcept {
    return name;
}

//============================================================================================================
//enum KeypadName{
//    Select=1<<1,
//    Menu=1<<2,
//    O=1<<3,
//    X=1<<4,
//    Up=1<<5,Down=1<<6,Left=1<<7,Right=1<<8,
//};
//static std::vector<std::pair<XrAction,KeypadName>> KeypadCheckList{{m_input.selectAction,    KeypadName::Select},
//                                                                   {m_input.menuAction,      KeypadName::Menu},
//                                                                   {m_input.oAction,         KeypadName::O},
//                                                                   {m_input.xAction,         KeypadName::X},
//                                                                   {m_input.upAction,        KeypadName::Up},
//                                                                   {m_input.downAction,      KeypadName::Down},
//                                                                   {m_input.leftAction,      KeypadName::Left},
//                                                                   {m_input.rightAction,     KeypadName::Right}
//};
