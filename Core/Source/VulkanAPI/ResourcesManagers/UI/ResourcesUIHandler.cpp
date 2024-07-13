#include "ResourcesUIHandler.h"

#include <iostream>

#include "imgui.h"
#include "FileSystem/FileHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"

namespace VULKAN
{
	ResourcesUIHandler* ResourcesUIHandler::instance = nullptr;
	HELPERS::FileHandler* fileHandlerInstanceRef;
	AssetsHandler* assetInstanceRef;

	ResourcesUIHandler::ResourcesUIHandler()
	{
		fileHandlerInstanceRef = HELPERS::FileHandler::GetInstance();
		assetInstanceRef= AssetsHandler::GetInstance();

		
	}

	ResourcesUIHandler* ResourcesUIHandler::GetInstance()
	{
		if (instance== nullptr)
		{
			
			instance = new ResourcesUIHandler;
		}
		return instance;
	}

	void ResourcesUIHandler::DisplayDirInfo()
	{
		ImGui::PushID("AssetsID");
		ImGui::SetWindowSize(ImVec2(400, 400));
		//ImGuiID dockSpaceId = ImGui::GetID("DockSpace Demo");
		//ImGui::DockSpace(dockSpaceId, ImVec2(0, 0),ImGuiWindowFlags_NoMove);
		ImGui::Begin("Assets");

		float panelWidth = ImGui::GetContentRegionAvail().x;

		float cellSize = thumbnailSpace + iconSize;
		columns = int(panelWidth/cellSize);

		//ImGui::SetNextWindowPos(ImVec2(0,0));
		//ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		if(ImGui::Button("<-", ImVec2(20, 20)))
		{
			if (fileHandlerInstanceRef->currentPathRelativeToAssets == fileHandlerInstanceRef->GetAssetsPath() )
			{
			}
			else
			{
				fileHandlerInstanceRef->currentPathRelativeToAssets = fileHandlerInstanceRef->currentPathRelativeToAssets.parent_path();
			}
		}


        if (columns==0){
            columns = 1;
        }
		ImGui::Columns(columns, "Columns", false);
//        ImGui::Columns(1, "Columns", false);
		int colCounter = 0;

		for (auto element : std::filesystem::directory_iterator(fileHandlerInstanceRef->currentPathRelativeToAssets))
		{
			ImGui::PushID(colCounter);
			std::string path = element.path().string();
			size_t shortNamePos = path.find_last_of("\\");
			std::string shortName = path.erase(0, shortNamePos + 1);
			if (element.is_directory())
			{
				if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize)))
				{
					fileHandlerInstanceRef->currentPathRelativeToAssets = std::filesystem::path(element.path());
				}

				ImGui::NextColumn();
				colCounter++;
			}
			else if (element.path().extension()== assetInstanceRef->assetFileExtension)
			{
                
                AssetData& asset = assetInstanceRef->GetAssetData(element.path().string());
                
                if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize)))
                {
                    std::cout << "Id: " << asset.assetId << "\n";
                    std::cout << "assetType: " << asset.assetType << "\n";
                    std::cout << "size: " << asset.sizeInBytes << "\n";
                    std::cout << "extensionType: " << asset.extensionType << "\n";
                }
				if (asset.extensionType== AssetsHandler::GetInstance()->codeModelFileExtension)
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
                        
                        ModelData& modelToLookInto= *ModelHandler::GetInstance()->allModelsOnApp.at(asset.codeFilePath).get();
						// Set payload to carry the index of our item (could be anything)
						std::filesystem::path relPath(std::filesystem::relative(modelToLookInto.pathToAssetReference,fileHandlerInstanceRef->GetAssetsPath()));
                        
						std::string stringData = relPath.string();
						const char* data = stringData.c_str();
						ImGui::SetDragDropPayload("MODEL_PATH", data,  strlen(data) + 1 * sizeof(char));
						// Display preview (could be anything, e.g. when dragging an image we could decide to display
						ImGui::EndDragDropSource();
					}
				}
                if (asset.extensionType== AssetsHandler::GetInstance()->matFileExtension)
                {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        // Set payload to carry the index of our item (could be anything)
                        
                        Material& materialToLookInto= ModelHandler::GetInstance()->GetMaterialFromPath(asset.codeFilePath);
//                        std::filesystem::path relPath(std::filesystem::relative(materialToLookInto.materialReferencePath,fileHandlerInstanceRef->GetAssetsPath()));
//                        std::string stringData = relPath.string();
//                        const char* data = stringData.c_str();
//                        ImGui::SetDragDropPayload("MODEL_PATH", data,  strlen(data) + 1 * sizeof(char));
                        // Display preview (could be anything, e.g. when dragging an image we could decide to display
//                        ImGui::EndDragDropSource();
                    }
                }
				colCounter++;

				ImGui::NextColumn();
			}

			ImGui::PopID();
						
		}
		ImGui::Columns(1);

		ImGui::NewLine();
		ImGui::SliderFloat("icons", &iconSize, 16.0f, 256.0f, "%.3f");
		ImGui::SliderFloat("thumbnailSpace", &thumbnailSpace,0.0f , 100.0f, "%.3f");

		ImGui::End();

		ImGui::PopID();
	}
}
