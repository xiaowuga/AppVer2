#pragma once

#include"cvrm.h"
#include"cvrmodel.h"

_CVR_API void cvrInit(const char *args = NULL, void *atexitFP=(void*)std::atexit);


//struct _CVRQuad;
//
//class _CVR_API CVRQuad
//	:public CVRRendable
//{
//	std::shared_ptr<_CVRQuad> _quad;
//public:
//	virtual void render(const Matx44f &sceneModelView, int flags);
//public:
//	CVRQuad(const cv::Point3f points[4], const cv::Mat &texImage);
//
//};

/* The matrixs related with OpenGL rendering, a 3D point X is transformed to an image point x by: x=X'*mModeli*mModel*mView*mProjection
*/
class _CVR_API CVRMats
{
public:
	Matx44f mModeli;      //transform the model to a standard state (postion, pose, size)
	Matx44f mModel;       //motion of the model
	Matx44f mView;        //motion of the camera
	Matx44f mProjection;  //projection from 3D to 2D (the projection matrix)
public:
	//initialize all matrix with mInit
	CVRMats(const Matx44f &mInit=cvrm::I());

	//set mView and mProjection
	//focalLength=fscale*viewHeight, fscale=sqrt(2)
	CVRMats(Size viewSize, float fscale = 1.5f, float eyeDist = 4.0f, float zNear = 0.1f, float zFar = 100.f);

	//set the matrixs to show a model in a standard way (used in mdshow(...) )
	CVRMats(const CVRModel &model, Size viewSize, float fscale = 1.5f, float eyeDist = 4.0f, float zNear = 0.1f, float zFar = 100.f);

	void setUtilizedModelView(const CVRModel &model, float eyeDist = 4.0f);

	//set Mats to show the model in an image or in an image ROI
	//sizeScale : scale of the object size in image space (by adjust eyeDist)
	//zNear,zFar: <=0 if need to be auto estimated
	void setInImage(const CVRModel &model, Size viewSize, const Matx33f &K, float sizeScale = 1.0f, float zNear=-1.f, float zFar=-1.f);

	void setInROI(const CVRModel &model, Size viewSize, cv::Rect roi, const Matx33f &K, float sizeScale = 1.0f, float zNear=-1.f, float zFar=-1.f);

	Matx44f modelView() const
	{
		return mModeli*mModel*mView;
	}
};

class _CVR_API CVRResult
{
public:
	cv::Mat  img;
	cv::Mat1f  depth;
	CVRMats    mats;
	cv::Rect   outRect;
public:
	void getDepthRange(float &minDepth, float &maxDepth) const;

	float getDepthRange() const;

	cv::Mat1f getNormalizedDepth() const;

	bool  getDepth(float x, float y, float &d) const;
	
	bool  isVisible(const cv::Point3f &pt, float depthDelta=1e-3f) const
	{
		float dz;
		return this->getDepth(pt.x, pt.y, dz) && pt.z < dz + depthDelta;
	}

	cv::Mat1b getMaskFromDepth(float eps = 1e-6f) const;

	//convert OpenGL depth (in [-1, 1]) to CV depth
	cv::Mat1f getZDistFromGLDepth() const;

	static CVRResult blank(Size viewSize, const CVRMats &_mats);
};

class _CVR_API CVRProjector
{
	cv::Matx44f _mModelView;
	cv::Matx44f _mProjection;
	cv::Vec4i   _viewport;
	
	cv::Mat1f   _depth;
	cv::Point2f _depthOffset;
public:
	CVRProjector();

	CVRProjector(const CVRResult &rr, cv::Size viewSize=cv::Size(0,0));

	CVRProjector(const CVRMats &mats, cv::Size viewSize);

	CVRProjector(const cv::Matx44f &mModelView, const cv::Matx44f &mProjection, cv::Size viewSize);

	CVRProjector(const cv::Matx44f &mModelView, const cv::Matx33f &cameraK, cv::Size viewSize, float nearP = 1.0f, float farP = 100.0f);

