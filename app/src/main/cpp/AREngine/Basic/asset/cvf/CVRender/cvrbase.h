#pragma once


#ifndef CVRENDER_SHARED
#define _CVR_API
#endif

#ifndef _CVR_API

#if defined(_WIN32)

#ifdef CVRENDER_EXPORTS
#define _CVR_API __declspec(dllexport)
#else
#define _CVR_API __declspec(dllimport)
#endif

#elif defined(__GNUC__) && __GNUC__ >= 4

#ifdef CVRENDER_EXPORTS
#define _CVR_API __attribute__ ((visibility ("default")))
#else
#define _CVR_API 
#endif

#elif
#define _CVR_API
#endif
#endif

#ifdef _MSC_VER

#pragma warning(disable:4267)
#pragma warning(disable:4251)
#pragma warning(disable:4190)
#pragma warning(disable:4819)

#endif


#include"opencv2/core/core.hpp"
#include<memory>

using cv::Matx44f;
using cv::Matx33f;
using cv::Size;
using cv::Point3f;

