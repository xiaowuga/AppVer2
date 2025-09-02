//#include"GL/glew.h"
//#include"GL/wglew.h"

#include"_cvrender.h"

#include"BFC/argv.h"

#include<list>
#include<condition_variable>
#include<mutex>
#include<thread>
#include<chrono>
#include<iostream>

#if 0
class _CVR_API CVRLock
{
public:
	~CVRLock();
};

typedef std::shared_ptr<CVRLock>  CVRLockPtr;

_CVR_API CVRLockPtr cvrLockGL();

//for using OpenGL in the multi-thread environment
#define CVR_LOCK() CVRLockPtr lockOpenGL=cvrLockGL();


static std::recursive_mutex g_glMutex;

CVRLock::~CVRLock()
{
	g_glMutex.unlock();
}

_CVR_API CVRLockPtr cvrLockGL()
{
	g_glMutex.lock();

	return CVRLockPtr(new CVRLock);
}
#endif


static void displayCB()
{
	//glutSwapBuffers();
	//std::cout << "render in:" << std::this_thread::get_id() << std::endl;
}

//static void onIdle()
//{
//	glutMainLoopEvent();
//}

void _initGL()
{
#if !defined(__ANDROID__)
	glShadeModel(GL_SMOOTH);

	//enable anti-alias
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glEnable(GL_NORMALIZE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
#endif
}

void cvrInit2(const char *args);

static decltype(std::atexit) *g_atexitFP = nullptr;

_CVR_API void cvrInit(const char *args, void *atexitFP)
{
	if(!g_atexitFP)
		g_atexitFP = (decltype(std::atexit) *)atexitFP;
	cvrInit2(args);
}

//_CVR_API void cvrUninit()
//{
//	cvrWaitFinish();
//}

#ifdef _WIN32

#if defined(USE_GLUT)
void cvrInit2(const char *args)
{
	static bool inited = false;
	if (!inited)
	{
		int argc = 1;
		char *argv[] = {(char*)""};

		glutInit(&argc, argv);
		inited = true;
	}
}

class _CVRDevice
{
public:
	int _id = -1;
	Size _size = Size(-1, -1);
public:
	~_CVRDevice()
	{
		//release();
	}
	void release()
	{
		if (_id >= 0)
		{
			glutDestroyWindow(_id);
			_id = -1;
		}
	}
	void create(int width, int height, int flags, const std::string &name)
	{
		cvrInit2(NULL);

		//std::cout <<"init. GLUT in:" << std::this_thread::get_id() << std::endl;

		this->release();
		glutInitDisplayMode(flags&CVRW_GLUT_MASK);
		//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
		//glDisable(0x809D);
		

		glutInitWindowSize(width, height);
		_size = Size(width, height);

		_id = glutCreateWindow(name.c_str());
		if (!(flags&CVRW_VISIBLE))
		{
			glutHideWindow();
			glutPostWindowRedisplay(_id);
		}

		glutDisplayFunc(displayCB);
		//glutIdleFunc(onIdle);
		glutMainLoopEvent();

		_initGL();
	}

	void setCurrent()
	{
		//CVR_LOCK();

		if (_id >= 0 && _id != glutGetWindow())
			glutSetWindow(_id);
	}
	void setSize(int width, int height)
	{
		Size newSize(width, height);
		if (newSize != _size)
		{
			//	CVR_LOCK();

			this->setCurrent();
			glutReshapeWindow(width, height);
			glutMainLoopEvent();

			glViewport(0, 0, width, height);
			_size = newSize;
		}
	}
	void postRedisplay()
	{
		//glutPostWindowRedisplay(_id);
		//glutPostRedisplay();
		//glutMainLoopEvent();
	}
};

#else

void cvrInit2(const char *args)
{
	static bool inited = false;
	if (!inited)
	{
		if (glfwInit() != GLFW_TRUE)
			FF_EXCEPTION1("glfwInit failed");

		inited = true;
	}
}

class _CVRDevice
{
public:
	GLFWwindow *_wnd=NULL;
	Size _size = Size(-1, -1);
public:
	~_CVRDevice()
	{
	}
	void release()
	{
		if (_wnd)
		{
			glfwDestroyWindow(_wnd);
			_wnd = NULL;
		}
	}
	void create(int width, int height, int flags, const std::string &name)
	{
		cvrInit2(NULL);

		this->release();

		//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

		printf("Use GL version:%s\n", glfwGetVersionString());

		auto wnd = glfwCreateWindow(width, height, "window", nullptr, nullptr);
		if (wnd == nullptr)
			FF_EXCEPTION1("failed to create GLFW window");


		glfwMakeContextCurrent(wnd);
		if (glewInit() != GLEW_OK) 
		{
			glfwDestroyWindow(wnd);
			wnd = nullptr;
			glfwTerminate();
			FF_EXCEPTION1("failed to init. GLEW");
		}
		//glfwMakeContextCurrent(nullptr);
		
		_size = Size(width, height);
		_wnd = wnd;

		_initGL();
	}

	void setCurrent()
	{
		//CVR_LOCK();

		if (_wnd)
			glfwMakeContextCurrent(_wnd);
	}
	void setSize(int width, int height)
	{
		Size newSize(width, height);
		if (_wnd && newSize != _size)
		{
			this->setCurrent();
			
			glfwSetWindowSize(_wnd, width, height);
			glViewport(0, 0, width, height);
			_size = newSize;
#if 0
			GLuint fbo_,rbo_depth_,rbo_normal_;

			glGenRenderbuffers(1, &rbo_normal_);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo_normal_);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width,height);
			//glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glGenRenderbuffers(1, &rbo_depth_);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth_);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
			//glBindRenderbuffer(GL_RENDERBUFFER, 0);

			// Initialize framebuffer bodies_render_data
			glGenFramebuffers(1, &fbo_);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,	GL_RENDERBUFFER, rbo_normal_);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth_);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
		}
	}
	void postRedisplay()
	{
	}
};