	//project 3D point to 2D
	cv::Point3f project(float x, float y, float z) const
	{
		cv::Point3f p = cvrm::project(cv::Point3f(x, y, z), _mModelView, _mProjection, &_viewport[0]);
		p.y = _viewport[3] - p.y;
		return p;
	}
	cv::Point3f project(const cv::Point3f &pt) const
	{
		return project(pt.x, pt.y, pt.z);
	}
	cv::Point2f project2(const cv::Point3f& pt) const
	{
		auto p=project(pt.x, pt.y, pt.z);
		return cv::Point2f(p.x, p.y);
	}
	void  project(const cv::Point3f vpt[], cv::Point3f dpt[], int count) const
	{
		for (int i = 0; i < count; ++i)
			dpt[i] = project(vpt[i]);
	}
	void project(const std::vector<cv::Point3f> &vpt, std::vector<cv::Point3f> &dpt) const
	{
		dpt.resize(vpt.size());
		if (!vpt.empty())
			project(&vpt[0], &dpt[0], (int)vpt.size());
	}
	void  project(const cv::Point3f vpt[], cv::Point2f dpt[], int count) const
	{
		for (int i = 0; i < count; ++i)
		{
			cv::Point3f ptx= project(vpt[i]);
			dpt[i] = cv::Point2f(ptx.x, ptx.y);
		}
	}
	void project(const std::vector<cv::Point3f> &vpt, std::vector<cv::Point2f> &dpt) const
	{
		dpt.resize(vpt.size());
		if (!vpt.empty())
			project(&vpt[0], &dpt[0], (int)vpt.size());
	}
	void project(const std::vector<cv::Point3f> &vpt, std::vector<cv::Point> &dpt) const
	{
		dpt.resize(vpt.size());
		for (size_t i = 0; i < vpt.size(); ++i)
		{
			cv::Point3f ptx = project(vpt[i]);
			dpt[i] = cv::Point(int(ptx.x + 0.5), int(ptx.y + 0.5));
		}
	}
	//unproject a 2D point (with depth) to 3D
	cv::Point3f unproject(float x, float y, float depth) const
	{
		return cvrm::unproject(cv::Point3f(x, _viewport[3] - y, depth), _mModelView, _mProjection, &_viewport[0]);
	}

	cv::Point3f unproject(float x, float y) const;
	
	cv::Point3f unproject(const cv::Point2f &pt) const
	{
		return unproject(pt.x, pt.y);
	}
	void  unproject(const cv::Point2f vpt[], cv::Point3f dpt[], int count) const
	{
		for (int i = 0; i < count; ++i)
			dpt[i] = unproject(vpt[i]);
	}
	void unproject(const std::vector<cv::Point2f> &vpt, std::vector<cv::Point3f> &dpt) const
	{
		dpt.resize(vpt.size());
		if (!vpt.empty())
			unproject(&vpt[0], &dpt[0], (int)vpt.size());
	}
};

class CVRProjectorKRt
{
	cv::Matx33f _KR;
	cv::Vec3f _Kt;
public:
	CVRProjectorKRt(const cv::Matx33f& _K, const cv::Matx33f& _R, const cv::Vec3f& _t)
		:_KR(_K* _R), _Kt(_K* _t)
	{
	}
	cv::Point2f operator()(const cv::Point3f& P) const
	{
		cv::Vec3f p = _KR * cv::Vec3f(P) + _Kt;
		return cv::Point2f(p[0] / p[2], p[1] / p[2]);
	}
	template<typename _ValT, typename _getPointT>
	std::vector<cv::Point2f> operator()(const std::vector<_ValT>& vP, _getPointT getPoint) const
	{
		std::vector<cv::Point2f> vp(vP.size());
		for (int i = 0; i < (int)vP.size(); ++i)
			vp[i] = (*this)(getPoint(vP[i]));
		return vp;
	}
	template<typename _ValT>
	std::vector<cv::Point2f> operator()(const std::vector<_ValT>& vP) const
	{
		return (*this)(vP, [](const _ValT& v) {return v; });
	}
};

enum
{
	CVRM_ENABLE_LIGHTING=0x01,
	CVRM_ENABLE_TEXTURE=0x02,
	CVRM_ENABLE_MATERIAL=0x04,
	CVRM_ENABLE_VERTEX_COLOR=0x08,
	CVRM_TEXTURE_NOLIGHTING=0x10,
	
