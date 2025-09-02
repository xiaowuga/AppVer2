
#include"_cvrender.h"

#include"cvrmodel.h"

#include "assimp/scene.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include"assimp/Exporter.hpp"

#include"BFC/err.h"
#include"BFC/stdf.h"
#include"BFC/portable.h"
#include"BFC/argv.h"
using namespace cv;


#include<iostream>

#include"./third-party/tinyply/tinyply.h"
#include"./third-party/tinyply/example-utils.hpp"
using namespace tinyply;

template<typename _ValT>
void readPlyData(PlyData &data, std::vector<_ValT> &vec)
{
	const size_t numVerticesBytes = data.buffer.size_bytes();
	CV_Assert(numVerticesBytes == sizeof(_ValT) * data.count);
	vec.resize(data.count);
	if (numVerticesBytes>0)
		std::memcpy(&vec[0], data.buffer.get(), numVerticesBytes);
}

void readPlyData(PlyData& data, std::vector<Point3f>& vec)
{
	const size_t numVerticesBytes = data.buffer.size_bytes();
	if (data.t == Type::FLOAT32)
	{
		CV_Assert(numVerticesBytes == sizeof(Point3f) * data.count);
		vec.resize(data.count);
		if (numVerticesBytes > 0)
			std::memcpy(&vec[0], data.buffer.get(), numVerticesBytes);
	}
	else
	{
		std::vector<Point3d> tvec;
		CV_Assert(numVerticesBytes == sizeof(Point3d) * data.count);
		tvec.resize(data.count);
		if (numVerticesBytes > 0)
			std::memcpy(&tvec[0], data.buffer.get(), numVerticesBytes);
		vec.resize(tvec.size());
		for (size_t i = 0; i < tvec.size(); ++i)
			vec[i] = Point3f(tvec[i]);
	}
}

