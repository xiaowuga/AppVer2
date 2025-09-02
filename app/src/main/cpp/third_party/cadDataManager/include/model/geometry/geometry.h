#pragma once
#include "global/baseDef.h"

//TODO: 鐎规矮绠熼崙鐘辩秿閺佺増宓侀敍瀛峫ement閵嗕笒ntity閵嗕赋roto閸氬嫯鍤滈柈鑺ョ€鍝勵嚠鎼存棑绱檓erge閿涘鐪扮痪褏娈慓eometry閿?闂団偓鐟曚椒绗屾径鏍潎閸栧綊鍘ょ€电懓绨?
namespace cadDataManager {
	class Geometry : public std::enable_shared_from_this<Geometry> {
	public:
		using Ptr = std::shared_ptr<Geometry>;
		static Ptr create() {
			return std::make_shared<Geometry>();
		}

		Geometry() = default;

		~Geometry() = default;

		void setPosition(std::vector<float>& addPosition);
		void setNormal(std::vector<float>& addNormal);
		void setUV(std::vector<float>& addUV);
		void setIndex(std::vector<int>& addIndex);

		std::vector<float> getPosition();
		std::vector<float> getNormal();
		std::vector<float> getUV();
		std::vector<int> getIndex();

	public:
		std::vector<float>          position;
		std::vector<float>          normal;
		std::vector<float>          uv;
		std::vector<int>            index;
	};
}