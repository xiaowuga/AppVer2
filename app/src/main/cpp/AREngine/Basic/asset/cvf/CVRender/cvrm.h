#pragma once

#include"cvrbase.h"


class _CVR_API cvrm
{
public:
	static Matx44f diag(float s);

	static Matx44f I()
	{
		return diag(1.0f);
	}

	static Matx44f translate(float tx, float ty, float tz);

	static Matx44f scale(float sx, float sy, float sz);

	static Matx44f scale(float s)
	{
		return scale(s, s, s);
	}

	static Matx44f rotate(float angle, const cv::Vec3f &axis);

	//rotation M that transforms s to t : t=s*M;
	static Matx44f  rotate(const cv::Vec3f &s, const cv::Vec3f &t);

	static Matx44f rotate(const float r0[3], const float r1[3], const float r2[3]);

	static Matx44f ortho(float left, float right, float bottom, float top, float nearP, float farP);

	static Matx44f perspective(float f, Size windowSize, float nearP, float farP);

	//conver intrinsic parameters to projection matrix
	static Matx44f perspective(float fx, float fy, float cx, float cy, Size windowSize, float nearP, float farP);

	//fscale = focal-length/image-height;
	static Matx33f defaultK(Size imageSize, float fscale);

	static Matx33f scaleK(const Matx33f& K, float scalex, float scaley);

	static Matx33f scaleK(const Matx33f& K, float scale)
	{
		return scaleK(K, scale, scale);
	}

	static Matx33f makeK(float fx, float fy, float cx, float cy);

	static Matx44f fromK(const Matx33f &K, Size windowSize, float nearP, float farP);

	static Matx33f getKInROI(Matx33f K, const cv::Rect& roi, float scale = 1.f);

	//convert rotation and translation vectors to model-view matrix
	//Note: @rvec and @tvec are in OpenCV coordinates, returned 4x4 matrix is in OpenGL coordinates
	static Matx44f fromRT(const cv::Vec3f &rvec, const cv::Vec3f &tvec, bool cv2gl=true);

	static Matx44f fromR33T(const cv::Matx33f &R, const cv::Vec3f &tvec, bool cv2gl=true);

	//decompose a 3x4 (OpenGL) transformation matrix to (OpenCV) Rotation-Translation vectors
	//the matrix @m should contain only rotation and translation
	static void decomposeRT(const Matx44f &m, cv::Vec3f &rvec, cv::Vec3f &tvec, bool gl2cv=true);

	static void decomposeRT(const Matx44f &m, cv::Matx33f &R, cv::Vec3f &tvec, bool gl2cv=true);

	static Matx44f lookat(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz);

	static cv::Point3f unproject(const cv::Point3f &winPt, const Matx44f &mModelview, const Matx44f &mProjection, const int viewport[4]);

	static cv::Point3f project(const cv::Point3f &objPt, const Matx44f &mModelview, const Matx44f &mProjection, const int viewport[4]);

	static void  sampleSphere(std::vector<cv::Vec3f> &vecs, int n);

	static cv::Vec3f rot2Euler(const cv::Matx33f &R);

	static cv::Matx33f euler2Rot(float rx, float ry, float rz);

	/*
	timesOfSubdiv=0, nPoints=12
	timesOfSubdiv=1, nPoints=42
	timesOfSubdiv=2, nPoints=162
	timesOfSubdiv=3, nPoints=642
	timesOfSubdiv=4, nPoints=2562
	timesOfSubdiv=5, nPoints=10242
	timesOfSubdiv=6, nPoints=40962
	timesOfSubdiv=7, nPoints=163842
	timesOfSubdiv=8, nPoints=655362
	*/
	static void  sampleSphereFromIcosahedron(std::vector<cv::Vec3f>& vecs, int timesOfSubdiv = 4);
};

// y=x*M;
_CVR_API cv::Vec3f operator*(const cv::Vec3f &x, const Matx44f &M);

_CVR_API cv::Point3f operator*(const cv::Point3f &x, const Matx44f &M);



class _CVRTrackBall;

class _CVR_API CVRTrackBall
{
	std::shared_ptr<_CVRTrackBall> impl;
public:
	CVRTrackBall();

	~CVRTrackBall();

	void onMouseDown(int x, int y);

	void onMouseMove(int x, int y, Size viewSize);

	void onMouseWheel(int x, int y, int val);

	void onKeyDown(int key, int flags);

	/* get and apply the transformations
	rotation and scale are applied separately to model and view transformations
	if @update==false, the input value of @mModel and @mView will be ignored, otherwise they will be multiplied with the TrackBall transformations
	*/
	void apply(Matx44f &mModel, Matx44f &mView, bool update = true);
};