void read_ply_file(const std::string & filepath, const bool preload_into_memory = true)
{
	std::cout << "........................................................................\n";
	std::cout << "Now Reading: " << filepath << std::endl;

	std::unique_ptr<std::istream> file_stream;
	std::vector<uint8_t> byte_buffer;

	try
	{
		// For most files < 1gb, pre-loading the entire file upfront and wrapping it into a 
		// stream is a net win for parsing speed, about 40% faster. 
		if (preload_into_memory)
		{
			byte_buffer = read_file_binary(filepath);
			file_stream.reset(new memory_stream((char*)byte_buffer.data(), byte_buffer.size()));
		}
		else
		{
			file_stream.reset(new std::ifstream(filepath, std::ios::binary));
		}

		if (!file_stream || file_stream->fail()) throw std::runtime_error("file_stream failed to open " + filepath);

		file_stream->seekg(0, std::ios::end);
		const float size_mb = file_stream->tellg() * float(1e-6);
		file_stream->seekg(0, std::ios::beg);

		PlyFile file;
		file.parse_header(*file_stream);

		std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
		for (const auto & c : file.get_comments()) std::cout << "\t[ply_header] Comment: " << c << std::endl;
		for (const auto & c : file.get_info()) std::cout << "\t[ply_header] Info: " << c << std::endl;

		for (const auto & e : file.get_elements())
		{
			std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
			for (const auto & p : e.properties)
			{
				std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
				if (p.isList) std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
				std::cout << std::endl;
			}
		}

		// Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers. 
		// See examples below on how to marry your own application-specific data structures with this one. 
		std::shared_ptr<PlyData> vertices, normals, colors, texcoords, faces, tripstrip;

		// The header information can be used to programmatically extract properties on elements
		// known to exist in the header prior to reading the data. For brevity of this sample, properties 
		// like vertex position are hard-coded: 
		try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { colors = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { colors = file.request_properties_from_element("vertex", { "r", "g", "b", "a" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		// Providing a list size hint (the last argument) is a 2x performance improvement. If you have 
		// arbitrary ply files, it is best to leave this 0. 
		try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		// Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
		// are specifically in the file, which is unlikely); 
		try { tripstrip = file.request_properties_from_element("tristrips", { "vertex_indices" }, 0); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		manual_timer read_timer;

		read_timer.start();
		file.read(*file_stream);
		read_timer.stop();

		const float parsing_time = static_cast<float>(read_timer.get()) / 1000.f;
		std::cout << "\tparsing " << size_mb << "mb in " << parsing_time << " seconds [" << (size_mb / parsing_time) << " MBps]" << std::endl;

		if (vertices)   std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
		if (normals)    std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
		if (colors)     std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
		if (texcoords)  std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
		if (faces)      std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
		if (tripstrip)  std::cout << "\tRead " << (tripstrip->buffer.size_bytes() / tinyply::PropertyTable[tripstrip->t].stride) << " total indices (tristrip) " << std::endl;

		// Example One: converting to your own application types
		{
			const size_t numVerticesBytes = vertices->buffer.size_bytes();
			std::vector<float3> verts(vertices->count);
			std::memcpy(verts.data(), vertices->buffer.get(), numVerticesBytes);
		}

		// Example Two: converting to your own application type
		{
			std::vector<float3> verts_floats;
			std::vector<double3> verts_doubles;
			if (vertices->t == tinyply::Type::FLOAT32) { /* as floats ... */ }
			if (vertices->t == tinyply::Type::FLOAT64) { /* as doubles ... */ }
		}
	}
	catch (const std::exception & e)
	{
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
	}
}

void write_ply_example(const std::string& filename)
{
	geometry cube = make_cube_geometry();

	std::filebuf fb_binary;
	fb_binary.open(filename + "-binary.ply", std::ios::out | std::ios::binary);
	std::ostream outstream_binary(&fb_binary);
	if (outstream_binary.fail()) throw std::runtime_error("failed to open " + filename);

	std::filebuf fb_ascii;
	fb_ascii.open(filename + "-ascii.ply", std::ios::out);
	std::ostream outstream_ascii(&fb_ascii);
	if (outstream_ascii.fail()) throw std::runtime_error("failed to open " + filename);

	PlyFile cube_file;

	cube_file.add_properties_to_element("vertex", { "x", "y", "z" },
		Type::FLOAT32, cube.vertices.size(), reinterpret_cast<uint8_t*>(cube.vertices.data()), Type::INVALID, 0);

	cube_file.add_properties_to_element("vertex", { "nx", "ny", "nz" },
		Type::FLOAT32, cube.normals.size(), reinterpret_cast<uint8_t*>(cube.normals.data()), Type::INVALID, 0);

	cube_file.add_properties_to_element("vertex", { "u", "v" },
		Type::FLOAT32, cube.texcoords.size(), reinterpret_cast<uint8_t*>(cube.texcoords.data()), Type::INVALID, 0);

	cube_file.add_properties_to_element("face", { "vertex_indices" },
		Type::UINT32, cube.triangles.size(), reinterpret_cast<uint8_t*>(cube.triangles.data()), Type::UINT8, 3);

	cube_file.get_comments().push_back("generated by tinyply 2.3");

	// Write an ASCII file
	cube_file.write(outstream_ascii, false);

	// Write a binary file
	cube_file.write(outstream_binary, true);
}

template<typename _ValT>
void _applyMask(std::vector<_ValT> &v, const std::vector<char> &mask)
{
	if (!v.empty())
	{
		CV_Assert(v.size() == mask.size());
		
		int n = 0;
		for (size_t i = 0; i < mask.size(); ++i)
			if (mask[i])
				v[n++] = v[i];
		v.resize(n);
	}
}

int CVRModel::Mesh::clearMask(bool removeUnmaskedVertices)
{
	int nRemoved = 0;

	if (removeUnmaskedVertices && !this->verticesMask.empty())
	{
		_applyMask(this->vertices, this->verticesMask);
		_applyMask(this->normals, this->verticesMask);
		_applyMask(this->colors, this->verticesMask);
		_applyMask(this->textureCoords, this->verticesMask);

		std::vector<int>  remap(this->vertices.size(),-1);
		{
			int n = 0;
			for (size_t i = 0; i < remap.size(); ++i)
				if (this->verticesMask[i])
					remap[i] = n++;
			nRemoved = (int)remap.size() - n;
		}
		for (auto &ft : this->faces)
		{
			const int NV = ft.numVertices;
			CV_Assert(ft.indices.size() % NV == 0);

			if (!ft.indices.empty())
			{
				for (auto &i : ft.indices)
					i = remap[i];

				int n = 0;
				if (NV == 3)
				{
					for (size_t i = 0; i < ft.indices.size(); i += 3)
					{
						const int *f = &ft.indices[i];
						if (f[0] >= 0 && f[1] >= 0 && f[2] >= 0)
						{
							memcpy(&ft.indices[n], f, sizeof(int) * 3);
							n += 3;
						}
					}
				}
				else
				{
					for (size_t i = 0; i < ft.indices.size(); i += NV)
					{
						const int *f = &ft.indices[i];
						int j = 0;
						for (; j < NV; ++j)
							if (f[j] < 0)
								break;
						if (j==NV)
						{
							memcpy(&ft.indices[n], f, sizeof(int) * NV);
							n += NV;
						}
					}
				}
				ft.indices.resize(n);
			}
		}
	}
	this->verticesMask.clear();
	return nRemoved;
}


CVRRendable::~CVRRendable()
{}
void CVRRendable::setVisible(bool visible)
{
	_visible = visible;
}

static Matx44f cvtMatx(const aiMatrix4x4 &m)
{
	Matx44f dm;
	static_assert(sizeof(dm) == sizeof(m), "");
	memcpy(&dm, &m, sizeof(dm));
	return dm.t(); //transpose the result
}

class _SceneImpl
	:public CVRModel
{
public:
	static std::vector<MeshPtr> loadMeshes(aiScene *scene)
	{
		std::vector<MeshPtr>  meshes(scene->mNumMeshes);

		for (uint mi = 0; mi < scene->mNumMeshes; ++mi)
		{
			const aiMesh* mesh = scene->mMeshes[mi];

			MeshPtr  dmesh(new Mesh);

			std::unique_ptr<char[]> _vmask(new char[mesh->mNumVertices]);
			char *vmask = _vmask.get();
			memset(vmask, 0, mesh->mNumVertices);

			uint maxFaceIndices = 0;
			for (uint t = 0; t < mesh->mNumFaces; ++t)
			{
				const struct aiFace* face = &mesh->mFaces[t];
				if (face->mNumIndices > maxFaceIndices)
					maxFaceIndices = face->mNumIndices;
			}
			maxFaceIndices += 1;
			std::unique_ptr<int[]> _vmap(new int[maxFaceIndices]);
			int *vmap = _vmap.get();
			memset(vmap, 0xff, sizeof(int)*maxFaceIndices);

			int nFaceType = 0;

			for (uint t = 0; t < mesh->mNumFaces; ++t)
			{
				const struct aiFace* face = &mesh->mFaces[t];
				int faceType = vmap[face->mNumIndices];
				if (faceType == -1)
				{
					faceType = vmap[face->mNumIndices] = nFaceType++;
					dmesh->faces.push_back(Mesh::FaceType());
					dmesh->faces.back().numVertices = (int)face->mNumIndices;
				}
				auto &v = dmesh->faces[faceType].indices;
				for (uint i = 0; i < face->mNumIndices; i++)		// go through all vertices in face
				{
					v.push_back(face->mIndices[i]);
					vmask[face->mIndices[i]] = 1;
				}
			}
			dmesh->vertices.reserve(mesh->mNumVertices);
			for (uint i = 0; i < mesh->mNumVertices; ++i)
			{
				const aiVector3D &v = mesh->mVertices[i];
				dmesh->vertices.push_back(Point3f(v.x, v.y, v.z));
			}
			//if (mesh->mNormals)
			if(mesh->HasNormals())
			{
				dmesh->normals.reserve(mesh->mNumVertices);
				for (uint i = 0; i < mesh->mNumVertices; ++i)
				{
					const aiVector3D &v = mesh->mNormals[i];
					dmesh->normals.push_back(Point3f(v.x, v.y, v.z));
				}
			}
			if (mesh->mNumVertices > 0)
			{
				dmesh->verticesMask.resize(mesh->mNumVertices);
				memcpy(&dmesh->verticesMask[0], vmask, mesh->mNumVertices);
			}

			if (mesh->HasTextureCoords(0))
			{
				//dmesh->textureCoords.resize(1);
				auto &v = dmesh->textureCoords;
				v.resize(mesh->mNumVertices);
				for (uint i = 0; i < mesh->mNumVertices; ++i)
				{
					auto &c = mesh->mTextureCoords[0][i];
					v[i] = Point2f(c.x, c.y);
				}
			}

			if (mesh->mColors[0])
			{
				dmesh->colors.resize(mesh->mNumVertices);
				for (uint i = 0; i < mesh->mNumVertices; ++i)
				{
					auto &c = mesh->mColors[0][i];
					dmesh->colors[i] = cv::Vec4f(c.r, c.g, c.b, c.a);
				}
			}

			dmesh->materialIndex = mesh->mMaterialIndex;

			meshes[mi] = dmesh;
		}
		return meshes;
	}
	static std::vector<MaterialPtr> loadMaterials(aiScene *scene, const std::string &modelDir)
	{
		std::vector<MaterialPtr>  materials(scene->mNumMaterials);

		for (uint mi = 0; mi < scene->mNumMaterials; ++mi)
		{
			MaterialPtr dmat(new Material);
			auto *mtl = scene->mMaterials[mi];

			int nTex;
			if ((nTex = mtl->GetTextureCount(aiTextureType_DIFFUSE)) > 0)
			{
				aiString path;	// filename
				for (int i = 0; i < nTex; ++i)
				{
					if (mtl->GetTexture(aiTextureType_DIFFUSE, i, &path) == AI_SUCCESS)
					{
						Texture tex;
						tex.name = std::string(path.data);
						tex.fullPath = ff::CatDirectory(modelDir, tex.name);
						dmat->textures.push_back(tex);
					}
				}
			}
			materials[mi] = dmat;
		}
		return materials;
	}
	static  NodePtr loadNodes(aiScene *scene, aiNode *node)
	{
		if (!node)
			return NodePtr(nullptr);

		NodePtr dnode(new Node);

		uint numChildren=node->mNumChildren;
		if (numChildren > 0)
		{
			dnode->children.resize(numChildren);
			for (uint i = 0; i < numChildren; ++i)
			{
				dnode->children[i] = loadNodes(scene, node->mChildren[i]);
			}
		}

		dnode->transformation = cvtMatx(node->mTransformation);

		dnode->meshes.reserve(node->mNumMeshes);
		for (uint i = 0; i < node->mNumMeshes; ++i)
			dnode->meshes.push_back(node->mMeshes[i]);

		dnode->name = std::string(node->mName.data);

		return dnode;
	}

	static bool loadAsPlyPointCloud(std::vector<MeshPtr> &meshs, NodePtr &rootNode, const std::string & filepath, const bool preload_into_memory = true)
	{
		std::unique_ptr<std::istream> file_stream;
		std::vector<uint8_t> byte_buffer;
		bool readOK = false;

		try
		{
			// For most files < 1gb, pre-loading the entire file upfront and wrapping it into a 
			// stream is a net win for parsing speed, about 40% faster. 
			if (preload_into_memory)
			{
				byte_buffer = read_file_binary(filepath);
				file_stream.reset(new memory_stream((char*)byte_buffer.data(), byte_buffer.size()));
			}
			else
			{
				file_stream.reset(new std::ifstream(filepath, std::ios::binary));
			}

			if (!file_stream || file_stream->fail()) throw std::runtime_error("file_stream failed to open " + filepath);

			file_stream->seekg(0, std::ios::end);
			const float size_mb = file_stream->tellg() * float(1e-6);
			file_stream->seekg(0, std::ios::beg);

			PlyFile file;
			file.parse_header(*file_stream);

#if 1
			std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
			for (const auto & c : file.get_comments()) std::cout << "\t[ply_header] Comment: " << c << std::endl;
			for (const auto & c : file.get_info()) std::cout << "\t[ply_header] Info: " << c << std::endl;

			for (const auto & e : file.get_elements())
			{
				std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
				for (const auto & p : e.properties)
				{
					std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
					if (p.isList) std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
					std::cout << std::endl;
				}
			}
#endif

			// Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers. 
			// See examples below on how to marry your own application-specific data structures with this one. 
			std::shared_ptr<PlyData> vertices, normals, colors, texcoords, faces, tripstrip;

			// The header information can be used to programmatically extract properties on elements
			// known to exist in the header prior to reading the data. For brevity of this sample, properties 
			// like vertex position are hard-coded: 
			try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
			catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

			try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
			catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

			try { colors = file.request_properties_from_element("vertex", { "red", "green", "blue"}); }
			catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

			try { colors = file.request_properties_from_element("vertex", { "r", "g", "b"}); }
			catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

			file.read(*file_stream);

			MeshPtr meshPtr(new Mesh);
			if(vertices)
				readPlyData(*vertices, meshPtr->vertices);
			if(normals)
				readPlyData(*normals, meshPtr->normals);
			if (colors)
			{
				std::vector<Vec3b>  t;
				readPlyData(*colors, t);
				meshPtr->colors.resize(t.size());
				for (size_t i = 0; i < t.size(); ++i)
					meshPtr->colors[i] = Vec4f(t[i][0]/255.f, t[i][1]/255.f, t[i][2]/255.f, 1.f);
			}

			meshs.clear();
			meshs.push_back(meshPtr);

			rootNode = NodePtr(new Node);
			rootNode->meshes.push_back(0);
			rootNode->transformation = cv::Matx44f::eye();
			readOK = true;
		}
		catch (const std::exception & e)
		{
			std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
		}
		return readOK;
	}
	static void saveAsPly(CVRModel &model, const std::string &file, ff::IArgSet &args)
	{
		CV_Assert(model.getMeshes().size() <= 1);

		if (model.getMeshes().empty())
			return;

		auto &mesh = *model.getSingleMesh();

		PlyFile fdata;

		if(!mesh.vertices.empty())
			fdata.add_properties_to_element("vertex", { "x", "y", "z" },
				Type::FLOAT32, mesh.vertices.size(), reinterpret_cast<uint8_t*>(mesh.vertices.data()), Type::INVALID, 0);

		if(!mesh.normals.empty())
			fdata.add_properties_to_element("vertex", { "nx", "ny", "nz" },
				Type::FLOAT32, mesh.normals.size(), reinterpret_cast<uint8_t*>(mesh.normals.data()), Type::INVALID, 0);

		if (!mesh.colors.empty())
		{
			std::vector<Vec3b> rgb(mesh.colors.size());
			for (size_t i = 0; i < mesh.colors.size(); ++i)
			{
				auto &c = mesh.colors[i];
				rgb[i] = Vec3b(uchar(c[0]*255),uchar(c[1]*255),uchar(c[2]*255));
			}
			fdata.add_properties_to_element("vertex", { "red", "green", "blue"},
				Type::UINT8, rgb.size(), reinterpret_cast<uint8_t*>(rgb.data()), Type::INVALID, 0);
		}
		
		if (!mesh.textureCoords.empty())
		{
			fdata.add_properties_to_element("vertex", { "u", "v" },
				Type::FLOAT32, mesh.textureCoords.size(), reinterpret_cast<uint8_t*>(mesh.textureCoords.data()), Type::INVALID, 0);
		}

		auto *tri = mesh.queryFaces(3);
		if (tri)
		{
			fdata.add_properties_to_element("face", { "vertex_indices" },
				Type::UINT32, tri->indices.size()/3, reinterpret_cast<uint8_t*>(tri->indices.data()), Type::UINT8, 3);
		}

		// 新增：在文件头中添加纹理文件路径作为注释
		//if (!textureFilename.empty()) 
		{
			fdata.get_comments().push_back(std::string("TextureFile ") + "box1.png");
		}


		fdata.get_comments().push_back("generated by tinyply 2.3");

		bool isBinary = !args.getd<bool>("ascii", false);

		std::ofstream os(file, std::ios::out|(isBinary? std::ios::binary : 0));
		fdata.write(os, isBinary);
		if(!os)
			throw std::runtime_error("failed to open " + file);
	}


	static aiScene* mesh_to_assimp_scene(const CVRModel::Mesh& mesh, const std::vector<MaterialPtr>  &materials)
	{
		// 创建Assimp场景
		aiScene* scene = new aiScene();
		scene->mFlags = 0;

		// 创建一个根节点
		scene->mRootNode = new aiNode();
		scene->mRootNode->mName = "RootNode";

		// 创建一个网格
		scene->mNumMeshes = 1;
		scene->mMeshes = new aiMesh * [1];
		scene->mMeshes[0] = new aiMesh();
		scene->mMeshes[0]->mName = "Mesh";

		// 将网格添加到根节点
		scene->mRootNode->mNumMeshes = 1;
		scene->mRootNode->mMeshes = new unsigned int[1];
		scene->mRootNode->mMeshes[0] = 0;

		// 创建一个材质
		scene->mNumMaterials = materials.size();
		if (!materials.empty())
		{
			scene->mMaterials = new aiMaterial * [materials.size()];
			for (size_t i = 0; i < materials.size(); ++i)
			{
				scene->mMaterials[i] = new aiMaterial();

				// 设置基本材质颜色
				aiColor3D diffuseColor(1.0f, 1.0f, 1.0f);
				scene->mMaterials[i]->AddProperty(&diffuseColor, 1, AI_MATKEY_COLOR_DIFFUSE);

				auto mptr = materials[i];
				if (mptr && !mptr->textures.empty())
				{
					for (int j = 0; j < mptr->textures.size(); ++j)
					{
						auto& tex = mptr->textures[j];
						aiString path(tex.name.c_str());
						scene->mMaterials[0]->AddProperty(&path, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, j));
					}
				}
			}
		}

		// 将网格与材质关联
		scene->mMeshes[0]->mMaterialIndex = mesh.materialIndex;

		// 设置顶点数量
		size_t vertexCount = mesh.vertices.size();
		scene->mMeshes[0]->mNumVertices = static_cast<unsigned int>(vertexCount);

		// 分配顶点内存
		scene->mMeshes[0]->mVertices = new aiVector3D[vertexCount];
		if (!mesh.normals.empty()) {
			scene->mMeshes[0]->mNormals = new aiVector3D[vertexCount];
		}
		if (!mesh.colors.empty()) {
			scene->mMeshes[0]->mColors[0] = new aiColor4D[vertexCount];
		}
		if (!mesh.textureCoords.empty()) {
			scene->mMeshes[0]->mTextureCoords[0] = new aiVector3D[vertexCount];
		}

		// 复制顶点数据
		for (size_t i = 0; i < vertexCount; ++i) {
			const cv::Point3f& v = mesh.vertices[i];
			scene->mMeshes[0]->mVertices[i] = aiVector3D(v.x, v.y, v.z);

			// 复制法线
			if (!mesh.normals.empty() && i < mesh.normals.size()) {
				const cv::Point3f& n = mesh.normals[i];
				scene->mMeshes[0]->mNormals[i] = aiVector3D(n.x, n.y, n.z);
			}

			// 复制颜色
			if (!mesh.colors.empty() && i < mesh.colors.size()) {
				const cv::Vec4f& c = mesh.colors[i];
				scene->mMeshes[0]->mColors[0][i] = aiColor4D(c[0], c[1], c[2], c[3]);
			}

			// 复制纹理坐标
			if (!mesh.textureCoords.empty() && i < mesh.textureCoords.size()) {
				const cv::Point2f& tc = mesh.textureCoords[i];
				scene->mMeshes[0]->mTextureCoords[0][i] = aiVector3D(tc.x, tc.y, 0.0f);
			}
		}

		// 设置面数据
		size_t faceCount = 0;
		for (auto& ft : mesh.faces)
			faceCount += ft.indices.size() / ft.numVertices;

		scene->mMeshes[0]->mNumFaces = static_cast<unsigned int>(faceCount);
		scene->mMeshes[0]->mFaces = new aiFace[faceCount];

		{
			int fi = 0;
			for (auto& ft : mesh.faces)
			{
				int count = ft.indices.size() / ft.numVertices;
				for (int i = 0; i < count; ++i, ++fi)
				{
					aiFace& aiFace = scene->mMeshes[0]->mFaces[fi];

					aiFace.mNumIndices = static_cast<unsigned int>(ft.numVertices);
					aiFace.mIndices = new unsigned int[aiFace.mNumIndices];

					for (size_t j = 0; j < ft.numVertices; ++j) {
						aiFace.mIndices[j] = static_cast<unsigned int>(ft.indices[i*ft.numVertices+j]);
					}
				}
			}
		}

		return scene;
	}

	static void saveAsPlyAssimp(CVRModel& _model, const std::string& file, ff::IArgSet& args)
	{
#ifndef __ANDROID__
		CVRModel model = _model.clone();
		//clear transforms and merge sub-meshs
		model.applyAndClearTransforms();

		CV_Assert(model.getMeshes().size() <= 1);

		if (model.getMeshes().empty())
			return;

		auto& mesh = *model.getSingleMesh();
		aiScene* scene = mesh_to_assimp_scene(mesh, model._this->materials);

		Assimp::Exporter exporter;
		auto ext=ff::GetFileExtention(file);
		ff::str2lower(ext);
		const char* formatId = ext.c_str();  
		
		auto dir = ff::GetDirectory(file);
		if (!ff::pathExist(dir))
			ff::makeDirectory(dir);

		aiReturn result = exporter.Export(scene, formatId, file);

		if (result != AI_SUCCESS) {
			FF_EXCEPTION1("export mesh failed");
		}
		else 
		{
			for (auto& m : model._this->materials)
			{
				for (auto& t : m->textures)
				{
					auto outfile = dir + "/" + t.name;
					ff::copyFile(t.fullPath, outfile, true);
				}
			}
		}
#endif
	}
};


void CVRModel::load(const std::string &file, int postProLevel, const std::string &options)
{
	std::string modelDir = ff::GetDirectory(ff::getFullPath(file));
	bool loadOK = false;

	if (true)
	{
		uint postPro = postProLevel == 0 ? 0 :
			postProLevel == 1 ? aiProcessPreset_TargetRealtime_Fast :
			postProLevel == 2 ? aiProcessPreset_TargetRealtime_Quality :
			aiProcessPreset_TargetRealtime_MaxQuality;

		aiScene *scene = (aiScene*)aiImportFile(file.c_str(), postPro);
		if (scene)
		{
			this->clear();
			_this->meshes = _SceneImpl::loadMeshes(scene);
			_this->materials = _SceneImpl::loadMaterials(scene, modelDir);
			_this->root = _SceneImpl::loadNodes(scene, scene->mRootNode);

			aiReleaseImport(scene);
			loadOK = true;
		}
	}
	if (!loadOK)
	{
		//read_ply_file(file, true);
		loadOK=_SceneImpl::loadAsPlyPointCloud(_this->meshes, _this->root, file);
		_this->materials.clear();

	}

	if(!loadOK)
		FF_EXCEPTION(ERR_FILE_OPEN_FAILED, file.c_str());

	_this->modelFile = ff::getFullPath(file);
	++_this->updateVersion;
}




void CVRModel::save(const std::string &file, const std::string &options)
{
	std::string ext = ff::GetFileExtention(file);
	ff::str2lower(ext);

	//CV_Assert(ext == "ply"); //currently only ply is surpported
	ff::CommandArgSet args(options);
	_SceneImpl::saveAsPlyAssimp(*this, file, args);
}


const CVRModel::Infos& CVRModel::getInfos() const
{
	if (!_this->infosPtr||_this->infosUpdateVersion!=_this->updateVersion)
	{
		_this->infosPtr = InfosPtr(new Infos);

		size_t nVertices = 0;
		for (auto &m : _this->meshes)
			nVertices += m->vertices.size();
		
		auto &vertices = _this->infosPtr->vertices;
		vertices.reserve(nVertices);
		this->forAllVertices([&vertices](const Point3f &p) {
			vertices.push_back(p);
		});

		if (!vertices.empty())
		{
			Point3f  vmin, vmax;
			vmin = vmax = vertices.front();
			
			auto setmm = [](float &vmin, float &vmax, float x) {
				if (x < vmin)
					vmin = x;
				else if (x > vmax)
					vmax = x;
			};

			for (auto &p : vertices)
			{
				setmm(vmin.x, vmax.x, p.x);
				setmm(vmin.y, vmax.y, p.y);
				setmm(vmin.z, vmax.z, p.z);
			}
			_this->infosPtr->bboxMin = vmin;
			_this->infosPtr->bboxMax = vmax;
			_this->infosPtr->center = (vmax + vmin)*0.5f;
		}
		_this->infosUpdateVersion = _this->updateVersion;
	}
	return *_this->infosPtr;
}


std::string CVRModel::_newTextureName(const std::string &nameBase)
{
	std::vector<std::string>  matched;
	for (auto &m : _this->materials)
	{
		for (auto &t : m->textures)
		{
			if (t.name.size() >= nameBase.size() && strncmp(t.name.c_str(), nameBase.c_str(), nameBase.size()) == 0)
				matched.push_back(t.name);
		}
	}
	if (matched.empty())
		return nameBase + ".png";

	for (int i = 1; ; ++i)
	{
		std::string name = nameBase+ff::StrFormat("_%d.png", i);
		if (std::find(matched.begin(), matched.end(), name) == matched.end())
			return name;
	}
	return "";
}

void CVRModel::addNodes(NodePtr root, std::vector<MeshPtr>  &meshes, std::vector<MaterialPtr> &materials)
{
	if (!_this->root)
	{
		_this->root = root;
		_this->meshes = meshes;
		_this->materials = materials;
		return;
	}

	int offset = (int)this->_this->materials.size();
	_this->materials.insert(_this->materials.end(), materials.begin(), materials.end());
	for (auto &m : meshes)
		m->materialIndex += offset;

	offset = (int)this->_this->meshes.size();
	_this->meshes.insert(_this->meshes.end(), meshes.begin(), meshes.end());
	_forAllNodes(root.get(), [offset](Node *node, const Matx44f &) {
		for (auto &mi : node->meshes)
			mi += offset;
	});

	root->transformation = _this->root->transformation.inv()*root->transformation;
	_this->root->children.push_back(root);
	++_this->updateVersion;
}

void CVRModel::addNodes(const std::string &name, MeshPtr mesh, MaterialPtr material, const cv::Matx44f &mT)
{
	std::vector<MeshPtr> meshes;
	meshes.push_back(mesh);

	std::vector<MaterialPtr> materials;
	materials.push_back(material);

	NodePtr root(new Node);
	root->meshes.push_back(0);
	root->transformation = mT;
	root->name = name;

	this->addNodes(root, meshes, materials);
}

void CVRModel::addQuad(const cv::Point3f points[4], const cv::Mat &texImage, const std::string &name)
{
	Texture tex;
	tex.name = this->_newTextureName(name);
	tex.image = texImage;

	MaterialPtr mat(new Material);
	mat->textures.push_back(tex);

	MeshPtr mesh(new Mesh);
	mesh->vertices.insert(mesh->vertices.end(), points, points + 4);
	mesh->faces.resize(1);
	auto &ft = mesh->faces[0];
	ft.numVertices = 4;
	for (int i = 0; i < 4; ++i)
		ft.indices.push_back(i);

	const float tc[][2] = { { 0.f,0.f },{ 1.f,0.f },{ 1.f,1.f },{ 0.f,1.f } };
	mesh->textureCoords.resize(4);
	for (int i = 0; i < 4; ++i)
		mesh->textureCoords[i] = Point2f(tc[i][0], tc[i][1]);
	mesh->materialIndex = 0;

	//this->addNodes(mesh, mat, name);
	this->addNodes(name, mesh, mat);
}

void CVRModel::addCuboid(const std::string &name, const cv::Vec3f &size, const std::vector<cv::Mat> &texImages, const std::string &options, const cv::Matx44f &mT)
{
	ff::CommandArgSet args(options);

	float dx = size[0] * 0.5f, dy = size[1] * 0.5f, dz = size[2] * 0.5f;
	Point3f corners[] = {
		{dx,dy,dz},{-dx,dy,dz},{-dx,-dy,dz},{dx,-dy,dz},
		{ dx,dy,-dz },{ -dx,dy,-dz },{ -dx,-dy,-dz },{ dx,-dy,-dz },
	};

	int vv[][8] = {
		{3,7,4,0, 2,6,5,1},{ 0,3,7,4, 5,6,2,1 },{3,7,4,0, 6,2,1,5}, //xyz, xzy
		{0,4,5,1, 3,7,6,2},{ 1,0,4,5, 6,7,3,2 },{0,4,5,1, 7,3,2,6}, //yxz, yzx
		{1,2,3,0, 5,6,7,4},{ 0,1,2,3, 7,6,5,4 }, {1,2,3,0, 6,5,4,7},  //zxy, zyx
	};

	struct DSize
	{
		float sz;
		char  axis;
	}
	vsize[3] = { {size[0],'x'},{size[1],'y'},{size[2],'z'} };

	std::string order = args.getd<std::string>("order", "auto");

	if (order == "auto")
	{
		std::sort(vsize, vsize + 3, [](const DSize &a, const DSize &b) {
			return a.sz > b.sz;
		});
	}
	else
	{
		DSize t[3];
		int n = 0;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				if (vsize[j].axis == order[i])
				{
					t[n++] = vsize[j]; vsize[j].axis = -1;
					break;
				}
			}
		}
		CV_Assert(n == 3);//invalid order
		memcpy(vsize, t, sizeof(t));
	}
	//std::swap(vsize[0], vsize[1]);


	char tag[4] = { vsize[0].axis,vsize[1].axis,vsize[2].axis, '\0' };

	struct Config
	{
		const char *tag;
		int  imain, isub;
	}
	vconfig[6] = {
		{"xyz",0,1},{"xzy",0,2},{"yxz",3,4},{"yzx",3,5},{"zxy",6,7},{"zyx",6,8}
	};

	Config cfg;
	for(auto &c : vconfig)
		if (strncmp(c.tag, tag, 3) == 0)
		{
			cfg = c; break;
		}

	auto getEdgeLen = [&corners](int i, int j) {
		Point3f dv = corners[i] - corners[j];
		return sqrt(dv.dot(dv));
	};

	const int *v = vv[cfg.imain];
	float a = getEdgeLen(v[0], v[1]), b = getEdgeLen(v[1], v[2]), c = vsize[1].sz / vsize[0].sz*vsize[2].sz;
	CV_Assert(__min(a, b) == vsize[2].sz && __max(a, b) == vsize[1].sz);

	bool lookInside = args.getd<bool>("lookInside",false);
	auto getImage = [lookInside, &texImages](int i) {
		Mat img = texImages[i];
		if (lookInside)
			flip(img, img, 1);
		return img;
	};

	float texHeight = vsize[0].sz, texWidth = (a + b + c) * 2;
	Mat img = getImage(0);
	if (img.rows > img.cols)
		cv::rotate(img, img, cv::ROTATE_90_COUNTERCLOCKWISE);

	a /= texWidth; b /= texWidth; c /= texWidth;

	int dwidth = int(img.rows*texWidth / texHeight + 0.5);
	if (texImages.size() > 1)
	{
		float vw[] = { a,b,a,b,c,c };
		int j = 5;
		Mat dimg(img.rows, dwidth, CV_8UC3);
		int x = dwidth;
		for (int i = (int)texImages.size() - 1; i >= 1&&j>=0; --i,--j)
		{
			int roiWidth = int(vw[j] * dwidth+0.5);
			Rect roi(x - roiWidth, 0, roiWidth, img.rows);
			Mat curImg = getImage(i);
			CV_Assert(curImg.type() == CV_8UC3);
			if (curImg.rows > curImg.cols && roi.height < roi.width)
				cv::rotate(curImg, curImg, cv::ROTATE_90_COUNTERCLOCKWISE);
			cv::resize(curImg, dimg(roi), roi.size());
			x = roi.x;
		}
		if (x > 0)
		{
			if (img.cols < x)
				cv::resize(img, img, Size(x, img.rows));
			cv::copyMem(img(Rect(0, 0, x, img.rows)), dimg(Rect(0, 0, x, img.rows)));
		}
		img = dimg;
	}
	else
	{
		if (img.cols > dwidth)
			img = img(Rect(0, 0, dwidth, img.rows)).clone();
	}

	int maxTexSize = args.getd<int>("maxTexSize", 2048 * 10);
	if (__max(img.cols, img.rows) > maxTexSize)
	{
		double scale = double(maxTexSize) / __max(img.cols, img.rows);
		img = imscale(img, scale, cv::INTER_LINEAR);
	}

	MeshPtr mesh(new Mesh);
	mesh->materialIndex = 0;

	const int *vx = vv[cfg.imain];
	for (int i = 0; i < 8; ++i)
		mesh->vertices.push_back(corners[vx[i]]);
	
	vx = vv[cfg.isub];
	for (int i = 0; i < 8; ++i)
		mesh->vertices.push_back(corners[vx[i]]);

	mesh->vertices.push_back(mesh->vertices[4]);
	mesh->vertices.push_back(mesh->vertices[0]);
	
	float tc[][2] = {
		{ 0,0 },{ a,0 },{ a + b,0 },{ a + b + a,0 },
		{ 0,1 },{ a,1 },{ a + b, 1 },{ a + b + a, 1 },
		{ (a + b) * 2,0 },{ (a + b) * 2,1.0f },{ (a + b) * 2 + c,1.0f },{ (a + b) * 2 + c,0.f },
		{ (a + b) * 2+c,0.f },{ (a + b) * 2+c,1.f },{ (a + b+c) * 2, 1.f },{ (a + b +c) * 2,0.f },
		{ (a + b) * 2,1.f },{ (a + b) * 2,0 }
	};
	for (int i = 0; i < 18; ++i)
		mesh->textureCoords.push_back(Point2f(tc[i][0], 1.f - tc[i][1]));

	mesh->faces.resize(1);
	auto &ft = mesh->faces[0];
	ft.numVertices = 4;
	ft.indices = { 0,4,5,1, 1,5,6,2, 2,6,7,3, 3,7,16,17,  8,9,10,11, 12,13,14,15 };

	//std::reverse(ft.indices.begin(), ft.indices.end());

	if (args.getd<bool>("notop", false))
	{
		size_t i = 0;
		auto isTop = [&mesh](size_t i) {
			return mesh->vertices[mesh->faces[0].indices[i]].y > 0.f;
		};

		for (; i < ft.indices.size(); i += 4)
		{
			if (isTop(i) && isTop(i + 1) && isTop(i + 2) && isTop(i + 3))
				break;
		}
		CV_Assert(i < ft.indices.size());
		ft.indices.erase(ft.indices.begin() + i, ft.indices.begin() + i + 4);
	}

	Texture tex;
	tex.name = this->_newTextureName(name);
	tex.image = img;

	MaterialPtr mat(new Material);
	mat->textures.push_back(tex);

	this->addNodes(name, mesh, mat, mT);
}

cv::Matx44f CVRModel::calcStdPose()
{
	const std::vector<Point3f>  &vtx = this->getInfos().vertices;

	Mat mvtx(vtx.size(), 3, CV_32FC1, (void*)&vtx[0]);
	cv::PCA pca(mvtx, noArray(), PCA::DATA_AS_ROW);

	Vec3f mean = pca.mean;
	Matx33f ev = pca.eigenvectors;

	//swap x-y so that the vertical directional is the longest dimension
	Vec3f vx(-ev(1, 0), -ev(1, 1), -ev(1, 2));
	Matx44f R = cvrm::rotate(&vx[0], &ev(0, 0), &ev(2, 0));
	if (determinant(R) < 0)
	{//reverse the direction of Z axis
		for (int i = 0; i < 4; ++i)
			R(i, 2) *= -1;
	}
	//std::cout << "det=" << determinant(R) << std::endl;

	return cvrm::translate(-mean[0], -mean[1], -mean[2])*R;
}

void CVRModel::setSceneTransformation(const cv::Matx44f &mT, bool multiplyCurrent)
{
	if (_this->root)
	{
		if (multiplyCurrent)
			_this->root->transformation = _this->root->transformation*mT;
		else
			_this->root->transformation = mT;
		
		++_this->updateVersion;
	}
}

void CVRModel::setUniformColor(const cv::Vec4f &color)
{
	this->forAllNodeMeshes([color](Mesh *mesh, const cv::Matx44f&) {
		mesh->colors.resize(1);
		mesh->colors[0] = color;
	});
}

Matx44f CVRModel::getUnitize(const cv::Vec3f &center, const cv::Vec3f &bbMin, const cv::Vec3f &bbMax)
{
	float tmp = 0;
	for (int i = 0; i < 3; ++i)
		tmp = __max(bbMax[i] - bbMin[i], tmp);
	tmp = 2.f / tmp;

	return cvrm::translate(-center[0], -center[1], -center[2]) * cvrm::scale(tmp, tmp, tmp);
}

Matx44f CVRModel::getUnitize() const
{
	if (!*this)
		return cvrm::I();

	Vec3f bbMin, bbMax;
	this->getBoundingBox(bbMin, bbMax);
	return getUnitize(this->getCenter(), bbMin, bbMax);
}


//====================================================================


struct _Texture
{
	GLuint texID;
};

typedef std::map<std::string, _Texture> TexMap;


class CVRModel::_Render
{
public:
	//CVRModel     *_site=nullptr;
	TexMap       _texMap;
	GLuint		_sceneList = 0;
	int			_sceneListRenderFlags = -1;
	uint        _sceneListVersion = 0;

public:
	~_Render()
	{
		this->clear();
	}
	void clear()
	{
#ifndef __ANDROID__
		if (!_texMap.empty() || _sceneList != 0)
		{
			cvrCall([this](int) {
				for (auto &t : _texMap)
				{
					if (t.second.texID > 0)
						glDeleteTextures(1, &t.second.texID);
				}
				_texMap.clear();


				if (_sceneList != 0)
				{
#ifndef __ANDROID__
					glDeleteLists(_sceneList, 1);
#endif
					_sceneList = 0;
					_sceneListRenderFlags = -1;
				}
			});
			cvrWaitFinish();
		}
#endif
	}

	_Texture& _getTexture(CVRModel  *_site, const CVRModel::Texture &tex)
	{
		auto itr = _texMap.find(tex.name);
		if (itr == _texMap.end())
		{
			cv::Mat img = tex.image;
			if (img.empty())
			{
				std::string imgFile = tex.fullPath;
				if (imgFile.empty())
					imgFile = ff::GetDirectory(_site->getFile()) + tex.name;

				img = cv::imread(imgFile, cv::IMREAD_ANYCOLOR);
				CV_Assert(img.type() == CV_8UC3);
				if (img.empty())
					printf("error: failed to load %s\n", imgFile.c_str());
			}
			CV_Assert(img.type() == CV_8UC3);

			makeSizePower2(img);


			GLuint texID = loadGLTexture(img);
			_texMap[tex.name].texID = texID;
			itr = _texMap.find(tex.name);
		}
		return itr->second;
	}

	void _render_faces(CVRModel  *_site,  MeshPtr meshPtr, int flags)
	{
#ifndef __ANDROID__
		auto *scene = _site->_this.get();
		glBindTexture(GL_TEXTURE_2D, 0);

		auto &mat = scene->materials[meshPtr->materialIndex];
		bool hasTexture = false;
		if ((flags&CVRM_ENABLE_TEXTURE) && !mat->textures.empty())
		{
			//auto &t = mTex[mat->textures.front().path];
			auto &t = this->_getTexture(_site, mat->textures.front());
			glBindTexture(GL_TEXTURE_2D, t.texID);
			hasTexture = true;
		}

		bool onlyTexture = hasTexture && (flags&CVRM_TEXTURE_NOLIGHTING);

		if (!meshPtr->normals.empty() && (flags&CVRM_ENABLE_LIGHTING) && !onlyTexture)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);

		const Point3f *normals = meshPtr->normals.empty() ? nullptr : &meshPtr->normals[0];
		const Point2f *texCoords = (flags&CVRM_ENABLE_TEXTURE) && !meshPtr->textureCoords.empty() ? &meshPtr->textureCoords[0] : nullptr;
		const Vec4f   *colors = meshPtr->colors.empty() || !(flags&CVRM_ENABLE_VERTEX_COLOR) ? nullptr : &meshPtr->colors[0];
		const bool isUniformColor = meshPtr->colors.size() == 1;

		if (colors)
			glEnable(GL_COLOR_MATERIAL);
		else
			glDisable(GL_COLOR_MATERIAL);

		for (auto &faceType : meshPtr->faces)
		{
			GLenum face_mode;

			switch (faceType.numVertices)
			{
			case 1: face_mode = GL_POINTS; break;
			case 2: face_mode = GL_LINES; break;
			case 3: face_mode = GL_TRIANGLES; break;
			case 4: face_mode = GL_QUADS; break;
			default: face_mode = GL_POLYGON; break;
			}

			const int *v = &faceType.indices[0];
			int nFaces = (int)faceType.indices.size() / faceType.numVertices;

			for (int f = 0; f < nFaces; ++f, v += faceType.numVertices)
			{
				glBegin(face_mode);

				for (int i = 0; i < faceType.numVertices; i++)		// go through all vertices in face
				{
					int vertexIndex = v[i];	// get group index for current index

					if (onlyTexture)
						glColor4f(1, 1, 1, 1);
					else
					{
						if (colors)
							glColor4fv(&colors[isUniformColor ? 0 : vertexIndex][0]);

						if (normals)
							glNormal3fv(&normals[vertexIndex].x);
					}

					if ((flags&CVRM_ENABLE_TEXTURE) && !meshPtr->textureCoords.empty())		//HasTextureCoords(texture_coordinates_set)
					{
						glTexCoord2f(texCoords[vertexIndex].x, /*1.0 -*/ texCoords[vertexIndex].y); //mTextureCoords[channel][vertex]
					}

					glVertex3fv(&meshPtr->vertices[vertexIndex].x);
				}
				glEnd();
			}
		}
#endif
	}
	void _render_pointcloud(CVRModel  *_site, MeshPtr meshPtr, int flags)
	{
#ifndef __ANDROID__
		auto *scene = _site->_this.get();

		bool hasTexture = false;

		bool onlyTexture = true;// hasTexture && (flags&CVRM_TEXTURE_NOLIGHTING);

		if (!meshPtr->normals.empty() && (flags&CVRM_ENABLE_LIGHTING) && !onlyTexture)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);

		const Point3f *normals = meshPtr->normals.empty() ? nullptr : &meshPtr->normals[0];
		const Vec4f   *colors = meshPtr->colors.empty() || !(flags&CVRM_ENABLE_VERTEX_COLOR) ? nullptr : &meshPtr->colors[0];
		const bool isUniformColor = meshPtr->colors.size() == 1;

		if (colors)
			glEnable(GL_COLOR_MATERIAL);
		else
			glDisable(GL_COLOR_MATERIAL);

		auto* vmask = !meshPtr->verticesMask.empty() && meshPtr->verticesMask.size() == meshPtr->vertices.size() ? &meshPtr->verticesMask[0] : nullptr;

		glPointSize(3.0f);		
		glBegin(GL_POINTS);		
		for (size_t i = 0; i < meshPtr->vertices.size(); ++i)
		{
			if (!vmask || vmask[i] != 0)
			{
				glColor3f(colors[i][0], colors[i][1], colors[i][2]);
				auto& p = meshPtr->vertices[i];
				glVertex3f(p.x, p.y, p.z);
			}
		}
		glEnd();
#endif
	}
	void render_node(CVRModel  *_site, CVRModel::Node *node, const Matx44f &mT, int flags)
	{
#ifndef __ANDROID__

		auto *scene = _site->_this.get();

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glMultMatrixf(mT.val);

		for (auto mi : node->meshes)
		{
			auto meshPtr = scene->meshes[mi];

			if (!meshPtr->faces.empty())
			{
				this->_render_faces(_site, meshPtr, flags);
			}
			else if(!meshPtr->vertices.empty())
			{
				this->_render_pointcloud(_site, meshPtr, flags);
			}
		}

		glPopMatrix();
#endif
	}


	void render(CVRModel *site, int flags)
	{
#ifndef __ANDROID__
#if 1
		if (_sceneList == 0)
		{
			_sceneList = glGenLists(1);
			_sceneListRenderFlags = flags - 1;//make them unequal to trigger re-compile
		}

		if (_sceneListRenderFlags != flags || _sceneListVersion != site->getUpdateVersion())
		{
			glNewList(_sceneList, GL_COMPILE);
			/* now begin at the root node of the imported data and traverse
			the scenegraph by multiplying subsequent local transforms
			together on GL's matrix stack. */
			site->forAllNodes([this, site, flags](CVRModel::Node *node, Matx44f &mT) {
				render_node(site, node, mT, flags);
			});
			glEndList();

			_sceneListRenderFlags = flags;
			_sceneListVersion = site->getUpdateVersion();
		}

		glCallList(_sceneList);

		checkGLError();
#else
		site->forAllNodes([this, site, flags](CVRModel::Node *node, Matx44f &mT) {
			render_node(_site, node, mT, flags);
#endif
#endif
	}
};

template<typename _ValT>
void _clonePtrVector(const std::vector<std::shared_ptr<_ValT>>& src, std::vector<std::shared_ptr<_ValT>> &tar)
{
	tar.clear();
	tar.resize(src.size());
	for (size_t i = 0; i < src.size(); ++i)
		if(src[i])
			tar[i] = std::shared_ptr<_ValT>(new _ValT(*src[i]));
}

CVRModel::NodePtr _cloneNodePtr(CVRModel::NodePtr ptr)
{
	if (ptr)
	{
		auto dptr = CVRModel::NodePtr(new CVRModel::Node(*ptr));
		for (auto& child : dptr->children)
			child = _cloneNodePtr(child);
		ptr = dptr;
	}
	return ptr;
}
CVRModel::This::This(const This &r)
	:modelFile(r.modelFile), infosUpdateVersion(r.infosUpdateVersion), updateVersion(r.updateVersion)
{
	_clonePtrVector(r.meshes, this->meshes);
	_clonePtrVector(r.materials, this->materials);
	if (r.infosPtr)
		this->infosPtr = std::make_shared<Infos>(*r.infosPtr);
	this->root = _cloneNodePtr(r.root);
}

CVRModel::MeshPtr CVRModel::This::getSingleMesh(bool createIfNotExist)
{
	if (meshes.empty() && createIfNotExist)
	{
		MeshPtr ptr(new Mesh);
		this->meshes.push_back(ptr);

		root = NodePtr(new Node);
		root->meshes.push_back(0);
		root->transformation = cv::Matx44f::eye();
		++this->updateVersion;
	}
	if (root->meshes.empty())
		return nullptr;

	CV_Assert(meshes.size() == 1);
	return meshes.front();
}

CVRModel::CVRModel()
	:_this(new This), _render(new _Render)
{}
CVRModel CVRModel::clone()
{
	CVRModel dst;
	dst._this = std::shared_ptr<This>(new This(*_this));
	return dst;
}
void CVRModel::clear()
{
	*this = CVRModel();
}

template<typename _ValT>
inline void _copyMeshData(std::vector<_ValT>& dst, const std::vector<_ValT>& src, int vertStart, _ValT defaultVal=_ValT())
{
	if (!src.empty())
	{
		dst.resize(dst.size() + src.size(), defaultVal);
		for (size_t i = 0; i < src.size(); ++i)
			dst[vertStart + i] = src[i];
	}
}

void CVRModel::applyAndClearTransforms()
{
	std::shared_ptr<This> newModel(new This);
	auto newMesh = newModel->getSingleMesh(true);
	newMesh->materialIndex = -1;

	size_t nVerts = 0;
	this->forAllNodeMeshes(
		[this, &nVerts](Mesh* mesh, const cv::Matx44f& mT) {
			nVerts += mesh->vertices.size();
		}
	);
	newMesh->vertices.reserve(nVerts);
	newMesh->normals.reserve(nVerts);
	newMesh->textureCoords.reserve(nVerts);
	newMesh->colors.reserve(nVerts);

	this->forAllNodeMeshes(
		[this, newMesh](Mesh* mesh, const cv::Matx44f& mT) {
			
			int vertStart = (int)newMesh->vertices.size();
			
			if (!mesh->vertices.empty())
			{
				newMesh->vertices.resize(newMesh->vertices.size() + mesh->vertices.size());
				for (size_t i = 0; i < mesh->vertices.size(); ++i)
				{
					newMesh->vertices[vertStart + i] = mesh->vertices[i] * mT;
				}
			}
			_copyMeshData(newMesh->verticesMask, mesh->verticesMask, vertStart, char(1));

			if (!mesh->normals.empty())
			{
				Point3f t = Point3f(0.f, 0.f, 0.f) * mT;
				newMesh->normals.resize(newMesh->vertices.size());
				for (size_t i = 0; i < mesh->normals.size(); ++i)
				{
					newMesh->normals[vertStart + i] = (Point3f)cv::normalize(Vec3f(mesh->normals[i] * mT - t));
				}
			}
			
			_copyMeshData(newMesh->colors, mesh->colors, vertStart, Vec4f(0.f, 0.f, 0.f, 0.f));
			_copyMeshData(newMesh->textureCoords, mesh->textureCoords, vertStart, cv::Point2f(0.f, 0.f));
			if (newMesh->materialIndex < 0 && mesh->materialIndex>=0)
				newMesh->materialIndex = mesh->materialIndex;

			for (auto& ft : mesh->faces)
			{
				auto* dft = newMesh->queryFaces(ft.numVertices);
				if (!dft)
				{
					newMesh->faces.push_back(Mesh::FaceType());
					newMesh->faces.back().numVertices = ft.numVertices;
					dft = &newMesh->faces.back();
				}

				size_t beg = dft->indices.size();
				dft->indices.insert(dft->indices.end(), ft.indices.begin(), ft.indices.end());
				for (size_t i = beg; i < dft->indices.size(); ++i)
					dft->indices[i] += vertStart;
			}
		}
	);


	newModel->modelFile = _this->modelFile;
	newModel->materials = _this->materials;
	newModel->updateVersion = _this->updateVersion + 1;

	_this = newModel;
}

void CVRModel::render(const Matx44f &sceneModelView, int flags)
{
#ifndef __ANDROID__
	if (*this)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(sceneModelView.val);

		if (this->isVisible())
		{
			_render->render(this, flags);
		}
	}
#endif
}


//=================================================================================


CVRModelEx::CVRModelEx(const CVRModel &_model, const Matx44f &_mModeli, const Matx44f &_mModel)
	:model(_model), mModeli(_mModeli), mModel(_mModel)
{}

void CVRModelEx::render(const Matx44f &sceneModelView, int flags)
{
	Matx44f mx = mModeli*mModel*sceneModelView;
	model.render(mx, flags);
}

void CVRModelArray::render(const Matx44f &sceneModelView, int flags)
{
	for (size_t i = 0; i < _v.size(); ++i)
		_v[i].render(sceneModelView, flags);
}

void CVRRendableArray::render(const Matx44f &sceneModelView, int flags)
{
	for (size_t i = 0; i < _v.size(); ++i)
		_v[i]->render(sceneModelView, flags);
}

//=================================================================================

//
//static void getSavedTextureName(const std::string &tarModelFile, const std::vector<TexImage> &vTex, std::vector<std::string> &curName, std::vector<std::string> &newName, bool isSTD)
//{
//	auto getNewFileName = [](const std::string &file, const std::string &prefix, bool single) {
//		std::string base = ff::GetFileName(file, false), ext = ff::GetFileExtention(file);
//		ff::str2lower(ext);
//		if (ext != "jpg")
//			ext = "png";
//		return prefix + (single ? "" : "_" + base) + "." + ext;
//	};
//
//	curName.resize(vTex.size());
//	newName.resize(vTex.size());
//
//	if (!vTex.empty())
//	{
//		for (size_t i = 0; i < vTex.size(); ++i)
//			curName[i] = vTex[i].file;
//
//		if (!isSTD)
//			newName = curName;
//		else
//		{
//			std::string tarName = ff::GetFileName(tarModelFile, false);
//
//			newName[0] = getNewFileName(curName[0], tarName, vTex.size() == 1 ? true : false);
//
//			for (size_t i = 1; i < vTex.size(); ++i)
//			{
//				newName[i] = getNewFileName(curName[i], tarName, false);
//			}
//		}
//	}
//}
//
//static void setTextureName(aiScene *scene, const std::vector<std::string> &curName, const std::vector<std::string> &newName)
//{
//	auto getNameIndex = [&curName](const std::string &name) {
//		int i = 0;
//		for (; i < curName.size(); ++i)
//			if (curName[i] == name)
//				break;
//		return i;
//	};
//
//	for (int m = 0; m < scene->mNumMaterials; ++m)
//	{
//		aiMaterial *mtl = scene->mMaterials[m];
//		int nTex;
//		if ((nTex = mtl->GetTextureCount(aiTextureType_DIFFUSE)) > 0)
//		{
//			aiString file;
//			for (int i = 0; i < nTex; ++i)
//			{
//				if (mtl->GetTexture(aiTextureType_DIFFUSE, i, &file) == AI_SUCCESS)
//				{
//					int idx = getNameIndex(file.data);
//					if (uint(idx) < newName.size())
//					{
//						aiString filex(newName[idx].c_str());
//						printf("%s\n", filex.data);
//						mtl->AddProperty(&filex, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, i));
//					}
//				}
//			}
//		}
//	}
//}
//
//
//
//static void saveTextureImages(const std::string &modelDir, const std::vector<TexImage> &vTex, const std::vector<std::string> &vTexName)
//{
//	for (size_t i = 0; i<vTex.size(); ++i)
//	{
//		if (!vTex[i].img.empty())
//		{
//			std::string fileloc = modelDir + "/" + vTexName[i];
//			if (!cv::imwrite(fileloc, vTex[i].img))
//				FF_EXCEPTION(ERR_FILE_WRITE_FAILED, fileloc.c_str());
//		}
//	}
//}
//
//
//void _saveModelAs(aiScene *scene, const std::string &file, std::string fmtID, const std::string & options)
//{
//	if (scene)
//	{
//		if (fmtID.empty())
//		{
//			fmtID = ff::GetFileExtention(file);
//			ff::str2lower(fmtID);
//		}
//
//		ff::ArgSet args(options);
//		bool isSTD = args.get<bool>("std");
//
//		std::vector<std::string>  curTexName, newTexName;
//		getSavedTextureName(file, vTex, curTexName, newTexName, isSTD);
//		if (isSTD)
//			setTextureName(scene, curTexName, newTexName);
//
//		Assimp::Exporter exp;
//		if (AI_SUCCESS != exp.Export(scene, fmtID, file))
//		{
//			FF_EXCEPTION(ERR_FILE_WRITE_FAILED, file.c_str());
//		}
//
//		if (isSTD)
//			setTextureName(scene, newTexName, curTexName);
//
//		std::string newDir = ff::GetDirectory(file);
//
//		saveTextureImages(newDir, vTex, newTexName);
//	}
//}

void saveModelFileAs(const std::string &inFile, const std::string &outFile, const Matx44f *mTransform)
{
#ifndef __ANDROID__
	aiScene *scene = (aiScene*)aiImportFile(inFile.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scene)
		FF_EXCEPTION(ERR_FILE_OPEN_FAILED, inFile.c_str());

	if (mTransform)
	{
		aiMatrix4x4 m;
		static_assert(sizeof(m) == sizeof(Matx44f), "");
		memcpy(&m, mTransform, sizeof(m));
		m.Transpose();

		aiMultiplyMatrix4(&m, &scene->mRootNode->mTransformation);
		scene->mRootNode->mTransformation = m;
	}

	auto fmtID = ff::GetFileExtention(outFile);
	ff::str2lower(fmtID);

	bool  err = false;
	Assimp::Exporter exp;
	if (AI_SUCCESS != exp.Export(scene, fmtID, outFile))
		err = true;
	
	if (!err)
	{
		std::string inDir = ff::GetDirectory(inFile), outDir = ff::GetDirectory(outFile);
		auto vmats = _SceneImpl::loadMaterials(scene, inDir);
		for (auto &m : vmats)
			for (auto &t : m->textures)
			{
				if (ff::pathExist(t.fullPath))
				{
					if (!ff::copyFile(t.fullPath, outDir + ff::GetFileName(t.fullPath), true))
						err = true;
				}
			}
	}

	aiReleaseImport(scene);

	if (err)
		FF_EXCEPTION1("failed to export model:" + inFile);
#endif
}

