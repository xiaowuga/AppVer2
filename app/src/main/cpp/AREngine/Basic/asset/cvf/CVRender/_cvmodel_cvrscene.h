#pragma once

#include"_cvrender.h"


//
//static void loadTextureImages(const CVRScene& scene, const std::string &modelDir, std::vector<TexImage> &vTex)
//{
//	vTex.clear();
//
//	for(auto &mtl : scene.materials)
//	{
//		size_t nTex=mtl->textures.size();
//		if (nTex > 0)
//		{
//			for (size_t i = 0; i < nTex; ++i)
//			{
//				vTex.push_back({ mtl->textures[i].path });
//			}
//		}
//	}
//
//	for (size_t i = 0; i<vTex.size(); i++)
//	{
//		std::string fileloc = modelDir + "/" + vTex[i].file;	/* Loading of image */
//		cv::Mat img = cv::imread(fileloc, IMREAD_UNCHANGED);
//
//		if (!img.empty()) /* If no error occurred: */
//		{
//			makeSizePower2(img);
//
//			vTex[i].img = img;
//		}
//		else
//		{
//			/* Error occurred */
//			printf("error: failed to load %s\n", fileloc.c_str());
//		}
//	}
//}

template<typename _ValT>
inline bool readStorage(cv::FileStorage &fs, const std::string &var, _ValT &val) {
	if (!fs.isOpened())
		return false;

	try {
		fs[var] >> val;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

void _CVRModel::load(const std::string &file, int postProLevel, const std::string &options)
{
	std::shared_ptr<CVRScene> newScene(new CVRScene);
	newScene->load(file, postProLevel, options);

	this->scene = newScene;

	this->_updateSceneInfo();

	//loadTextureImages(*this->scene, ff::GetDirectory(file), vTex);
}

void get_bounding_box(CVRScene *scene, Point3f* min, Point3f* max)
{
	min->x = min->y = min->z = 1e10f;
	max->x = max->y = max->z = -1e10f;

	/*double c[3] = { 0,0,0 };
	int n = 0;*/

	scene->forAllVertices([min, max/*, &c, &n*/](Point3f &tmp) {
		min->x = __min(min->x, tmp.x);
		min->y = __min(min->y, tmp.y);
		min->z = __min(min->z, tmp.z);

		max->x = __max(max->x, tmp.x);
		max->y = __max(max->y, tmp.y);
		max->z = __max(max->z, tmp.z);

		//c[0] += tmp.x; c[1] += tmp.y; c[2] += tmp.z;
		//++n;
	});
}

void _CVRModel::_updateSceneInfo()
{
	/*get_bounding_box(scene.get(), &scene_min, &scene_max);
	scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
	scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
	scene_center.z = (scene_min.z + scene_max.z) / 2.0f;*/
}

//==================================================================================


void _CVRModel::saveAs(const std::string &file, std::string fmtID, const std::string & options)
{
}


void _CVRModel::render_node(CVRScene *scene, CVRScene::Node *node, const Matx44f &mT, _CVRModel *site, int flags)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(mT.val);

	for (auto mi : node->meshes)
	{
		auto meshPtr = scene->_meshes[mi];

		glBindTexture(GL_TEXTURE_2D, 0);

		auto &mat = scene->_materials[meshPtr->materialIndex];
		bool hasTexture = false;
		if ((flags&CVRM_ENABLE_TEXTURE) && !mat->textures.empty())
		{
			//auto &t = mTex[mat->textures.front().path];
			auto &t = site->_getTexture(mat->textures.front());
			glBindTexture(GL_TEXTURE_2D, t.texID);
			hasTexture = true;
		}

		bool onlyTexture = hasTexture && (flags&CVRM_TEXTURE_NOLIGHTING);

		if (!meshPtr->normals.empty() && (flags&CVRM_ENABLE_LIGHTING) && !onlyTexture)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
		
		const Point3f *normals = meshPtr->normals.empty() ? nullptr : &meshPtr->normals[0];
		const Point3f *texCoords = (flags&CVRM_ENABLE_TEXTURE) && !meshPtr->textureCoords.empty() ? &meshPtr->textureCoords[0] : nullptr;
		const Vec4f   *colors = meshPtr->colors.empty() ? nullptr : &meshPtr->colors[0];

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
							glColor4fv(&colors[vertexIndex][0]);

						if (normals)
							glNormal3fv(&normals[vertexIndex].x);
					}

					if ((flags&CVRM_ENABLE_TEXTURE) && !meshPtr->textureCoords.empty())		//HasTextureCoords(texture_coordinates_set)
					{
						glTexCoord2f(texCoords[vertexIndex].x, 1.0 - texCoords[vertexIndex].y); //mTextureCoords[channel][vertex]
					}

					glVertex3fv(&meshPtr->vertices[vertexIndex].x);
				}
				glEnd();
			}
		}
	}

	glPopMatrix();
}

void _CVRModel::render(int flags)
{
	//_initGL();
#if 1
	if (_sceneList == 0)
	{
		_sceneList = glGenLists(1);
		_sceneListRenderFlags = flags - 1;//make them unequal to trigger re-compile
	}

	if (_sceneListRenderFlags != flags || _sceneListVersion!=scene->getUpdateVersion())
	{
		glNewList(_sceneList, GL_COMPILE);
		/* now begin at the root node of the imported data and traverse
		the scenegraph by multiplying subsequent local transforms
		together on GL's matrix stack. */
		scene->forAllNodes([this, flags](CVRScene::Node *node, Matx44f &mT) {
			render_node(scene.get(), node, mT, this, flags);
		});
		glEndList();

		_sceneListRenderFlags = flags;
		_sceneListVersion = scene->getUpdateVersion();
	}

	glCallList(_sceneList);

	checkGLError();
#else
	scene->forAllNodes([this, flags](CVRScene::Node *node, Matx44f &mT) {
		render_node(scene.get(), node, mT, this->_texMap, flags);
	});
#endif
}



#if 1

void _CVRModel::getMesh(CVRMesh &mesh, int flags)
{
	CV_Assert(false);
	//_getMesh(this->scene, mesh);
}
#endif

Matx44f _CVRModel::calcStdPose()
{
	CV_Assert(false);
}



void _CVRModel::setSceneTransformation(const Matx44f &trans, bool updateSceneInfo)
{
	CV_Assert(false);
	//aiMatrix4x4 m;
	//static_assert(sizeof(m) == sizeof(trans), "");
	//memcpy(&m, &trans, sizeof(m));
	//m.Transpose();

	//_transformAllVetices(scene, m);
	////aiIdentityMatrix4(&this->_sceneTransform);
	//aiIdentityMatrix4(&this->_sceneTransformInFile);


	////aiMultiplyMatrix4(&m, &_sceneTransformInFile);
	////scene->mRootNode->mTransformation = m;

	//if (updateSceneInfo)
	//	this->_updateSceneInfo();

	//_vertices.clear();
}



