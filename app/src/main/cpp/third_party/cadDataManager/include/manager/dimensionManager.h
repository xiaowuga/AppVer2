#pragma once
#include "global/baseDef.h"
#include "model/dimension/modelBasedDefinition.h"
#include "math/Vector3.h"
#include "math/Line3.h"
#include "math/Plane.h"
#include "assemblyManager.h"
namespace cadDataManager {
	struct pmiInfo {
		std::vector<std::vector<float>> points; //(Diagonal/Horizontal: startPnt1 endPnt1 startPnt2 endPnt2) (Diameter/Radius: startPnt1杩?startPnt2杩?centerPnt) 涓栫晫鍧愭爣绯讳笅
		std::vector<std::vector<float>> text; // textPnt, textNormal, textDir(浠呭渾鐩村緞澶杙mi鏂囧瓧鏈?
		std::string type; // pmi type
		std::string value; // text value 
		std::string protoId; //鎵€灞瀙roto 
		int eleId; // pmi 瀵瑰簲element 
		std::vector<std::vector<float>> instanceMatrixList; //瀵瑰簲instance matrix list
	};


	class DimensionManager
	{
	private:
		std::unordered_map<int, ModelBasedDefinition::Ptr>			mDimensionMap{}; //鐢╡leId鍖哄垎
		std::vector<pmiInfo>										mPmiInfos{};

	public:
		DimensionManager() {};
		~DimensionManager() {};

		using Ptr = std::shared_ptr<DimensionManager>;
		static Ptr create() {
			return std::make_shared<DimensionManager>();
		}

		std::unordered_map<int, ModelBasedDefinition::Ptr> getDimensionMap() { return mDimensionMap; };

		void clearDimensionMap();

		void addDimension(ModelBasedDefinition::Ptr dimension);

		void removeDimension(int id);

		ModelBasedDefinition::Ptr getDimension(int id);

		bool existDimension(int id);

		void addToDimensionMap(ModelBasedDefinition::Ptr dimension);

		void buildPmiInfo(ModelBasedDefinition::Ptr dimension, std::string protoId);

		std::vector<pmiInfo> getPmiInfoList() { return mPmiInfos; };

		void getDistancePmiInfo(ModelBasedDefinition::Ptr dimension, std::string protoId);

		void getDiameterPmiInfo(ModelBasedDefinition::Ptr dimension, std::string protoId);
	};
}