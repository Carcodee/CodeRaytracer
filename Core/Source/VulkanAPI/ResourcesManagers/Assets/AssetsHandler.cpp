#include "AssetsHandler.h"

#include <assert.h>
#include <iostream>
#include <queue>
#include <stack>

#include "FileSystem/FileHandler.h"
#include "VulkanAPI/Utility/InputSystem/InputHandler.h"


namespace VULKAN
{
	AssetsHandler* AssetsHandler::instance = nullptr;
	HELPERS::FileHandler* fileHandlerInstance = HELPERS::FileHandler::GetInstance();

	AssetsHandler::AssetsHandler()
	{

		assetFilepath = fileHandlerInstance->GetAssetsPath() + "/AssetsData.txt";
		fileHandlerInstance = HELPERS::FileHandler::GetInstance();

//		if (std::filesystem::exists(assetFilepath))
//		{
//			LoadMetadata(assetFilepath);
//		}
//		else
//		{
//			//if the data file does not exist we search for the assets and create a file 
////			CreateMetadata();
//		}

        assetThreat.AddTask([this](){
            SearchAllAssets(fileHandlerInstance->GetAssetsPath());
            
//            DeserializeCodeFile<Material>(ModelHandler::GetInstance()->allMaterialsOnApp);
            DeserializeCodeFile<ModelData>(ModelHandler::GetInstance()->allModelsOnApp);
            std::cout << "Assets Founded: " << assets.size() <<"\n";
            
        });
        

		
		
	}

	AssetsHandler::~AssetsHandler()
	{
		SaveMetadata();
	}

	void AssetsHandler::SearchAllAssets(std::filesystem::path path)
	{

		std::queue<std::filesystem::path> pathsToSeek{};
		std::filesystem::path currentPath = path;
		pathsToSeek.push(path);
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
				else if (directoryPath.path().extension()== assetFileExtension)
				{
					std::string pathToCheck = directoryPath.path().string();
					LoadSingleAssetMetadata(pathToCheck);
                    
				}

			}
			pathsToSeek.pop();
			if (pathsToSeek.size()>0)
			{
				currentPath = pathsToSeek.front();
			}
			
		}

	}

	void AssetsHandler::SaveMetadata()
	{
		assets.clear();
		SearchAllAssets(fileHandlerInstance->GetAssetsPath());
        int counter = 0;
        for(auto& pair: assets){
            std::fstream fstream(pair.first, std::ios::out | std::ios::trunc);
            if (fstream.is_open())
            {
                fstream.clear();
                fstream.close();
            }
            else
            {
                std::cout << "There is no metadata file \n";
                continue;
            }
            counter++;
            nlohmann::json jsonData = pair.second.Serialize();
            fileHandlerInstance->AppendToFile(pair.first, jsonData.dump(4));
        }

        std::cout << "Asset Count saved: "<< counter<<"\n";




    }

	void AssetsHandler::RegisterSaves()
	{
		if (InputHandler::GetInstance()->GetUserInput(InputHandler::KEY_RIGHT_CONTROL, InputHandler::ACTION_HOLD)
		&& InputHandler::GetInstance()->GetUserInput(InputHandler::KEY_S, InputHandler::ACTION_DOWN)){
//			SaveMetadata();
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


	void AssetsHandler::AddAssetData(std::string path, std::string codeAssetPath)
	{
		AssetData asset{};
		asset.assetType = NONE;
		std::filesystem::path currentPath(path);
		if (!std::filesystem::is_regular_file(path))
		{
			return;
		}
		asset.extensionType = currentPath.extension().string();
        
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

        std::string name =std::filesystem::path(path).filename().string();
        size_t objPos= name.find_last_of('.');
        name= name.substr(0,objPos);
        
		asset.name = name;

		asset.sizeInBytes = fileHandlerInstance->GetFileSize(path);


		asset.assetId = static_cast<int>(assets.size()) + 1;
        
        
        asset.codeFilePath=path;

        std::filesystem::path pathToAsset(path);
        
        std::string strPathToAsset = fileHandlerInstance->RemovePathExtension(pathToAsset);
        strPathToAsset = strPathToAsset + assetFileExtension;
        
        nlohmann::json data=asset.Serialize();
        
        fileHandlerInstance->CreateFile(strPathToAsset,data.dump(4));
        
        assets.try_emplace(strPathToAsset, asset);

	}

	AssetData& AssetsHandler::GetAssetData(std::string path)
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

    
    void AssetsHandler::CreateSingleAssetMetadata(std::filesystem::path path, std::string data) {
        assert(path != "" && "Asset must be different from \"\"");
        fileHandlerInstance->CreateFile(path.string(),data);
        
        AddAssetData(path.string());
    }

    
    void AssetsHandler::LoadSingleAssetMetadata(std::filesystem::path path) {

        if (!std::filesystem::exists(path)){
            std::cout<<"Couldn't open path to load data: "<<path<<"\n";
            return;
        }
        
        bool extensionIsValid= path.extension() == assetFileExtension;
        assert(extensionIsValid && "Loading asset must be only for asset extension filetypes");
        
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
                    auto result =assets.try_emplace(path.string(), asset);
                    if (result.second){
                        if (asset.extensionType==matFileExtension){
                            materialPaths.push_back(asset.codeFilePath);
                        } else if (asset.extensionType==codeModelFileExtension){
                            modelsPaths.push_back(asset.codeFilePath);
                        }
                    }


                    currentJsonString = "";
                }
                catch (nlohmann::json::parse_error& e)
                {
                    std::cout << "Parse error at line " << line << ": " << e.what() << "\n";
                }
            }

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
                "CodeFilePath",this->codeFilePath
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
        this->codeFilePath = jsonObj.at("CodeFilePath");
        this->assetType = jsonObj.at("AssetType");
        this->extensionType = jsonObj.at("ExtensionType");
		this->name = jsonObj.at("Name");
		this->sizeInBytes = jsonObj.at("SizeInBytes");
		return *this;
	}

	
}
