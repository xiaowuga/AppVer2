#pragma once
#include "flatbuffers/flatbuffers.h"
#include "parser/serialization_generated.h"
#include "math/Matrix4.h"
#include "model/appearance/appearanceParams.h"
#include "model/proto/proto.h"
#include "model/attribute/renderStateParams.h"

namespace cadDataManager {
	class Instance : public std::enable_shared_from_this<Instance> {
	public:
		using Ptr = std::shared_ptr<Instance>;
		static Ptr create() {
			return std::make_shared<Instance>();
		}

		Instance() = default;
		~Instance() = default;

		void setId(std::string id) { mId = id; };
		void setIndex(std::string index) { mIndex = index; };
		void setName(std::string name) { mName = name; };
		void setAppearanceParam(AppearanceParams::Ptr appearance) { mAppearanceParam = appearance; };
		void setRenderStateParam(RenderStateParams::Ptr renderStateParam) { mRenderStateParam = renderStateParam; };
		void setProtoId(std::string protoId) { mProtoId = protoId; };
		void setStatus(std::string status) { mStatus = status; };
		void setVisible(bool visible) { mVisible = visible; };
		void setOriginVisible(bool originVisible) { mOriginVisible = originVisible; };
		void setFix(bool fix) { mFix = fix; };
		void setIsValid(int isValid) { mIsValid = isValid; };
		void setOriginIsValid(int originIsValid) { mOriginIsValid = originIsValid; };
		void setMatrix(const flatbuffers::Vector<double>* matrix);
		void setOriginMatrix(const flatbuffers::Vector<double>* matrix);
		void setParent(Instance::Ptr parent) { mParent = parent; };
		void setChild(Instance::Ptr child) { mChildren.push_back(child); };
		void setChildren(std::vector<Instance::Ptr> children) { mChildren = children; };
		void setProtoRef(Proto::Ptr proto) { mProtoRef = proto; };

		std::string getId() { return mId; };
		std::string getIndex() { return mIndex; };
		std::string getName() { return mName; };
		std::string getProtoId() { return mProtoId; };
		std::string getStatus() { return mStatus; };
		AppearanceParams::Ptr getAppearanceParam() { return mAppearanceParam; };
		RenderStateParams::Ptr getRenderStateParam() { return mRenderStateParam; };
		bool getVisible() { return mVisible; };
		bool getOriginVisible() { return mOriginVisible; };
		bool getFix() { return mFix; };
		int getIsValid() { return mIsValid; };
		int getOriginIsValid() { return mOriginIsValid; };
		std::vector<double> getMatrix() { return mMatrix; };
		std::vector<double> getOriginMatrix() { return mOriginMatrix; };
		std::vector<Instance::Ptr> getChildren() { return mChildren; };
		Instance::Ptr getParent() {
			if (!mParent.expired()) {
				return mParent.lock();
			}

			return nullptr;
		};
		Proto::Ptr getProtoRef() { return mProtoRef; };

		void bindChildren();
		void bindChild(Instance::Ptr childInstance);
		Matrix4 getMatrixWorld();
		Matrix4& getMatrixWorld(Matrix4& m);
		Instance::Ptr getTopInstance();


	private:
		std::string                   mId{ "" };
		std::string                   mIndex{ "" };
		std::string                   mName{ "" };
		std::string                   mProtoId{ "" };
		std::string                   mStatus{ "" };
		AppearanceParams::Ptr         mAppearanceParam { nullptr };
		RenderStateParams::Ptr        mRenderStateParam { nullptr };
		bool                          mVisible{ true };
		bool                          mOriginVisible{ true };
		bool                          mFix{ false };
		int                           mIsValid{ 1 }; // 鍒ゆ柇瀹炰緥鏍戜腑鏄惁澧炲姞涓㈠け鏍囪瘑锛屽瓙涓㈠け鐖剁骇涔熼渶瑕佹樉绀轰涪澶?
		int                           mOriginIsValid{ 1 }; // 鍐呮牳杩斿洖鐨勫疄渚嬫槸鍚︿涪澶辩殑鐘舵€?
		std::vector<double>           mMatrix;
		std::vector<double>           mOriginMatrix;
		std::vector<Instance::Ptr>    mChildren{};
		std::weak_ptr<Instance>		  mParent;
		Proto::Ptr                    mProtoRef{nullptr};
	};
}