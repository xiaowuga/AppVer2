#pragma once

#include"cvrbase.h"
#include"cvrm.h"
//
//class _CVR_API CVRMesh
//{
//public:
//	std::vector<char>           verticesMask;
//	std::vector<cv::Vec3f>		vertices;
//	std::vector<cv::Vec3f>      normals;
//
//	struct Faces
//	{
//		int  numVertices;
//		std::vector<int>   indices;
//	};
//
//	std::vector<Faces>		faces;
//	std::vector<std::string>   texFiles;
//public:
//	void clear()
//	{
//		verticesMask.clear();
//		vertices.clear();
//		normals.clear();
//		faces.clear();
//		texFiles.clear();
//	}
//};

class _CVR_API CVRRendable
{
private:
	bool  _visible = true;
public:
	virtual void render(const Matx44f &sceneModelView, int flags) = 0;

	virtual void setVisible(bool visible);

	bool  isVisible() const
	{
		return _visible;
	}

	virtual ~CVRRendable();
};

class _CVR_API CVRModel
	:public CVRRendable
{
public:
	class _CVR_API  Mesh
	{
	public:
		std::vector<char>               verticesMask; //mask whether the vertex is used
		std::vector<cv::Point3f>		vertices;
		std::vector<cv::Point3f>        normals;
		std::vector<cv::Vec4f>          colors;
		
		std::vector<cv::Point2f> textureCoords;
		int                         materialIndex;

		struct FaceType
		{
			int  numVertices;
			std::vector<int>   indices;
		};

		std::vector<FaceType>		faces; //indexed by numVertices of faces
	public:
		FaceType* queryFaces(int numOfFaceVertices)
		{
			/*if (size_t(numOfFaceVertices) >= faces.size())
				return nullptr;
			CV_Assert(faces[numOfFaceVertices].numVertices == numOfFaceVertices);
			return &this->faces[numOfFaceVertices];*/
			for (auto& ft : faces)
				if (ft.numVertices == numOfFaceVertices)
					return &ft;
			return nullptr;
		}
		//return the number of vertices removed
		int  clearMask(bool removeUnmaskedVertices = true);
	};

	typedef std::shared_ptr<Mesh>  MeshPtr;

	class _CVR_API Texture
	{
	public:
		std::string  name;
		std::string  fullPath;
		cv::Mat      image;
	};

	class _CVR_API Material
	{
	public:
		std::vector<Texture>   textures;
	};
	typedef std::shared_ptr<Material> MaterialPtr;


	class Node;
	typedef std::shared_ptr<Node>  NodePtr;

	class _CVR_API Node
	{
	public:
		std::string           name;

		cv::Matx44f           transformation;
		std::vector<int>      meshes;

		std::vector<NodePtr>  children;
	};

	class _CVR_API Infos
	{
	public:
		std::vector<Point3f>  vertices; //collections of all vertices;

		Point3f   center;
		Point3f   bboxMin, bboxMax;
	};
	typedef std::shared_ptr<Infos>  InfosPtr;

	class _CVR_API This
	{
	public:
		std::string     modelFile;

		std::vector<MeshPtr>  meshes;
		std::vector<MaterialPtr>  materials;

		NodePtr  root;

		InfosPtr  infosPtr;
		uint      infosUpdateVersion = 0;

		uint      updateVersion = 0;
	public:
		This() = default;

		This(const This& r);

		This& operator=(const This&) = delete;

		MeshPtr getSingleMesh(bool createIfNotExist);
	};
protected:
	std::shared_ptr<This>   _this;
	
	class _Render;
	friend class _Render;
	friend class _SceneImpl;
	std::shared_ptr<_Render> _render;

protected:
	template<typename _OpT>
	static void _forAllNodes(Node* node, const cv::Matx44f &prevT, _OpT &op)
	{
		cv::Matx44f curT = prevT*node->transformation;

		op(node, curT);

		for (auto &c : node->children)
			_forAllNodes(c.get(), curT, op);
	}
	template<typename _OpT>
	static void _forAllNodes(Node *node, _OpT op) {
		cv::Matx44f T = cv::Matx44f::eye();
		_forAllNodes(node, T, op);
	}
	std::string _newTextureName(const std::string &nameBase);
public:
	CVRModel();

	CVRModel(const std::string &file, int postProLevel = 3, const std::string &options = "")
		:CVRModel()
	{
		this->load(file, postProLevel, options);
	}
	CVRModel clone();
	
	void clear();

	/* load model file, the supported formats include .obj, .3ds, .ply, .stl, .dae ... (see https://github.com/assimp/assimp)

	The @postProLevel (0-3) specifies the post-processing levels for optimizing the mesh for rendering. 0 means no post processing.
	*/
	void load(const std::string &file, int postProLevel = 3, const std::string &options = "");

	/* save the model to a file, any file format that supported by Assimp can be used, such as .obj, .3ds, .ply, .stl, .dae ,...
	@fmtID : a string to specify the file format, the file extension will be used if is empty.
	@options : additional options, use "-std" to rename the texture image files in a standard way.
	*/
	void save(const std::string &file, const std::string &options = "-std");


	bool empty() const
	{
		return !_this->root;
	}
	operator bool() const
	{
		return !empty();
	}

	This* getData()
	{
		return _this.get();
	}
	std::vector<MeshPtr>& getMeshes()
	{
		return _this->meshes;
	}
	MeshPtr  getSingleMesh(bool createIfNotExist=false)
	{
		//CV_Assert(_this->meshes.size() == 1);
		return _this->getSingleMesh(createIfNotExist);
	}
	int  nMeshes() const
	{
		return (int)_this->meshes.size();
	}
	int clearVerticesMask(bool removeUnmaskedVertices = true)
	{
		int nRemoved = 0;
		for (auto &m : _this->meshes)
			nRemoved += m->clearMask(removeUnmaskedVertices);
		return nRemoved;
	}
	const Infos& getInfos() const;

	uint  getUpdateVersion() const
	{
		return _this->updateVersion;
	}

	const std::string& getFile() const
	{
		return _this->modelFile;
	}

	void addNodes(NodePtr root, std::vector<MeshPtr>  &meshes, std::vector<MaterialPtr> &materials);

	void addNodes(const std::string &name, MeshPtr mesh, MaterialPtr material, const cv::Matx44f &mT = cv::Matx44f::eye());

	void addQuad(const cv::Point3f points[4], const cv::Mat &texImage, const std::string &name);

	void addCuboid(const std::string &name, const cv::Vec3f &size_xyz, const std::vector<cv::Mat> &texImages, const std::string &options="",  const cv::Matx44f &mT=cv::Matx44f::eye());

	cv::Matx44f calcStdPose();

	cv::Matx44f getSceneTransformation() const
	{ 
		return _this->root ? _this->root->transformation : cv::Matx44f::eye();
	}

	void setSceneTransformation(const cv::Matx44f &mT, bool multiplyCurrent = true);

	void setUniformColor(const cv::Vec4f &color);

	template<typename _OpT>
	void forAllNodes(_OpT op)  const {
		_forAllNodes(_this->root.get(), op);
	}

	template<typename _OpT>
	void forAllNodeMeshes(_OpT op)  const {
		this->forAllNodes(
			[&op,this](const Node *node, const cv::Matx44f &mT) {
			for (auto mi : node->meshes)
			{
				op(_this->meshes[mi].get(), mT);
			}
		}
		);
	}
	template<typename _OpT>
	void forAllVertices(_OpT op) const {
		this->forAllNodeMeshes(
			[&op, this](const Mesh *mesh, const cv::Matx44f &mT) {
			for (size_t i = 0; i < mesh->vertices.size(); ++i)
			{
				//if (mesh->verticesMask.empty()||mesh->verticesMask[i])
				{
					cv::Point3f v = mesh->vertices[i] * mT;
					op(v);
				}
			}
		}
		);
	}

public:
	//get a transformation that transform the model to a standard position and size (centered at the origin and scale to fit in a unit cube)
	static Matx44f getUnitize(const cv::Vec3f &center, const cv::Vec3f &bbMin, const cv::Vec3f &bbMax);

	Matx44f getUnitize() const;

	cv::Vec3f getCenter()  const
	{
		return (cv::Vec3f)this->getInfos().center;
	}

	const std::vector<cv::Point3f>& getVertices() const
	{
		return this->getInfos().vertices;
	}

	void    getBoundingBox(cv::Vec3f &cMin, cv::Vec3f &cMax) const
	{
		auto &infos = this->getInfos();
		cMin = infos.bboxMin;
		cMax = infos.bboxMax;
	}

	cv::Vec<float, 6> __getBoundingBox()  const
	{
		cv::Vec3f bb[2];
		this->getBoundingBox(bb[0], bb[1]);
		return reinterpret_cast<cv::Vec<float, 6>&>(bb);
	}

	std::vector<cv::Point3f>  getBoundingBoxCorners()  const
	{
		using cv::Point3f;
		cv::Vec3f cMin, cMax;
		this->getBoundingBox(cMin, cMax);
		return{
			Point3f(cMin[0],cMin[1],cMin[2]), Point3f(cMax[0],cMin[1],cMin[2]), Point3f(cMax[0],cMax[1],cMin[2]), Point3f(cMin[0],cMax[1],cMin[2]),
			Point3f(cMin[0],cMin[1],cMax[2]), Point3f(cMax[0],cMin[1],cMax[2]), Point3f(cMax[0],cMax[1],cMax[2]), Point3f(cMin[0],cMax[1],cMax[2]),
		};
	}

	cv::Vec3f  getSizeBB() const
	{
		cv::Vec3f cMin, cMax;
		this->getBoundingBox(cMin, cMax);
		return cMax - cMin;
	}

	

	/*set a transformation to the model that place it in a standard way
	this transformation will be applied before the transformations in CVRMats
	i.e., it acts as a change to the coordinates of model vertices.
	*/
	void    transform(const Matx44f &trans)
	{
		this->setSceneTransformation(trans, true);
	}

	void applyAndClearTransforms();

	virtual void render(const Matx44f &sceneModelView, int flags);
};