	//rendering with default lighting, texture and material
	CVRM_ENABLE_ALL= CVRM_ENABLE_LIGHTING| CVRM_ENABLE_TEXTURE | CVRM_ENABLE_MATERIAL | CVRM_ENABLE_VERTEX_COLOR,

	//for object with textures, rendering texture color without lighting effect.
	CVRM_TEXCOLOR= CVRM_ENABLE_ALL | CVRM_TEXTURE_NOLIGHTING,

	CVRM_VERTCOLOR=CVRM_ENABLE_VERTEX_COLOR,

	CVRM_DEFAULT=CVRM_TEXCOLOR 
};

enum
{
	CVRM_IMAGE=0x01,
	CVRM_DEPTH=0x02,
//	CVRM_ALPHA=0x04
//	CVRM_NO_VFLIP = 0x10,
	CVRM_FLIP = 0x10,
};


class _CVRender;

class _CVR_API CVRender
{
public:
	class _CVR_API UserDraw
	{
	public:
		virtual void draw() = 0;

		virtual ~UserDraw();
	};

	template<typename _DrawOpT>
	class UserDrawX
		:public UserDraw
	{
		_DrawOpT _op;
	public:
		UserDrawX(const _DrawOpT &op)
			:_op(op)
		{}
		virtual void draw()
		{
			_op();
		}
	};
public:
	std::shared_ptr<_CVRender> impl;
public:
	CVRender();

	CVRender(CVRRendable &rendable);

	CVRender(CVRRendablePtr rendablePtr);

	~CVRender();

	bool empty() const;

	operator bool() const
	{
		return !empty();
	}

	void setBgImage(const cv::Mat &img);

	void clearBgImage()
	{
		this->setBgImage(cv::Mat());
	}

	void setBgColor(float r, float g, float b, float a=1.0f);

	CVRResult exec(CVRMats &mats, Size viewSize, int output=CVRM_IMAGE|CVRM_DEPTH, int flags=CVRM_DEFAULT, UserDraw *userDraw=NULL, cv::Rect outRect=cv::Rect(0,0,0,0));

	//for python call
	CVRResult __exec(CVRMats &mats, Size viewSize, int output = CVRM_IMAGE | CVRM_DEPTH, int flags = CVRM_DEFAULT, cv::Rect outRect = cv::Rect(0, 0, 0, 0))
	{
		return this->exec(mats, viewSize, output, flags, (UserDraw*)NULL, outRect);
	}

	//facilitate user draw with lambda functions
	template<typename _UserDrawOpT>
	CVRResult exec(CVRMats &mats, Size viewSize, _UserDrawOpT &op, int output = CVRM_IMAGE | CVRM_DEPTH, int flags = CVRM_DEFAULT, cv::Rect outRect = cv::Rect(0, 0, 0, 0))
	{
		UserDrawX<_UserDrawOpT> userDraw(op);
		return exec(mats, viewSize, output, flags, &userDraw, outRect);
	}

	//render with a temporary bg-image. The bg-image set by setBgImage will not be changed.
	//CVRResult exec(CVRMats &mats, const cv::Mat &bgImg, int output = CVRM_IMAGE | CVRM_DEPTH, int flags = CVRM_DEFAULT);

	//const CVRModel& model() const;
};

template<typename _DrawOpT>
inline std::shared_ptr<CVRender::UserDraw> newUserDraw(const _DrawOpT &op)
{
	return std::shared_ptr<CVRender::UserDraw>(new CVRender::UserDrawX<_DrawOpT>(op));
}

_CVR_API void drawPoints(const cv::Point3f pts[], int npts, float pointSize=1.0f);



#include<mutex>
class _CVR_API CVRShowModelBase
{
public:
	class _CVR_API ResultFilter
	{
	public:
		virtual void exec(CVRResult &result) = 0;

		virtual ~ResultFilter();
	};
	template<typename _FilterT>
	class ResultFilterX
		:public ResultFilter
	{
		_FilterT _op;
	public:
		ResultFilterX(const _FilterT &op)
			:_op(op)
		{}
		virtual void exec(CVRResult &result)
		{
			_op(result);
		}
	};
public:
	CVRModel	 model;
	Size         viewSize;
	CVRMats      initMats;
	cv::Mat      bgImg;
	int          renderFlags;
	std::shared_ptr<CVRender::UserDraw> userDraw;
	std::shared_ptr<ResultFilter>  resultFilter;

