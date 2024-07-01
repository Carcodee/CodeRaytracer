#include "AssetsHandler.h"

#include <assert.h>
#include <iostream>
#include <queue>
#include <stack>
#include <fstream>

#include "FileSystem/FileHandler.h"


namespace VULKAN
{
	AssetsHandler* AssetsHandler::instance = nullptr;
	HELPERS::FileHandler* fileHandlerInstance = HELPERS::FileHandler::GetInstance();

	AssetsHandler::AssetsHandler()
	{

		assetFilepath = fileHandlerInstance->GetAssetsPath() + "/AssetsData.txt";

		fileHandlerInstance = HELPERS::FileHandler::GetInstance();

		if (std::filesystem::exists(assetFilepath))
		{

			LoadMetadata(assetFilepath);
		}
		else
		{
			//if the data file does not exist we search for the assets and create a file 
			SearchAllAssets(fileHandlerInstance->GetAssetsPath());
			CreateMetadata();
		}

		std::cout << "Assets Founded: " << assets.size() <<"\n";
		
		
	}

	AssetsHandler::~AssetsHandler()
	{
	
		SaveMetadataOnClose();

	}

	void AssetsHandler::SearchAllAssets(std::filesystem::path path)
	{

		std::queue<std::filesystem::path> pathsToSeek{};
		std::filesystem::path currentPath = path;
		pathsToSeek.push(path);
		int counter = 0;
		while (!pathsToSeek.empty())
		{
			for (auto& directoryPath : std::filesystem::directory_iterator(currentPath))
			{
				if (directoryPath.path() == std::filesystem::path(assetFilepath))
				{
					continue;
				}
				if (is_directory(directoryPath))
				{
					pathsToSeek.push(directoryPath);
				}
				else
				{

					std::string pathToCheck = directoryPath.path().string();
					AddAssetData(pathToCheck);
				}

			}
			pathsToSeek.pop();
			if (pathsToSeek.size()>0)
			{
				currentPath = pathsToSeek.front();
			}
			
		}



	}

	
	void AssetsHandler::CreateMetadata()
	{
		if (!std::filesystem::exists(assetFilepath))
		{
			fileHandlerInstance->CreateFile(assetFilepath);
			std::vector<nlohmann::json> assetsJson;
			for (auto& pairVal : assets)
			{
				assetsJson.push_back(pairVal.second.Serialize());
			}
			assetsOnLoad = assets;
			for (auto json : assetsJson)
			{
				fileHandlerInstance->AppendToFile(assetFilepath, json.dump(4));
			}
		}
		
	}

	void AssetsHandler::SaveMetadataOnClose()
	{
		std::vector<AssetData>newAssets;
		assets.clear();
		SearchAllAssets(fileHandlerInstance->GetAssetsPath());


		bool recreateMetata = false;
		int offset = 0;
		for (auto pair : assetsOnLoad)
		{
			if (!assets.contains(pair.first))
			{
				//offset++;

				//pair.second.assetId = assets.size() + offset;
				//assetsOnLoad.try_emplace(pair.first, pair.second);
				//
				//nlohmann::json assetJson = pair.second.Serialize();

				//fileHandlerInstance->AppendToFile(assetFilepath,assetJson.dump(4));
				recreateMetata = true;

			}

		}

		for (auto& pair : assets)
		{
			if (!assetsOnLoad.contains(pair.first))
			{
				recreateMetata = true;
			}
		}

		if (recreateMetata)
		{
			std::fstream fstream(assetFilepath, std::ios::out | std::ios::trunc);
			if (fstream.is_open())
			{
				fstream.clear();
				fstream.close();
			}
			else
			{
				std::cout << "There is no metadata file \n";
			}

			std::vector<nlohmann::json> assetsJson;
			for (auto& pairVal : assets)
			{
				assetsJson.push_back(pairVal.second.Serialize());
			}
			assetsOnLoad = assets;
			for (auto json : assetsJson)
			{
				fileHandlerInstance->AppendToFile(assetFilepath, json.dump(4));
			}

		}

		std::cout << "Total saved on close: " << assets.size() << "\n";




	}

	void AssetsHandler::LoadMetadata(std::string path)
	{
		std::vector<AssetData>assetsToLoad;
		std::ifstream inFile(path);
		std::string line;
		std::string currentJsonString = "";
		while (std::getline(inFile, line))
		{
			
			currentJsonString += line;

			if (line[0]=='}')
			{
				try
				{
					nlohmann::json currentJson = nlohmann::json::parse(currentJsonString);
					AssetData asset{};
					asset.Deserialize(currentJson);
					assets.try_emplace(asset.absolutePath, asset);
					assetsOnLoad.try_emplace(asset.absolutePath, asset);
					currentJsonString = "";
				}
				catch (nlohmann::json::parse_error& e)
				{
					std::cout << "Parse error at line " << line << ": " << e.what() << "\n";
				}
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


	void AssetsHandler::AddAssetData(std::string path)
	{
		AssetData asset{};
		asset.assetType = NONE;
		std::filesystem::path currentPath(path);
		if (!std::filesystem::is_regular_file(path))
		{
			return;
		}
		std::string extension = fileHandlerInstance->GetPathExtension(path);

		asset.extensionType = extension;

		std::string ext = currentPath.extension().string();
		if (asset.extensionType== ".obj")
		{
			asset.assetType = MODEL;
		}
		else if (asset.extensionType == ".png" || asset.extensionType == ".jpg")
		{
			asset.assetType = IMAGE;
		}
		else
		{
			asset.assetType = FILE;
		}

		asset.name = std::filesystem::path(path).filename().string();

		asset.sizeInBytes = fileHandlerInstance->GetFileSize(path);

		asset.absolutePath = path;

		asset.assetId = static_cast<int>(assets.size()) + 1;

		assets.try_emplace(path, asset);


	}

	AssetData AssetsHandler::GetAssetData(std::string path)
	{
		if (assets.contains(path))
		{
			return assets.at(path);
		}
		else
		{
			AssetData asset{  };
			asset.assetType = NONE;
			return asset;
		}
	}
	
	nlohmann::json AssetData::Serialize()
	{
		nlohmann::json jsonData;
		jsonData = {
			{
				"AssetId",this->assetId
			},
			{
				"Name",this->name
			},
			{
				"AbsolutePath",this->absolutePath
			},
			{
				"ExtensionType",this->extensionType
			},
			{
				"SizeInBytes",this->sizeInBytes
			},
			{
				"AssetType",this->assetType
			},
		};
		return jsonData;

	}

	void AssetData::SaveData()
	{
		if (AssetsHandler::GetInstance())
		{
			
		}
	}


	AssetData AssetData::Deserialize(nlohmann::json& jsonObj)
	{

		this->assetId = jsonObj.at("AssetId");
		this->name = jsonObj.at("Name");
		this->absolutePath = jsonObj.at("AbsolutePath");
		this->sizeInBytes = jsonObj.at("SizeInBytes");
		this->assetType = jsonObj.at("AssetType");
		return *this;
	}

	
}
