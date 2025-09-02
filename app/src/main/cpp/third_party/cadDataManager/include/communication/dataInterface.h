/*
	* 鏈枃妗ｅ畾涔夊澶栨彁渚涚殑閫氫俊閾炬帴銆佹暟鎹浆鎹€佹暟鎹幏鍙栫殑鎺ュ彛
	* cadar闈掑矝鏈嶅姟鍣細 host 172.22.10.69锛?port 12345
*/

#pragma once

#include "global/baseDef.h"
#include "flatbuffers/flatbuffers.h"
#include "renderGroup/renderUnit.h"
#include "manager/dimensionManager.h"
#include "communication/request.h"
#include "communication/dataStructure.h"
#include <json.hpp>
#include "math/Raycaster.h"

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

using json = nlohmann::json;

namespace cadDataManager {

	static int port;

#ifdef _WIN32
	static SOCKET socketInstance;
#else
	static int socketInstance;
#endif


	class CAD_DATA_MANAGER_API DataInterface {

	public: 
		DataInterface& GetInstance() {
			static DataInterface instance;
			return instance;
		}

		//----------------------------------------鍒濆鍖?-----------------------------------------------------------------------------------------------------------------------------
		//鍒濆鍖栵細璇诲彇init.json鏂囦欢
		static void init();
		//浠巌nit.json涓幏鍙栦俊鎭細浠庢湰鍦拌鍙朏B鏂囦欢 or 浠庝簯绔浆鎹?
		static bool isReadLocalFBData();
		//浠巌nit.json涓幏鍙栦俊鎭細杞崲鏈湴CAD鏂囦欢 or 杞崲浜戠CAD鏂囦欢
		static bool isConvertModelByFile();




		//-----------------------------------------閫氳繃鏈湴璇诲彇FlatBuffer鏂囦欢锛岃幏鍙栬浆鎹㈠悗鐨勬ā鍨嬫暟鎹細鍒嗕负init.json璇诲弬銆佺洿鎺ヤ紶鍙備袱涓増鏈?----------------------------------------------
		//瑙ｆ瀽鏈湴CAD锛坒latBuffer锛夋暟鎹細浠呬粠鏈湴璇诲彇鏃惰皟鐢?
		static void parseLocalModel();
		static void parseLocalModel(std::string filePath);
        static void parseLocalModel(std::string fileName, std::string filePath);




        //-----------------------------------------瀵硅浆鎹㈢殑妯″瀷杩涜瑙ｆ瀽銆佺鐞嗐€佺紪杈戙€佸垹闄?----------------------------------------------

		//CAD妯″瀷鏁版嵁瑙ｆ瀽锛氫粠鏈湴璇诲彇鐨凜AD鏁版嵁鎴栦粠浜戠鍔犺浇鐨凜AD鏁版嵁锛屽潎閫氳繃璇ユ柟娉曡В鏋愶紝璇ユ柟娉曞唴閮ㄨ嚜鍔ㄨ皟鐢?
		static void parseModelData(std::string fileName, char* modelBuffer, size_t modelBufferSize);

		//鍚屾椂鍔犺浇澶氫釜妯″瀷鏃讹紝鍒囨崲鏌愪竴涓ā鍨嬩负娲昏穬鐘舵€?
		static void setActiveDocumentData(std::string fileName);

		//鍚屾椂鍔犺浇澶氫釜妯″瀷鏃讹紝鍒犻櫎鍏朵腑鏌愪竴涓ā鍨?
		static void removeModelData(std::string fileName);

		//娓呯┖褰撳墠宸茶В鏋愮殑鎵€鏈夋ā鍨嬫暟鎹?
		static void disposeDataManager();


		//-----------------------------------------鏁版嵁鑾峰彇鎺ュ彛-----------------------------------------------
		
		//娓叉煋鏁版嵁鑾峰彇鎺ュ彛: 鑾峰彇锛堝綋鍓嶆椿璺冩ā鍨嬶級鏌愪竴涓浂浠剁殑鍑犱綍鏁版嵁
		static std::vector<RenderInfo> getRenderInfoByProtoId(std::string protoId, bool printLog = false);

		//娓叉煋鏁版嵁鑾峰彇鎺ュ彛: 鑾峰彇锛堝綋鍓嶆椿璺冩ā鍨嬶級鎵€鏈夐浂浠跺嚑浣曟暟鎹殑鍒楄〃銆?锛堟帹鑽愪娇鐢ㄤ笅闈㈢殑getRenderInfoMap鑾峰彇妯″瀷鏁版嵁锛?
		static std::vector<RenderInfo> getRenderInfo(bool printLog = false);
		