	CVRender	 render;
	CVRTrackBall trackBall;
	
protected:
	std::mutex   _resultMutex;
	CVRResult    _currentResult;
	bool         _hasResult = false;
public:
	CVRShowModelBase(const CVRModel &_model, Size _viewSize, const CVRMats &_mats, const cv::Mat &_bgImg, int _renderFlags)
		:model(_model),viewSize(_viewSize),initMats(_mats),bgImg(_bgImg),renderFlags(_renderFlags)
	{}

	virtual ~CVRShowModelBase();

	//wait until the background rendering thread becomes idle, which means all rendering messages have been processed
	//call this function before accessing data in the window's event loop, especially for events that may trigger re-rendering, such as mouse-move, left-button-down, left-button-up, etc.
	void waitDone();

	//re-rendering the model. If @waitDone is true, @currentResult will be updated when this function return, otherwise it may have not.
	void update(bool waitDone=false);

	virtual void showModel(const CVRModel &model);

	//called by the bg-thread
	void   setCurrentResult(const CVRResult &r);

	//called in the main-thread
	//return True if has result
	bool   showCurrentResult(bool waitResult=false);

	const CVRResult& getCurrentResult() const
	{
		return _currentResult;
	}

	virtual void present(CVRResult &result) =0;
};

template<typename _FilterOpT>
inline std::shared_ptr<CVRShowModelBase::ResultFilter> newResultFilter(const _FilterOpT &op)
{
	return std::shared_ptr<CVRShowModelBase::ResultFilter>(new CVRShowModelBase::ResultFilterX<_FilterOpT>(op));
}

class _CVRShowModelEvent
{
public:
	enum {
		F_IGNORABLE=0x01
	};
public:
	CVRShowModelBase *winData;
	int code;
	int param1, param2;
	int data;
	int flags=0;
public:
	_CVRShowModelEvent()
	{}
	_CVRShowModelEvent(CVRShowModelBase *_winData, int _code, int _param1, int _param2, int _data, int _flags=0)
		:winData(_winData),code(_code),param1(_param1),param2(_param2), data(_data),flags(_flags)
	{}
};

void _CVR_API _postShowModelEvent(const _CVRShowModelEvent &evt);

//============================================================================
#ifndef CVRENDER_NO_GUI

//show model in CVX windows (based on opencv windows)
#include"CVX/gui.h"

 class CVXShowModel
 	:public CVRShowModelBase
 {
 public:
 	cv::CVWindow *wnd;
 public:
 	CVXShowModel(cv::CVWindow *_wnd, const CVRModel &_model, Size _viewSize, const CVRMats &_mats, const cv::Mat &_bgImg, int _renderFlags)
 		:wnd(_wnd), CVRShowModelBase(_model,_viewSize,_mats,_bgImg,_renderFlags)
 	{}

 	virtual void present(CVRResult &result)
 	{
 		if (wnd && !result.img.empty())
 			wnd->show(result.img);
 	}
 };


typedef std::shared_ptr<CVXShowModel> CVXShowModelPtr;

 inline CVXShowModelPtr mdshow(const std::string &wndName, const CVRModel &model, Size viewSize=Size(800,800), int renderFlags = CVRM_DEFAULT, const cv::Mat bgImg=cv::Mat())
{
	CVXShowModelPtr dptr;
	//return dptr;
	
	cv::CVWindow *wnd = cv::getWindow(wndName, true);
	
	if (wnd)
	{
		//realize the window
		cv::Mat1b img=cv::Mat1b::zeros(viewSize);
		wnd->show(img);

		CVRMats mats(model,viewSize);

		dptr= CVXShowModelPtr(new CVXShowModel(wnd,model, viewSize, mats, bgImg, renderFlags));

		wnd->setEventHandler([dptr](int evt, int param1, int param2, cv::CVEventData data) {
			//dptr->showCurrent();
			_postShowModelEvent(_CVRShowModelEvent(dptr.get(), evt, param1, param2,data.ival,_CVRShowModelEvent::F_IGNORABLE));
			
		}, "cvxMDShowEventHandler");

		dptr->update(true);
		//wait and show the first image
		dptr->showCurrentResult(true);
	}
	return dptr;
}

#endif
