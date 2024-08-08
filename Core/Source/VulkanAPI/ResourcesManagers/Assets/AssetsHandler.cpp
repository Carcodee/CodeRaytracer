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
            ModelHandler::GetInstance()->AddMaterial(ModelHandler::GetInstance()->materialBase);
            SearchAllAssets(fileHandlerInstance->GetAssetsPath());
            ModelHandler::GetInstance()->ReCalculateMaterialOffsets();
            
            
//            DeserializeCodeFile<Material>(ModelHandler::GetInstance()->allMaterialsOnApp);
//            DeserializeCodeFile<ModelData>(ModelHandler::GetInstance()->allModelsOnApp);
            std::cout << "MaterialOffset: " << ModelHandler::GetInstance()->currentMaterialsOffset<<"\n";
            std::cout << "Assets Founded: " << assetsLoaded.size() <<"\n";
            
        });
        

		
		
	}

	AssetsHandler::~AssetsHandler()
	{
//		SaveMetadata();
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
				else if (directoryPath.path().extension()== codeModelFileExtension ||directoryPath.path().extension()== matFileExtension)
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
//		SearchAllAssets(fileHandlerInstance->GetAssetsPath());
        int counter = 0;
        for(auto& pair: assetsLoaded){
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
            nlohmann::json jsonData;
            if (fileHandlerInstance->GetPathExtension(pair.first)==matFileExtension){
                Material& data = *ModelHandler::GetInstance()->allMaterialsOnApp.at(pair.second);
                jsonData = data.Serialize();
            }
            else if (fileHandlerInstance->GetPathExtension(pair.first)==codeModelFileExtension){
                ModelData& data = *ModelHandler::GetInstance()->allModelsOnApp.at(pair.second);
                jsonData = data.Serialize();
            }
            if(!jsonData.empty()){
                fileHandlerInstance->AppendToFile(pair.first, jsonData.dump(4));
            }
        }

        std::cout << "Asset Count saved: "<< counter<<"\n";




    }

	void AssetsHandler::RegisterSaves()
	{
		if (InputHandler::GetInstance()->GetUserInput(InputHandler::KEY_RIGHT_CONTROL, InputHandler::ACTION_HOLD)
		&& InputHandler::GetInstance()->GetUserInput(InputHandler::KEY_S, InputHandler::ACTION_DOWN)){
			SaveMetadata();
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


    
    void AssetsHandler::CreateSingleAssetMetadata(std::filesystem::path path, std::string data, int id) {
        assert(path != "" && "Asset must not be empty ");
        fileHandlerInstance->CreateFile(path.string(),data);
        assetsLoaded.try_emplace(path.string(), id);
    }

    
    void AssetsHandler::LoadSingleAssetMetadata(std::filesystem::path path) {

        if (!std::filesystem::exists(path)){
            std::cout<<"Couldn't open path to load data: "<<path<<"\n";
            return;
        }
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
                    if(path.extension()==matFileExtension){
                        Material mat{};
                        mat.Deserialize(currentJson);
                        ModelHandler::GetInstance()->allMaterialsOnApp.try_emplace(mat.id,std::make_shared<Material>(mat));
                        assetsLoaded.try_emplace(path.string(), mat.id);
                    }
                    else if(path.extension()==codeModelFileExtension){
                        ModelData modelData{};
                        modelData.Deserialize(currentJson);
                        assert(!ModelHandler::GetInstance()->allModelsOnApp.contains(modelData.id) &&"Two models contains the same id");
                        ModelHandler::GetInstance()->allModelsOnApp.try_emplace(modelData.id,std::make_shared<ModelData>(modelData));
                        assetsLoaded.try_emplace(path.string(), modelData.id);
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

    bool AssetsHandler::IsValidImageFormat(std::string extension) {
        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr"){
            return true;
        }else{
            return false;
        }
    }


}