		//娓叉煋鏁版嵁鑾峰彇鎺ュ彛: 鑾峰彇锛堝綋鍓嶆椿璺冩ā鍨嬶級鎵€鏈夐浂浠跺嚑浣曟暟鎹殑Map锛宬ey鍊间负闆朵欢ID銆?锛堟帹鑽愪互Map褰㈠紡鑾峰彇锛屾柟渚胯繘琛屾ā鍨嬬殑鏇存柊锛?
		static std::unordered_map<std::string, std::vector<RenderInfo>> getRenderInfoMap(bool printLog = false);

		//pmi鑾峰彇鎺ュ彛锛氳幏鍙栵紙褰撳墠娲昏穬妯″瀷锛夋墍鏈塒MI鏁版嵁
		static std::vector<pmiInfo> getPmiInfos(bool printLog = false);

		//instance鑾峰彇鎺ュ彛锛氳繑鍥烇紙褰撳墠娲昏穬妯″瀷锛夊畬鏁寸殑Instance鏁版嵁锛孖nstance鍙傛暟缁撴瀯浼氳緝涓哄鏉?
		static std::unordered_map<std::string, Instance::Ptr> getInstances(bool printLog = false);

		//instanceInfo鑾峰彇鎺ュ彛锛欼nstanceInfo鏄instance杩涜绠€鍖栵紝鍙彁渚涙嬀鍙栦氦浜掑繀瑕佺殑淇℃伅锛屽彲浠ユ牴鎹渶姹傛墿灞?
		static std::unordered_map<std::string, InstanceInfo::Ptr> getInstanceInfos(bool printLog = false);

		//鑾峰彇杞崲寰楀埌鐨勶紙褰撳墠娲昏穬妯″瀷锛塅latBuffer鏁版嵁
		static std::string getModelFlatbuffersData();



		//-----------------------------------------妯″瀷淇℃伅淇敼锛屽苟杩斿洖淇敼鍚庣殑鏁版嵁-----------------------------------------------
		
		//淇敼锛堝綋鍓嶆椿璺冩ā鍨嬶級鍏冪礌鐨勫瑙?
		static std::unordered_map<std::string, std::vector<RenderInfo>> modifyElementColor(std::string instanceId, std::vector<int> elementIds, std::string color);
		
		//鎭㈠锛堝綋鍓嶆椿璺冩ā鍨嬶級鍏冪礌鐨勫瑙?
		static std::unordered_map<std::string, std::vector<RenderInfo>> restoreElementColor(std::string instanceId, std::vector<int> elementIds);
	
		//淇敼锛堝綋鍓嶆椿璺冩ā鍨嬶級瀹炰緥鐨勫瑙?
		static std::unordered_map<std::string, std::vector<RenderInfo>> modifyInstanceColor(std::string instanceId, std::string color);

		//鎭㈠锛堝綋鍓嶆椿璺冩ā鍨嬶級瀹炰緥鐨勫瑙?
		static std::unordered_map<std::string, std::vector<RenderInfo>> restoreInstanceColor(std::string instanceId);

		//淇敼锛堝綋鍓嶆椿璺冩ā鍨嬶級瀹炰緥鐨勪綅濮跨煩闃?
		static std::unordered_map<std::string, std::vector<RenderInfo>> modifyInstanceMatrix(std::string instanceId, std::vector<float> matrix);

		//鎭㈠锛堝綋鍓嶆椿璺冩ā鍨嬶級瀹炰緥鐨勪綅濮跨煩闃?
		static std::unordered_map<std::string, std::vector<RenderInfo>> restoreInstanceMatrix(std::string instanceId);





		//-----------------------------------------JSON鏁版嵁鍔犺浇瑙ｆ瀽锛堢洰鍓嶅彧閽堝椹鹃┒鑸变氦浜掓ā鍨嬶級-----------------------------------------------
		//鍔犺浇鍔ㄧ敾鐘舵€佹暟鎹?: 绌哄弬鏃朵細浠巌nit.json涓鍙栧弬鏁?
		static void loadAnimationStateData(std::string filePath = "");

		//鑾峰彇鍔ㄧ敾鐘舵€佸師濮婮SON鏁版嵁
		static std::string getAnimationStateJsonStr(std::string filePath = "");

		//鍔犺浇鍔ㄧ敾鍔ㄤ綔璇存槑鏁版嵁
		static void loadAnimationActionData(std::string filePath = "");

		//鑾峰彇鍔ㄧ敾鍔ㄤ綔鍘熷JSON鏁版嵁
		static std::string getAnimationActionJsonStr(std::string filePath = "");

		//鍔犺浇鏉愯川鍙傛暟
		static void loadMaterialData(std::string filePath = "");

		//鑾峰彇鏉愯川鍙傛暟鍘熷JSON鏁版嵁
		static std::string getMaterialJsonStr(std::string filePath = "");



		
		//-----------------------------------------鎷惧彇-----------------------------------------------
		static Intersection::Ptr pickInstance(std::vector<float> origin, std::vector<float> direction);
	};
}