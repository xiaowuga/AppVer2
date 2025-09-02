#include "Animator.h"

Animator::Animator::Animator(int curState, SceneObjectPtr sceneObjectPtr, const std::string& instanceStatePath)
{
	sceneObject = sceneObjectPtr;
	StateLoader stateLoader = StateLoader(instanceStatePath);
	allStates = stateLoader.FindAllStates(sceneObjectPtr->name);
	currentStateIndex = curState;
	nextStateIndex = curState;
	stateAnimations = stateLoader.FindAnimation(sceneObjectPtr->name);
	stateMaterials = stateLoader.FindMaterial(sceneObjectPtr->name);
	
	for (auto& StateMaterial : stateMaterials)
	{
		if (StateMaterial.state == allStates[currentStateIndex])
		{
			highlightElementID = StateMaterial.highlightElementID;
			break;
		}
	}
	isLoaded = true;
	animationSpeed = 0.05f;
	isPlaying = false;
}

void Animator::Animator::LoadMaterialForStateChange(std::string targetState)
{
	StateMaterial targetMaterial;
	for (auto& StateMaterial : stateMaterials)
	{
		if (StateMaterial.state == targetState)
		{
			targetMaterial = StateMaterial;
			break;
		}
	}
	highlightElementID = targetMaterial.highlightElementID;
	//TODO: Update the material of the sceneObject
}

void Animator::Animator::LoadAnimationForStateChange(std::string currentState, std::string targetState)
{
	bool isReverse = false;
	StateAnimation targetAnimation;
	for (auto& StateAnimation : stateAnimations)
	{
		if (StateAnimation.currState == currentState && StateAnimation.nextState == targetState)
		{
			targetAnimation = StateAnimation;
			break;
		}
		else if (StateAnimation.currState == targetState && StateAnimation.nextState == currentState)
		{
			targetAnimation = StateAnimation;
			isReverse = true;
			break;
		}
	}
	animationSequence = targetAnimation.animationSequence;
	if (isReverse)
	{
		std::reverse(animationSequence.begin(), animationSequence.end());
	}
	isPlaying = true;
}

void Animator::Animator::PlayNextFrame()
{
	if (animationSequence.size() == 0)
	{
		return;
	}
	if (animationSequenceIndex >= animationSequence.size()) {
		currentStateIndex = nextStateIndex;
		isPlaying = false;
		animationTimeAccumulator = 0.0f;
	}
	else {
		//TODO: Get delta time
		animationTimeAccumulator += animationSpeed;
		if (animationTimeAccumulator >= animationSpeed)
		{
			
			int prevAnimationSequenceIndex = animationSequenceIndex > 0 ? animationSequenceIndex - 1 : animationSequence.size() - 1;
			Pose prevPose = animationSequence[prevAnimationSequenceIndex];
			Pose pose = animationSequence[animationSequenceIndex];
			//TODO: ͳһ����ϵ
			//Update the sceneObject pose
			cv::Matx44f ori_pose = sceneObject->transform.GetPosition();
			cv::Matx44f ori_rotation = sceneObject->transform.GetRotation();
			cv::Matx44f position = pose.GetPosition();
			cv::Matx44f rotation = pose.GetRotation();
			cv::Matx44f prev_position_inv = prevPose.GetPosition().inv();
			cv::Matx44f prev_rotation_inv = prevPose.GetRotation().inv();
			cv::Matx44f trans = ori_pose * rotation * prev_rotation_inv * ori_rotation;
			sceneObject->transform = Pose(trans);
			animationTimeAccumulator = 0.0f;
			animationSequenceIndex++;

		}
	}
}

Animator::StateLoader::StateLoader(std::string instanceStateJsonPath)
{
	this->instanceStateJsonPath = instanceStateJsonPath;
}

