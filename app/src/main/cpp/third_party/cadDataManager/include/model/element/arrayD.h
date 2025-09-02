#pragma once
#include "flatbuffers/flatbuffers.h"
#include "parser/serialization_generated.h"
namespace cadDataManager {
	//TODO: D鏄唴鏍哥殑缁撴瀯锛屽簲璇ヨ繘涓€姝ヨВ鏋?
	class arrayD
	{
	public:
		int                                                                                       FT;
		int                                                                                       FN;
		const flatbuffers::Vector<int>* I;
		const flatbuffers::Vector<unsigned short>* IC;
		const flatbuffers::Vector<float>* N;
		const flatbuffers::Vector<unsigned short>* NC;
		const flatbuffers::Vector<float>* V;
		const flatbuffers::Vector<float>* TE;
		int                                                                                       VN;
		int                                                                                       VO;

	};
}