#endif

#elif defined(__ANDROID__)

void cvrInit2(const char *args)
{
}

class _CVRDevice
{
public:
    int _id = -1;
    Size _size = Size(-1, -1);
public:
    ~_CVRDevice()
    {
    }
    void release()
    {
    }
    void create(int width, int height, int flags, const std::string &name)
    {
        throw std::string("not implemented");
    }

    void setCurrent()
    {
    }
    void setSize(int width, int height)
    {
    }
    void postRedisplay()
    {
    }
};

#else

#include <EGL/egl.h>

#include <X11/X.h>

static EGLDisplay eglDpy=EGL_NO_DISPLAY;
static EGLConfig eglCfg;

void cvrInit2(const char *args)
{
	//testEGL();
	// 1. Initialize EGL
	eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(eglDpy==EGL_NO_DISPLAY)
	{
#if 1
		std::string dpyName=":0.0";
		if(args)
		{
			ff::ArgSet argSet(args);
			dpyName=argSet.getd<std::string>("display",dpyName);
		}
		Display *dpy=XOpenDisplay(dpyName.empty()? NULL : dpyName.c_str());
		eglDpy=eglGetDisplay((EGLNativeDisplayType)dpy);
#endif
	}
	if (eglDpy == EGL_NO_DISPLAY)
	{
		printf("EGL: can't open display");
		return;
	}

	EGLint major, minor; 

	eglInitialize(eglDpy, &major, &minor);

	// 2. Select an appropriate configuration
	EGLint numConfigs;

	static const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};
	eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);
}

static cv::Size  eglBufSize(0, 0);

