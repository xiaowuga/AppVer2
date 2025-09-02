#include "BasicData.h"
#include "ConfigLoader.h"
#include"RPC.h"

// HandPose 类成员函数定义
HandPose::HandPose() : tag(hand_tag::Null), joints{} {}

HandPose::HandPose(int tag_value, std::array<cv::Vec3f, 21> joint_positions,
                   std::array<std::array<float, 2>, 21> joints2d_positions,
                   std::array<std::array<std::array<float, 3>, 3>, 16> rotation_matrices,
                   std::array<float, 10> shape_params, cv::Vec3f origin)
        : tag(static_cast<hand_tag>(tag_value)),
          joints(joint_positions), joints2d(joints2d_positions),rotation(rotation_matrices),
          shape(shape_params), landmarkOrigin(origin) {}

HandPose::HandPose(int tag_value, std::vector<cv::Point3f> joint_positions,
                   std::array<std::array<float, 2>, 21> joints2d_positions,
                   std::array<std::array<std::array<float, 3>, 3>, 16> rotation_matrices,
                   std::array<float, 10> shape_params, cv::Vec3f origin)
        :tag(static_cast<hand_tag>(tag_value)),	joints2d(joints2d_positions),rotation(rotation_matrices),shape(shape_params), landmarkOrigin(origin)
{
    for(int i=0;i<jointNum;i++){
        joints[i] = cv::Vec3f(joint_positions[i].x,
                              joint_positions[i].y,
                              joint_positions[i].z);
    }
}

HandPose::HandPose(const HandPose& other)
        : tag(other.tag),
          joints(other.joints),
          joints2d(other.joints2d),
          rotation(other.rotation),
          shape(other.shape),
          landmarkOrigin(other.landmarkOrigin) {}




HandPose& HandPose::operator=(const HandPose& other) {
    if (this != &other) { // 防止自赋值
        tag = other.tag;
        joints = other.joints;
        joints2d = other.joints2d;
        rotation = other.rotation;
        shape = other.shape;
        landmarkOrigin = other.landmarkOrigin;
    }
    return *this;
}
// 获取 joints
std::array<cv::Vec3f, 21>& HandPose::getjoints() {
    return joints;
}
std::array<std::array<float, 2>, 21>& HandPose::getjoints2d() {
    return joints2d;
}

// 获取 rotation
std::array<std::array<std::array<float, 3>, 3>, 16>& HandPose::getrotation() {
    return rotation;
}

// 获取 shape
std::array<float, 10>& HandPose::getshape() {
    return shape;
}
hand_tag HandPose::gettag(){
    return tag;
}

cv::Vec3f HandPose:: getlandmarkOrigin(){
    return landmarkOrigin;
}
void HandPose::setPose(int tag, std::array<cv::Vec3f, HandPose::jointNum> joints, cv::Vec3f landmarkOrigin) {
    this->tag = hand_tag(tag);
    this->joints = joints;

    this->landmarkOrigin = landmarkOrigin;
}

void HandPose::showResult(){
    std::cout<<"joints[i]:  ";
    for(int i=0;i<21;i++){
        std::cout<<joints[i] <<"\t";
    }
    std::cout<<"joints2d[i]:  ";
    for(int i=0;i<21;i++){
        std::cout<<joints2d[i][0]<<" "<< joints2d[i][1]<<"\t";
    }

    std::cout<<"\nrotation[i]:  ";
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                std::cout<<rotation[i][j][k]<<"\t";
            }
        }
        std::cout<<"\t";
    }
    std::cout<<"\nshape[i]:  ";
    for(int i=0;i<10;i++){
        std::cout<<shape[i]<<"\t";
    }
}

// BodyPose 类成员函数定义
BodyPose::BodyPose() :  joints{} {}

BodyPose::BodyPose( std::array<cv::Vec3f, jointNum> joints) :joints(joints) {}


std::array<cv::Vec3f, BodyPose::jointNum> BodyPose::getjoints() {
    return joints;
}

void BodyPose::setPose(std::array<cv::Vec3f, BodyPose::jointNum> joints) {
    this->joints = joints;
}

// FacePose 类成员函数定义
FacePose::FacePose() :  joints{} {}

FacePose::FacePose( std::array<cv::Vec3f, jointNum> joints) :joints(joints) {}


std::array<cv::Vec3f, FacePose::jointNum> FacePose::getjoints() {
    return joints;
}

void FacePose::setPose(std::array<cv::Vec3f, FacePose::jointNum> joints) {
    this->joints = joints;
}



bool BasicData::setData(const std::string& key, const std::any& value)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

	if (!_hasData(key))
	{
		this->data.emplace(key, value);
	}
	else {
		this->data[key] = value;
	}
	return true;
}

std::any BasicData::getData(const std::string& key)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
	if (_hasData(key))
	{
		return data[key];
	}
	return {}; //return empty
}

bool BasicData::hasData(const std::string& key) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return _hasData(key);
}

bool BasicData::removeData(const std::string& key)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
	if (!_hasData(key))
	{
		return false;
	}
	this->data.erase(this->data.find(key));
	return true;
}

void BasicData::clearData()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
	this->data.clear();
}

BasicData::~BasicData()
{}

void Pose::setPose(cv::Matx44f mat)
{
    matrix=mat;
}

cv::Matx44f Pose::GetMatrix() const
{
    return this->matrix;
}

AppData::AppData()
{
	this->_continue = true;
	// this->setData("engineDir", std::any(std::string(PROJECT_PATH)+"/AREngine"));  // AREngine Dir path
}

void Camera::Init(){
	ConfigLoader configloader(this->filePath);
	this->width = configloader.getValue<int>("width");
	this->height = configloader.getValue<int>("height");
	this->fps = configloader.getValue<int>("fps");
	this->IntrinsicMatrix = configloader.getCamIntrinsicMatrix();
	std::cout << "cam-K:" << this->IntrinsicMatrix << std::endl;
    this->distCoeffs = configloader.getCamDistCoeff();
	std::cout << "cam-K2:" << this->distCoeffs << std::endl;
	return;
}

bool FrameData::hasUploaded(const std::string& key) const
{
	return serilizedFramePtr && serilizedFramePtr->has(key);
}

