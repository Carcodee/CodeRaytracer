#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "VulkanAPI/Utility/ISerializable.h"
#include "VulkanAPI/Model/ModelHandler.h"

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
        std::string codeFilePath="";
        std::string metaDataPath="";
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

		AssetsHandler();
		void SearchAllAssets(std::filesystem::path path);
		void SaveMetadata();
		int assetCounter;

        int counter = 0;
        void CreateSingleAssetMetadata(std::filesystem::path path, std::string data);
        void LoadSingleAssetMetadata(std::filesystem::path path);

        template<typename T>
        void HandleAssetLoad(ISerializable<T>& iSerializableObj, std::string path , std::string fileType);
        friend ModelHandler;
        
		std::string assetFilepath;

	public:
		
		~AssetsHandler();
		void RegisterSaves();

        std::string assetFileExtension = ".cAST";
        std::string codeFileExtension = ".CODE";
        
		void AddAssetData(std::string path);
		AssetData& GetAssetData(std::string path);

		static AssetsHandler* GetInstance();

	};

    template<typename T>
    void AssetsHandler::HandleAssetLoad(ISerializable<T>& iSerializableObj, std::string path, std::string fileType) {
        nlohmann::json fileData = iSerializableObj.Serialize();
        std::filesystem::path filePath(path);
        filePath = filePath.parent_path();
        std::string name = filePath.filename().string();
        size_t extensionPos= name.find_last_of('.');
        name = name.substr(0, extensionPos);
        
        std::string metaFileName=filePath.string()+"\\"+ name + fileType;
        if (std::filesystem::exists(metaFileName)){
            std::cout<<"Loading asset from file: "<<metaFileName<<"\n";
            LoadSingleAssetMetadata(metaFileName);
        }else {
            std::cout<<"Creating asset from file: "<<metaFileName<<"\n";
            CreateSingleAssetMetadata(metaFileName, fileData.dump(4));
        }
    }
}
