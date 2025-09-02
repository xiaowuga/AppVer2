#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <BasicData.h>
#include "Basic/include/json.hpp"
namespace Animator {	
	struct AttributeData
	{
		std::string state;
		std::vector<float> localMatrix;
		std::vector<int> highlightElementId;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(AttributeData, state, localMatrix, highlightElementId)
	};
	struct AnimationData
	{
		std::string originState;
		std::string targetState;
		std::vector<float> positionArray;
		std::vector<float> quaternionArray;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationData, originState, targetState, positionArray, quaternionArray)
	};
	struct InstanceData
	{
		std::string modelName;
		std::string instanceId;
		std::string instanceName;
		std::vector<std::string> allStates;
		std::string originState;
		std::vector<AttributeData> attribute;
		std::vector<AnimationData> animation;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(InstanceData, modelName, instanceId, instanceName, allStates, originState, attribute, animation)
	};
	struct StateAnimation
	{
		std::string currState;
		std::string nextState;
		std::vector<Pose> animationSequence;
	};
	struct StateMaterial
	{
		std::string state;
		std::vector<int> highlightElementID;
	};
	class StateLoader
	{
	private:
		std::string instanceStateJsonPath;
		std::vector<InstanceData> instanceData;

	public:
		StateLoader(std::string instanceStateJsonPath);
		std::vector<StateAnimation> FindAnimation(std::string instanceName);
		std::vector<StateMaterial> FindMaterial(std::string instanceName);
		std::vector<std::string> FindAllStates(std::string instanceName);
	};
	class Animator
	{
	public:
		/**
		* @brief Initialize the animator
		* @param curState The current animation state
		* @param sceneObjectPtr The scene object pointer
		* @param instanceStatePath The instance state path
		* */
		Animator(int curState, SceneObjectPtr sceneObjectPtr, const std::string& instanceStatePath);

		/**
		* @brief Update the material to target state
		* @param targetState The target animation state
		**/
		void LoadMaterialForStateChange(std::string targetState);

		/**
		* @brief Update the animation to target state
		* @param targetState The target animation state
		**/
		void LoadAnimationForStateChange(std::string currentState, std::string targetState);

		/**
		* @brief Play the animation
		**/
		void PlayNextFrame();
		
		//Is the animation playing
		bool isPlaying;
		//Current animation state index
		int currentStateIndex;
		//Next animation state index
		int nextStateIndex;
		//All animation states list
		std::vector<std::string> allStates;
		//Animation sequence index
		int animationSequenceIndex;
	private:
		//Animator object
		SceneObjectPtr sceneObject;
		//Animation speed
		float animationSpeed;
		//Animation sequence
		std::vector<Pose> animationSequence;		
		//Animation time accumulator
		float animationTimeAccumulator = 0.0;
		//Animations
		std::vector<StateAnimation> stateAnimations;
		//Materials
		std::vector<StateMaterial> stateMaterials;
		//Highlight elements
		std::vector<int> highlightElementID;
		//Animation Loaded
		bool isLoaded;
	};

}; // namespace Animator