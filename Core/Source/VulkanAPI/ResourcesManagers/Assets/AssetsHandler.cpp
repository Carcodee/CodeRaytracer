#include "AssetsHandler.h"

#include <assert.h>

#include "FileSystem/FileHandler.h"


namespace VULKAN
{
	AssetsHandler* AssetsHandler::instance = nullptr;
	HELPERS::FileHandler* fileHandlerInstance = HELPERS::FileHandler::GetInstance();
	std::map<std::string, AssetData> AssetsHandler::assets{};

	AssetsHandler::AssetsHandler()
	{
		fileHandlerInstance = HELPERS::FileHandler::GetInstance();
		LoadAllAssets(fileHandlerInstance->GetAssetsPath());
	}

	void AssetsHandler::LoadAllAssets(std::filesystem::path path)
	{
		std::vector<std::filesystem::path>fileEntries;
		for (auto element : std::filesystem::directory_iterator(path))
		{
			if (element.is_directory())
			{
				LoadAllAssets(element);
			}
		}

	}

	AssetsHandler* AssetsHandler::GetInstance()
	{
		if (instance==nullptr)
		{
			instance = new AssetsHandler;
		}
		return instance;
	}


	AssetData AssetsHandler::GetAssetData(std::string path)
	{
		AssetData asset{};
		asset.assetType = NONE;
		if (!std::filesystem::is_regular_file(path))
		{
			return asset;
		}
		std::filesystem::path currentPath(path);
		
		if (assets.contains(path))
		{
			return assets.at(path);
		}
		std::string extension = fileHandlerInstance->GetPathExtension(path);

		asset.extensionType = extension;

		if (asset.extensionType== "obj")
		{
			asset.assetType = MODEL;
		}
		else if (asset.extensionType == "png" || asset.extensionType == "jpg")
		{
			asset.assetType = IMAGE;
		}
		else
		{
			asset.assetType = FILE;
		}

		asset.sizeInBytes = fileHandlerInstance->GetFileSize(path);

		asset.absolutePath = path;

		asset.assetId = static_cast<int>(assets.size()) + 1;

		assets.try_emplace(path, asset);

		return asset;






	}

}