bool cvrSetGLContext(Size size)
{
	if(eglDpy==EGL_NO_DISPLAY)
	{
		cvrInit2(NULL);
		if(eglDpy==EGL_NO_DISPLAY)
			return false;
	}
	static EGLSurface eglSurf=NULL;
	static EGLContext eglCtx=NULL;

	if (eglCtx && size.width <= eglBufSize.width && size.height <= eglBufSize.height)
		return true;

	auto dsize=[](int w){
		enum{S=512};
		w=w%S==0? w/S : w/S+1;
		return __max(w,2)*S; //minSize=2*S=1024
	};
	
	size = cv::Size(dsize(size.width), dsize(size.height));

	const EGLint pbufferAttribs[] = {
		EGL_WIDTH, size.width,
		EGL_HEIGHT, size.height,
		EGL_NONE,
	};

	//printf("EGL: set size to (%d,%d)\n",size.width,size.height);

	// 3. Create a surface
	EGLSurface _eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);
	if (!_eglSurf)
	{
		printf("EGL: failed to create surface");
		return false;
	}
	else
	{
		if (eglSurf)
			eglDestroySurface(eglDpy,eglSurf);
		eglSurf = _eglSurf;
	}

	if (!eglCtx)
	{
		// 4. Bind the API
		eglBindAPI(EGL_OPENGL_API);

		// 5. Create a context and make it current
		EGLContext _eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);
		if (!_eglCtx)
		{
			printf("EGL: failed to create context");
			return false;
		}
		else
			eglCtx = _eglCtx;
	}

	eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);
	eglBufSize=size;

	return true;
}

class _CVRDevice
{
public:
	Size _size = Size(-1, -1);
public:
	~_CVRDevice()
	{
		//release();
	}
	void release()
	{
	}
	void create(int width, int height, int flags, const std::string &name)
	{
		//CVR_LOCK();
		//cvrInitGL();
		cvrSetGLContext(Size(width, height));

		_initGL();
		_size = Size(0, 0);
		this->setSize(width, height);
	}

	void setCurrent()
	{

	}
	void setSize(int width, int height)
	{
		Size newSize(width, height);
		if (newSize != _size)
		{
			cvrSetGLContext(Size(width, height));

			glViewport(0, 0, width, height);
			_size = newSize;
		}
	}
	void postRedisplay()
	{
	}
};

#endif


CVRDevice::CVRDevice()
	:_impl(new _CVRDevice)
{
}
CVRDevice::CVRDevice(Size size, int flags, const std::string &name)
	: _impl(new _CVRDevice)
{
	_impl->create(size.width, size.height, flags, name);
}
CVRDevice::~CVRDevice()
{
}

void CVRDevice::create(Size size, int flags, const std::string &name)
{
	_impl->create(size.width, size.height, flags, name);
}

Size CVRDevice::size() const
{
	return _impl->_size;
}
bool CVRDevice::empty() const
{
	return _impl->_size.width <= 0;
}
void CVRDevice::setCurrent()
{
	_impl->setCurrent();
}

void CVRDevice::setSize(int width, int height)
{
	_impl->setSize(width, height);
}
void CVRDevice::postRedisplay()
{
	_impl->postRedisplay();
}

static std::thread::id  g_glThreadID;
static std::list<GLEvent*>  g_glEvents;
static std::mutex g_glEventsMutex;
static std::condition_variable g_glWaitCV;
CVRDevice  theDevice;

//struct _WaitNoEvents
//{
//	~_WaitNoEvents()
//	{
//		cvrWaitFinish();
//	}
//};

//wait no events when program exit
//_WaitNoEvents _g_waitNoEvents; 

