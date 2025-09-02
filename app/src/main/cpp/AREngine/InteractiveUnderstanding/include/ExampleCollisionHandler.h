#ifndef ARTEST_EXAMPLECOLLISIONHANDLER_H
#define ARTEST_EXAMPLECOLLISIONHANDLER_H
#pragma once
#include <iostream>
//#include "Basic/include/BasicData.h"
//#include "InteractionConfigLoader.h"
#include "BasicData.h"
#include "InteractionConfigLoader.h"
#include "Animator.h"
class LogCollisionHandler : public CollisionHandler {
public:
    using Ptr = std::shared_ptr<LogCollisionHandler>;
    static CollisionHandler::Ptr create() {
        return std::make_shared<LogCollisionHandler>();
    }
    void OnCollision(std::shared_ptr<CollisionData> obj1, std::shared_ptr<CollisionData> obj2, FrameDataPtr _frameDataPtr, AppData* appDataPtr) override {
        std::cout << "Collision detected between " << obj1->name << " and " << obj2->name << std::endl;
    }
    static inline int registered = InteractionConfigLoader::RegisterCollisionHandler("LogCollisionHandler", &LogCollisionHandler::create);
};

class ProtectiveCoverCollisionHandler : public CollisionHandler {
public:
	using Ptr = std::shared_ptr<ProtectiveCoverCollisionHandler>;
	static CollisionHandler::Ptr create() {		
		return std::make_shared<ProtectiveCoverCollisionHandler>();
	}
	void OnCollision(std::shared_ptr<CollisionData> obj1, std::shared_ptr<CollisionData> obj2, FrameDataPtr _frameDataPtr, AppData* appDataPtr) override {		
		std::cout << "Collision detected between " << obj1->name << " and " << obj2->name << std::endl;
		if (animator == nullptr) {
			animator = new Animator::Animator(0, obj2->obj, appDataPtr->dataDir + "/InstanceState.json");
		}
		if (animator->isPlaying) {
			animator->PlayNextFrame();
		}
		else {
			//TODO: ���ݶ���ѡ����һ������һ��״̬
			if (animator->currentStateIndex < animator->allStates.size()) {
				animator->currentStateIndex++;
			}
			else {
				animator->currentStateIndex = 0;
			}
			if (animator->currentStateIndex != animator->nextStateIndex) {
				animator->LoadAnimationForStateChange(animator->allStates[animator->currentStateIndex], animator->allStates[animator->nextStateIndex]);
				animator->isPlaying = true;
				animator->animationSequenceIndex = 0;
			}
		}
	}
	bool isOpen() {
		return animator->allStates[animator->currentStateIndex] == "OPEN";
	}
	static inline int registered = InteractionConfigLoader::RegisterCollisionHandler("ProtectiveCoverCollisionHandler", &ProtectiveCoverCollisionHandler::create);
private:
	Animator::Animator* animator;
};

class MoveCollisionHandler : public CollisionHandler {
public:
    using Ptr = std::shared_ptr<MoveCollisionHandler>;
    static CollisionHandler::Ptr create() {
        return std::make_shared<MoveCollisionHandler>();
    }
    void OnCollision(std::shared_ptr<CollisionData> obj1, std::shared_ptr<CollisionData> obj2, FrameDataPtr _frameDataPtr, AppData* appDataPtr) override {
		auto initPos = obj2->obj->initTransform.GetPosition();
		auto initRot = obj1->obj->initTransform.GetRotation();
		//obj1->obj->initTransform = initPos * initRot;
		obj1->obj->initTransform = obj2->obj->initTransform.GetMatrix();

        auto pos = obj2->obj->transform.GetMatrix();
		auto rot = obj1->obj->transform.GetMatrix();
		//obj1->obj->transform = pos * rot;
		obj1->obj->transform = obj2->obj->transform.GetMatrix();
		_frameDataPtr->controlling = obj1->name;
    }
    static inline int registered = InteractionConfigLoader::RegisterCollisionHandler("MoveCollisionHandler", &MoveCollisionHandler::create);
};
#endif //ARTEST_EXAMPLECOLLISIONHANDLER_H
