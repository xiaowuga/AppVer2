#pragma once

#include "global/baseDef.h"
#include "math/Box3.h"
#include "model/geometry/geometry.h"
#include "global/typeDefine.h"
#include "model/appearance/appearanceParams.h"

namespace cadDataManager {
	enum ConversionPrecision {
		middle,
		low,
		high
	};


	class GeometryInfo {
	public:
		using Ptr = std::shared_ptr<GeometryInfo>;
		static Ptr create() {
			return std::make_shared<GeometryInfo>();
		}

		GeometryInfo() = default;
		~GeometryInfo() = default;

	public:
		ElementType type;
		std::vector<Geometry::Ptr> geometries;

		Box3 mGeometryBox; //鍑犱綍鐨凮OBB寮忓寘鍥寸洅
		std::vector<float> mGeometryBoxCenter; //鍑犱綍鐨勫寘鍥寸洅涓績

		void buildBoxInfo() {
			Box3 protoGeometryBox;
			for (Geometry::Ptr geometry : geometries) {
				Box3 box;
				std::vector<float> positionArray = geometry->getPosition();
				box.setFromVector(positionArray);
				protoGeometryBox.unionBox3(box);
			}
			mGeometryBox = protoGeometryBox;
			mGeometryBoxCenter = mGeometryBox.getCenter().buildArray();
		}
	};


	class ElementInfo {
	public:
		int mID;
		ElementType mType;
		GeomType mGeometryType;
		Geometry::Ptr mGeometry;
		Box3 mGeometryBox; //鍑犱綍鐨凮OBB寮忓寘鍥寸洅

		//TODO 鍏抽敭鍑犱綍淇℃伅锛?
		/*
			1.鏅€氬钩闈細娉曞悜銆傚彲璁＄畻
			2.鍦嗗瀷骞抽潰锛氭硶鍚戙€佸崐寰勩€佸渾蹇冦€傛殏鏃?
			3.鏌遍潰锛氳酱鍚戙€佸崐寰勩€佷笂椤堕潰鍦嗗績銆佷笅椤堕潰鍦嗗績銆傛殏鏃?
			4.鐞冮潰锛氱悆蹇冦€佸崐寰勩€傛殏鏃?
			5.鏅€氭洸闈細鏃犮€?

			1.鐩寸嚎锛氳捣濮嬬偣銆佺粓姝㈢偣銆侀暱搴︺€?
			2.鍦嗕笌鍦嗗姬绾匡細娉曞悜锛堝彲璁＄畻锛夈€佸渾蹇冦€佸崐寰勩€?

			閰嶅悎涓殏鏃舵病鏈夋湁鏁堜俊鎭?鍙湁鍏宠仈Element涓庡叧鑱擨nstance鐨処D锛岀嚎涓庣嚎涔嬮棿鐨勯厤鍚堝彲浠ラ€氳繃Element鏈韩鐨勪俊鎭彇鍒般€?

			鍔ㄧ敾鐨勪俊鎭斁鍦ㄥ唴鏍?
		*/

	public:
		using Ptr = std::shared_ptr<ElementInfo>;
		static Ptr create() {
			return std::make_shared<ElementInfo>();
		}

		ElementInfo() = default;
		~ElementInfo() = default;

		void buildBoxInfo() {
			std::vector<float> positionArray = mGeometry->getPosition();
			mGeometryBox.setFromVector(positionArray);
		}
	};


	class InstanceInfo {
	public:
		std::string mInstanceId;
		std::string mProtoId;
		std::string mType; //instance绫诲瀷锛?闆朵欢 or 瑁呴厤
		std::string mParentId;
		std::vector<std::string> mChildIds;
		std::vector<GeometryInfo::Ptr> mGeometryInfos;
		std::vector<float> mMatrixWorld;
		std::unordered_map<int, ElementInfo::Ptr> mElementInfoMap;

	public:
		using Ptr = std::shared_ptr<InstanceInfo>;
		static Ptr create() {
			return std::make_shared<InstanceInfo>();
		}

		InstanceInfo() = default;
		~InstanceInfo() = default;
	};


	struct RenderInfo
	{
		AppearanceParams::Ptr params; //澶栬绫?
		Geometry::Ptr geo; //鍑犱綍淇℃伅
		int matrixNum; //鐭╅樀涓暟
		std::vector<float> matrix; //鐭╅樀
		std::string type; //鍑犱綍绫诲瀷 face edge
		std::string protoId;
		std::vector<std::string> instanceIds;

		void console() const {
			//TODO: 鍑犱綍椤剁偣鏁伴噺銆佷笁瑙掗潰鐗囨暟閲忋€侀鑹层€侀€忔槑搴?
			std::vector<float> position = this->geo->getPosition();
			std::vector<int> index = this->geo->getIndex();
			if (this->type == "face") {
			}
			//TODO: 鏄惁鎵撳嵃鐨勬帶鍒跺櫒
			int flag = 0;
			for (auto it = this->matrix.begin(); it != this->matrix.end(); ++it) {
				if (++flag == 16) {
					std::cout << "\n";
					flag = 0;
				}
			}
		}

		size_t getTriangleNumber() {
			if (this->type == "face") {
				std::vector<int> index = this->geo->getIndex();
				size_t number = index.size() / 3 * this->matrixNum;
				return number;
			}
			return 0;
		}
	};
}