static void glEventsProc()
{
#ifdef __ANDROID__
    return;
#endif
	g_glThreadID = std::this_thread::get_id();

	//create the device in the background thread
	theDevice = CVRDevice(Size(10, 10));

	try{
		while (true)
		{
			GLEvent *evt = nullptr;
			int nLeft = 0;
			{
				std::lock_guard<std::mutex> lock(g_glEventsMutex);

				if (!g_glEvents.empty())
				{
					evt = g_glEvents.front();
					g_glEvents.pop_front();
					nLeft = (int)g_glEvents.size();
				}
			}

			if (!evt)
			{
				g_glWaitCV.notify_all();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			else
			{
				evt->exec(nLeft);
				evt->release();
			}
		}
	}
	catch(...)
	{}
} 

void cvrWaitFinish()
{
	std::mutex  waitMutex;
	std::unique_lock<std::mutex> lock(waitMutex);
	g_glWaitCV.wait(lock);
}

static void _atExit()
{
	//printf("_atExit\n");
	cvrWaitFinish();
}

static std::mutex _postMutex;

void cvrPostEvent(GLEvent *evt, bool wait)
{
	std::lock_guard<std::mutex> _lock(_postMutex);

	static bool threadStarted = false;
	if (!threadStarted)
	{
		std::lock_guard<std::mutex> lock(g_glEventsMutex);

		if (!threadStarted)
		{
			std::thread t(glEventsProc);
			t.detach();
			threadStarted = true;

			if (g_atexitFP)
				g_atexitFP(_atExit);
			/*else
				std::atexit(_atExit);*/
		}
	}
	//if posted from the GL thread itself and required to wait finish, execute directly without post event
	if (wait && std::this_thread::get_id() == g_glThreadID)
	{
		evt->exec((int)g_glEvents.size());
		evt->release();
	}
	else
	{
		{
			std::lock_guard<std::mutex> lock(g_glEventsMutex);

			g_glEvents.push_back(evt);
		}
		if (wait)
			cvrWaitFinish();
	}
}

//=================================================================


/* ---------------------------------------------------------------------------- */
//
//void get_bounding_box_for_node(const aiScene *scene, const  aiNode* nd,
//	aiVector3D* min,
//	aiVector3D* max,
//	aiMatrix4x4* trafo
//) {
//	aiMatrix4x4 prev;
//	unsigned int n = 0, t;
//
//	prev = *trafo;
//	aiMultiplyMatrix4(trafo, &nd->mTransformation);
//
//	for (; n < nd->mNumMeshes; ++n) {
//		const  aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
//		for (t = 0; t < mesh->mNumVertices; ++t) {
//
//			aiVector3D tmp = mesh->mVertices[t];
//			aiTransformVecByMatrix4(&tmp, trafo);
//
//			min->x = __min(min->x, tmp.x);
//			min->y = __min(min->y, tmp.y);
//			min->z = __min(min->z, tmp.z);
//
//			max->x = __max(max->x, tmp.x);
//			max->y = __max(max->y, tmp.y);
//			max->z = __max(max->z, tmp.z);
//		}
//	}
//
//	for (n = 0; n < nd->mNumChildren; ++n) {
//		get_bounding_box_for_node(scene, nd->mChildren[n], min, max, trafo);
//	}
//	*trafo = prev;
//}

/* ---------------------------------------------------------------------------- */


void makeSizePower2(Mat &img)
{
	int width = img.cols, height = img.rows;

	int xSize2 = width;
	int ySize2 = height;

	{
		double xPow2 = log((double)xSize2) / log(2.0);
		double yPow2 = log((double)ySize2) / log(2.0);

		int ixPow2 = int(xPow2 + 0.5);
		int iyPow2 = int(yPow2 + 0.5);

		if (xPow2 != (double)ixPow2)
			ixPow2++;
		if (yPow2 != (double)iyPow2)
			iyPow2++;

		xSize2 = 1 << ixPow2;
		ySize2 = 1 << iyPow2;
	}
	if (xSize2 != width || ySize2 != height)
		resize(img, img, Size(xSize2, ySize2));
}


GLuint loadGLTexture(const Mat3b &_img)
{
#ifndef __ANDROID__
	Mat3b img;
	flip(_img, img, 0);

	GLuint texID;
	glGenTextures(1, &texID);

	//printf("texID=%d, width=%d, height=%d\n", texID, t.img.cols,t.img.rows);

	// Binding of texture name
	glBindTexture(GL_TEXTURE_2D, texID);
	// redefine standard texture values
	// We will use linear interpolation for magnification filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// We will use linear interpolation for minifying filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Texture specification
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols,
		img.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);
	// we also want to be able to deal with odd texture dimensions
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
#else
    return 0;
#endif
}

