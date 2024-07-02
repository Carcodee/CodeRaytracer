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
		ImGui::SetWindowSize(ImVec2(400, 400));
		ImGuiID dockSpaceId = ImGui::GetID("DockSpace Demo");
		ImGui::DockSpace(dockSpaceId, ImVec2(0, 0),ImGuiWindowFlags_NoMove);
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


		ImGui::Columns(columns, "Columns", false);
		int colCounter = 0;

		for (auto element : std::filesystem::directory_iterator(fileHandlerInstanceRef->currentPathRelativeToAssets))
		{
			ImGui::PushID(colCounter);
			std::string path = element.path().string();
			AssetData asset = assetInstanceRef->GetAssetData(path);
			size_t shortNamePos = path.find_last_of("\\");
			std::string shortName = path.erase(0, shortNamePos + 1);
			if (element.is_directory())
			{
				if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize)))
				{
					fileHandlerInstanceRef->currentPathRelativeToAssets = std::filesystem::path(element.path());
				}

				ImGui::NextColumn();
			}
			else if (asset.assetType == FILE || asset.assetType == MODEL || asset.assetType == IMAGE)
			{
				
				if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize)))
				{
					std::cout << "Opening: " << asset.absolutePath << "\n";
					std::cout << "Id: " << asset.assetId << "\n";
					std::cout << "assetType: " << asset.assetType << "\n";
					std::cout << "size: " << asset.sizeInBytes << "\n";
					std::cout << "extensionType: " << asset.extensionType << "\n";
				}
				if (asset.assetType==MODEL)
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						// Set payload to carry the index of our item (could be anything)
						std::filesystem::path relPath(std::filesystem::relative(asset.absolutePath,fileHandlerInstanceRef->GetAssetsPath()));
						std::string stringData = relPath.string();
						const char* data = stringData.c_str();

						ImGui::SetDragDropPayload("MODEL_PATH",&data, strlen(data) * sizeof(char));
						
						// Display preview (could be anything, e.g. when dragging an image we could decide to display
						ImGui::EndDragDropSource();
					}
				}

				ImGui::NextColumn();
			}

			ImGui::PopID();
						
		}
		ImGui::Columns(1);

		ImGui::NewLine();
		ImGui::SliderFloat("icons", &iconSize, 16.0f, 256.0f, "%.3f");
		ImGui::SliderFloat("thumbnailSpace", &thumbnailSpace,0.0f , 100.0f, "%.3f");

		ImGui::End();
	}
}