template<typename _ValT>
class CVRArray
	:public CVRRendable
{
protected:
	typedef std::vector<_ValT> _CtrT;

	_CtrT  _v;
public:
	typedef _ValT value_type;
	typedef typename _CtrT::iterator iterator;
	typedef typename _CtrT::const_iterator const_iterator;
public:
	CVRArray()
	{}
	CVRArray(size_t size)
		:_v(size)
	{
	}
	void resize(size_t size)
	{
		_v.resize(size);
	}
	void push_back(const _ValT &val)
	{
		_v.push_back(val);
	}
	size_t size() const
	{
		return _v.size();
	}
	_ValT& operator[](int i)
	{
		return _v[i];
	}
	const _ValT& operator[](int i) const
	{
		return _v[i];
	}
	iterator begin()
	{
		return _v.begin();
	}
	const_iterator begin() const
	{
		return _v.begin();
	}
	iterator end()
	{
		return _v.end();
	}
	const_iterator end() const
	{
		return _v.end();
	}
};

class _CVR_API CVRModelEx
	:public CVRRendable
{
public:
	CVRModel   model;
	Matx44f    mModeli;
	Matx44f    mModel;
public:
	CVRModelEx()
		:mModeli(cvrm::I()), mModel(cvrm::I())
	{}
	CVRModelEx(const CVRModel &_model, const Matx44f &_mModeli = cvrm::I(), const Matx44f &_mModel = cvrm::I());

	virtual void render(const Matx44f &sceneModelView, int flags);
};

class _CVR_API CVRModelArray
	:public CVRArray<CVRModelEx>
{
public:
	CVRModelArray()
	{}
	CVRModelArray(size_t size)
		:CVRArray<CVRModelEx>(size)
	{
	}
	virtual void render(const Matx44f &sceneModelView, int flags);
};

typedef std::shared_ptr<CVRRendable>  CVRRendablePtr;

class _CVR_API CVRRendableArray
	:public CVRArray<CVRRendablePtr>
{
public:
	CVRRendableArray()
	{}
	CVRRendableArray(size_t size)
		:CVRArray<CVRRendablePtr>(size)
	{
	}
	virtual void render(const Matx44f &sceneModelView, int flags);
};


void saveModelFileAs(const std::string &inFile, const std::string &outFile, const Matx44f *mTransform=nullptr);