//
//static void loadGLTextures(const std::vector<TexImage> &vTexImages, TexMap &mTex)
//{
//	mTex.clear();
//	for (auto &t : vTexImages)
//	{
//		if (!t.img.empty()) /* If no error occurred: */
//		{
//			mTex[t.file].texID = loadGLTexture(t.img);
//		}
//	}
//}
//
//void _CVRModel::_loadTextures()
//{
//	if (this->_texNotLoaded())
//	{
//		loadGLTextures(vTex, _texMap);
//
//		checkGLError();
//	}
//}
//
//_Texture& _CVRModel::_getTexture(const CVRScene::Texture &tex)
//{
//	auto itr = _texMap.find(tex.name);
//	if (itr == _texMap.end())
//	{
//		cv::Mat img = tex.image;
//		if (img.empty())
//		{
//			std::string imgFile = tex.fullPath;
//			if (imgFile.empty())
//				imgFile = ff::GetDirectory(this->scene->getModelFile()) + tex.name;
//
//			img = cv::imread(imgFile, cv::IMREAD_UNCHANGED);
//			if (img.empty())
//				printf("error: failed to load %s\n", imgFile.c_str());
//		}
//		makeSizePower2(img);
//
//		GLuint texID = loadGLTexture(img);
//		_texMap[tex.name].texID = texID;
//		itr = _texMap.find(tex.name);
//	}
//	return itr->second;
//}
//
//
//void _CVRModel::render_node(CVRScene *scene, CVRScene::Node *node, const Matx44f &mT, _CVRModel *site, int flags)
//{
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glMultMatrixf(mT.val);
//
//	for (auto mi : node->meshes)
//	{
//		auto meshPtr = scene->_meshes[mi];
//
//		glBindTexture(GL_TEXTURE_2D, 0);
//
//		auto &mat = scene->_materials[meshPtr->materialIndex];
//		bool hasTexture = false;
//		if ((flags&CVRM_ENABLE_TEXTURE) && !mat->textures.empty())
//		{
//			//auto &t = mTex[mat->textures.front().path];
//			auto &t = site->_getTexture(mat->textures.front());
//			glBindTexture(GL_TEXTURE_2D, t.texID);
//			hasTexture = true;
//		}
//
//		bool onlyTexture = hasTexture && (flags&CVRM_TEXTURE_NOLIGHTING);
//
//		if (!meshPtr->normals.empty() && (flags&CVRM_ENABLE_LIGHTING) && !onlyTexture)
//			glEnable(GL_LIGHTING);
//		else
//			glDisable(GL_LIGHTING);
//
//		const Point3f *normals = meshPtr->normals.empty() ? nullptr : &meshPtr->normals[0];
//		const Point3f *texCoords = (flags&CVRM_ENABLE_TEXTURE) && !meshPtr->textureCoords.empty() ? &meshPtr->textureCoords[0] : nullptr;
//		const Vec4f   *colors = meshPtr->colors.empty() ? nullptr : &meshPtr->colors[0];
//
//		if (colors)
//			glEnable(GL_COLOR_MATERIAL);
//		else
//			glDisable(GL_COLOR_MATERIAL);
//
//		for (auto &faceType : meshPtr->faces)
//		{
//			GLenum face_mode;
//
//			switch (faceType.numVertices)
//			{
//			case 1: face_mode = GL_POINTS; break;
//			case 2: face_mode = GL_LINES; break;
//			case 3: face_mode = GL_TRIANGLES; break;
//			case 4: face_mode = GL_QUADS; break;
//			default: face_mode = GL_POLYGON; break;
//			}
//
//			const int *v = &faceType.indices[0];
//			int nFaces = (int)faceType.indices.size() / faceType.numVertices;
//
//			for (int f = 0; f < nFaces; ++f, v += faceType.numVertices)
//			{
//				glBegin(face_mode);
//
//				for (int i = 0; i < faceType.numVertices; i++)		// go through all vertices in face
//				{
//					int vertexIndex = v[i];	// get group index for current index
//
//					if (onlyTexture)
//						glColor4f(1, 1, 1, 1);
//					else
//					{
//						if (colors)
//							glColor4fv(&colors[vertexIndex][0]);
//
//						if (normals)
//							glNormal3fv(&normals[vertexIndex].x);
//					}
//
//					if ((flags&CVRM_ENABLE_TEXTURE) && !meshPtr->textureCoords.empty())		//HasTextureCoords(texture_coordinates_set)
//					{
//						glTexCoord2f(texCoords[vertexIndex].x, 1.0f - texCoords[vertexIndex].y); //mTextureCoords[channel][vertex]
//					}
//
//					glVertex3fv(&meshPtr->vertices[vertexIndex].x);
//				}
//				glEnd();
//			}
//		}
//	}
//
//	glPopMatrix();
//}
//
//void _CVRModel::render(int flags)
//{
//	//_initGL();
//#if 1
//	if (_sceneList == 0)
//	{
//		_sceneList = glGenLists(1);
//		_sceneListRenderFlags = flags - 1;//make them unequal to trigger re-compile
//	}
//
//	if (_sceneListRenderFlags != flags || _sceneListVersion != scene->getUpdateVersion())
//	{
//		glNewList(_sceneList, GL_COMPILE);
//		/* now begin at the root node of the imported data and traverse
//		the scenegraph by multiplying subsequent local transforms
//		together on GL's matrix stack. */
//		scene->forAllNodes([this, flags](CVRScene::Node *node, Matx44f &mT) {
//			render_node(scene.get(), node, mT, this, flags);
//		});
//		glEndList();
//
//		_sceneListRenderFlags = flags;
//		_sceneListVersion = scene->getUpdateVersion();
//	}
//
//	glCallList(_sceneList);
//
//	checkGLError();
//#else
//	scene->forAllNodes([this, flags](CVRScene::Node *node, Matx44f &mT) {
//		render_node(scene.get(), node, mT, this->_texMap, flags);
//	});
//#endif
//}

