#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "VulkanAPI/Utility/ISerializable.h"

namespace VULKAN
{
	enum ASSET_TYPE
	{
		NONE,
		FILE,
		IMAGE,
		TEXTURE,
		MATERIAL,
		MODEL
	};
	struct AssetData: ISerializable<AssetData>
	{
		int assetId;
		std::string name;
		std::string absolutePath;
		std::string extensionType;
		uintmax_t sizeInBytes;
		ASSET_TYPE assetType;


		AssetData Deserialize(nlohmann::json& jsonObj) override;
		nlohmann::json Serialize() override;
		void SaveData() override;

	};

	class AssetsHandler
	{

	protected:


		static AssetsHandler* instance;
		std::map<std::string, AssetData> assets;
		std::map<std::string, AssetData> assetsOnLoad;

		AssetsHandler();
		void SearchAllAssets(std::filesystem::path path);
		void LoadMetadata(std::string path);
		void CreateMetadata();
		void SaveMetadata();
		int assetCounter;

		std::string assetFilepath ;

	public:
		
		~AssetsHandler();
		void RegisterSaves();
		void AddAssetData(std::string path);
		AssetData GetAssetData(std::string path);

		static AssetsHandler* GetInstance();



	};

}
