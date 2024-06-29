#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

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
	struct AssetData
	{
		int assetId;
		std::string name;
		std::string absolutePath;
		std::string extensionType;
		uintmax_t sizeInBytes;
		ASSET_TYPE assetType;
	};

	class AssetsHandler
	{

	protected:
		static AssetsHandler* instance;
		static std::map<std::string, AssetData> assets;
		AssetsHandler();
		void LoadAllAssets(std::filesystem::path path);
		int assetCounter;

	public:
		
		AssetData GetAssetData(std::string path);
		static AssetsHandler* GetInstance();



	};

}
