
//#define CVX_NO_CODEC

#include"core.cpp"
#include"ipf.cpp"
#include"mean_filter.cpp"
#include"resample.cpp"
#include"resize.cpp"

#ifndef __ANDROID__
#ifndef CVX_NO_GUI
#include"gui.cpp"
#endif
#endif

#ifndef CVX_NO_CODEC
#include"codec.cpp"
#endif

#ifndef CVX_NO_IMGSET
#include"imgset.cpp"
#endif


