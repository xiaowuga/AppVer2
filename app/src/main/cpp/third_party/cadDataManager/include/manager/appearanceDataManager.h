#pragma once
#include "global/baseDef.h"
#include "parser/serialization_generated.h"


//澶栬鏁版嵁绠＄悊绫?
//鐢ㄤ簬瑙ｆ瀽鍐呮牳杩斿洖鐨勫瑙傛暟鎹瓵ppearanceDatas
//闆朵欢鏂囨。 瀹炰綋銆侀潰澶栬淇℃伅鍦?Data/AppearanceDatas涓?
//瑁呴厤鏂囨。 瀹炰綋銆侀潰澶栬淇℃伅鍦?Data/ProtoDatas/AppearanceDatas涓? 瀹炰緥澶栬淇℃伅瀛樺湪 Data/AppearanceDatas涓?

namespace cadDataManager {
	class AppearanceDataManager {
	public:
		using Ptr = std::shared_ptr<AppearanceDataManager>;
		static Ptr create() {
			return std::make_shared<AppearanceDataManager>();
		}

		static AppearanceDataManager& GetInstance() {
			static AppearanceDataManager instance;
			return instance;
		}

		std::string getAppearanceDataParamsStr(int appearanceId, std::string docId = "");

		void addAppearanceData(const FlatBufferDocSpace::AppearanceData* appearanceObj, std::string docId = "");

		void clearAppearanceData();

	public:

		//澶栬鏁版嵁Map
		//灏嗗唴鏍歌繑鍥炵殑澶栬鏁版嵁瑙ｆ瀽鎴怣ap,渚夸簬绱㈠紩
		//key: appearanceDataId 澶栬淇℃伅绱㈠紩鍊?
		//value: appearanceDataParamStr 澶栬淇℃伅瀛楃涓?

		/**
		 * 瀛樺偍涓嶅悓鏂囨。鐨刟ppearanceData
		 * key: appearanceDataId (Data涓嬬殑浣跨敤documentId,Data/ProtoDatas/涓嬩娇鐢╬rotoId)
		 * value: appearanceDataMap (璇ap瑙ｆ瀽骞跺瓨鍌ˋppearanceDatas涓殑鏁版嵁  key: appearanceId 澶栬淇℃伅绱㈠紩鍊? value: appearanceParamStr 澶栬淇℃伅瀛楃涓?)
		 */
		std::unordered_map<std::string, std::unordered_map<int, std::string>> appearanceDatasMap;
		AppearanceDataManager() {};
		~AppearanceDataManager() {};
	};
}