//#if defined(USE_ASSIMP)
//#include"_cvmodel_assimp.h"
//#elif defined(USE_VCGLIB)
//#include"_cvmodel_vcglib.h"
//#elif defined(USE_CVRSCENE)
//#include"_cvmodel_cvrscene.h"
//#endif


//==========================================================
#if 0
struct ShowImageEvent
{
	CVRShowModelBase *winData;
	CVRResult  result;
};
static std::list<ShowImageEvent>  g_showImageEvents;
static std::mutex				  g_showImageMutex;

void showImageProc()
{
	while (true)
	{
		ShowImageEvent evt;
		evt.winData = nullptr;

		{
			std::lock_guard<std::mutex> lock(g_showImageMutex);

			if (!g_showImageEvents.empty())
			{
				evt = g_showImageEvents.front();
				g_showImageEvents.pop_front();
			}
		}

		if (evt.winData)
		{
			auto wd = evt.winData;

			if (wd->resultFilter)
				wd->resultFilter->exec(evt.result);
			//if(!evt.result.img.empty())
			wd->present(evt.result);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
	}
}

void postShowImage(CVRShowModelBase *winData)
{
	static bool showImageThreadStarted = false;
	if (!showImageThreadStarted)
	{
		std::thread t(showImageProc);
		t.detach();
		showImageThreadStarted = true;
	}
	{
		std::lock_guard<std::mutex> lock(g_showImageMutex);
		//g_showImageEvents.push_back({ winData, winData->currentResult });
	}
}
#endif