std::vector<Animator::StateAnimation> Animator::StateLoader::FindAnimation(std::string instanceName)
{
	//Parse the json file and save the data to instanceData
	if (instanceData.size() == 0)
	{
		std::ifstream file(instanceStateJsonPath);
		if (!file) {
			throw std::runtime_error("Cannot open file: " + instanceStateJsonPath);
		}
		nlohmann::json jsonData;
		file >> jsonData;  // ʹ�� nlohmann/json ֱ�ӽ����ļ�
		file.close();
		instanceData = jsonData.get<std::vector<InstanceData>>();
	}
	std::vector<StateAnimation> instanceAnimation;
	//Find the instanceData with the given instanceName
	auto instance = std::find_if(instanceData.begin(), instanceData.end(), [instanceName](InstanceData& instanceData) {return instanceData.instanceName == instanceName; });
	if (instance != instanceData.end())
	{
		for (auto& animationData : instance->animation)
		{
			std::string currState = animationData.originState;
			std::string nextState = animationData.targetState;
			std::vector<Pose> animationSequence;
			// Parse animation sequence
			for (int i = 0; i < animationData.positionArray.size() / 3; i++)
			{
				cv::Vec3f position = cv::Vec3f(animationData.positionArray[i * 3], animationData.positionArray[i * 3 + 1], animationData.positionArray[i * 3 + 2]);
				cv::Vec4f rotation = cv::Vec4f(animationData.quaternionArray[i * 4], animationData.quaternionArray[i * 4 + 1], animationData.quaternionArray[i * 4 + 2], animationData.quaternionArray[i * 4 + 3]);
				cv::Matx44f mat = cv::Matx44f(
					1 - 2 * rotation[1] * rotation[1] - 2 * rotation[2] * rotation[2], 2 * rotation[0] * rotation[1] - 2 * rotation[2] * rotation[3], 2 * rotation[0] * rotation[2] + 2 * rotation[1] * rotation[3], position[0],
					2 * rotation[0] * rotation[1] + 2 * rotation[2] * rotation[3], 1 - 2 * rotation[0] * rotation[0] - 2 * rotation[2] * rotation[2], 2 * rotation[1] * rotation[2] - 2 * rotation[0] * rotation[3], position[1],
					2 * rotation[0] * rotation[2] - 2 * rotation[1] * rotation[3], 2 * rotation[1] * rotation[2] + 2 * rotation[0] * rotation[3], 1 - 2 * rotation[0] * rotation[0] - 2 * rotation[1] * rotation[1], position[2],
					0, 0, 0, 1);
				Pose pose(mat);
				animationSequence.push_back(pose);
			}
			instanceAnimation.push_back({ currState, nextState, animationSequence });
		}
	}
	return instanceAnimation;
}

std::vector<Animator::StateMaterial> Animator::StateLoader::FindMaterial(std::string instanceName)
{
	//Parse the json file and save the data to instanceData
	if (instanceData.size() == 0)
	{
		std::ifstream file(instanceStateJsonPath);
		if (!file) {
			throw std::runtime_error("Cannot open file: " + instanceStateJsonPath);
		}
		nlohmann::json jsonData;
		file >> jsonData;  // ʹ�� nlohmann/json ֱ�ӽ����ļ�
		file.close();
		instanceData = jsonData.get<std::vector<InstanceData>>();
	}
	std::vector<StateMaterial> instanceMaterial;
	//Find the instanceData with the given instanceName
	auto instance = std::find_if(instanceData.begin(), instanceData.end(), [instanceName](InstanceData& instanceData) {return instanceData.instanceName == instanceName; });
	if (instance != instanceData.end())
	{
		for (auto& attributeData : instance->attribute)
		{
			std::string state = attributeData.state;
			std::vector<int> highlightElementID = attributeData.highlightElementId;
			instanceMaterial.push_back({ state, highlightElementID });
		}
	}
	return instanceMaterial;
}

std::vector<std::string> Animator::StateLoader::FindAllStates(std::string instanceName)
{
	//Parse the json file and save the data to instanceData
	if (instanceData.size() == 0)
	{
		std::ifstream file(instanceStateJsonPath);
		if (!file) {
			throw std::runtime_error("Cannot open file: " + instanceStateJsonPath);
		}
		nlohmann::json jsonData;
		file >> jsonData;  // ʹ�� nlohmann/json ֱ�ӽ����ļ�
		file.close();
		instanceData = jsonData.get<std::vector<InstanceData>>();
	}
	std::vector<std::string> allStates;
	//Find the instanceData with the given instanceName
	auto instance = std::find_if(instanceData.begin(), instanceData.end(), [instanceName](InstanceData& instanceData) {return instanceData.instanceName == instanceName; });
	if (instance != instanceData.end())
	{
		allStates = instance->allStates;
	}
	return allStates;
}
