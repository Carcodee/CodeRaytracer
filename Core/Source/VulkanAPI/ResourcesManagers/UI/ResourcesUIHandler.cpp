#include "ResourcesUIHandler.h"

#include <iostream>

#include "imgui.h"
#include "FileSystem/FileHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"
#include "VulkanAPI/RenderSystems/ImguiRenderSystem.h"

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
			else if (element.path().extension()== assetInstanceRef->codeModelFileExtension)
			{
                if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize))){
                    int modelId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                    ModelData& modelToLookInto= *ModelHandler::GetInstance()->allModelsOnApp.at(modelId).get();
                    modelIDInspected=modelToLookInto.id; 
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {

                    int modelId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                    ModelData& modelToLookInto= *ModelHandler::GetInstance()->allModelsOnApp.at(modelId).get();
//                     Set payload to carry the index of our item (could be anything)
//                    std::filesystem::path relPath(std::filesystem::relative(modelToLookInto.pathToAssetReference,fileHandlerInstanceRef->GetAssetsPath()));

//                    std::string stringData = relPath.string();
//                    const char* data = stringData.c_str();
//                    ImGui::SetDragDropPayload("MODEL_PATH", data,  strlen(data) + 1 * sizeof(char));
                    ImGui::SetDragDropPayload("MODEL_PATH", &modelId,  sizeof(int));
//                     Display preview (could be anything, e.g. when dragging an image we could decide to display
                    ImGui::EndDragDropSource();
                }
				colCounter++;
				ImGui::NextColumn();
			}
            else if (element.path().extension()== assetInstanceRef->matFileExtension) {
                int matId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                Material &materialToLookInto = *ModelHandler::GetInstance()->allMaterialsOnApp.at(matId);
                if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize))){
                    
                    materialIDInspected= materialToLookInto.id;
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {

                    int matId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                    Material& material= *ModelHandler::GetInstance()->allMaterialsOnApp.at(matId).get();
//                     Set payload to carry the index of our item (could be anything)
//                    std::filesystem::path relPath(std::filesystem::relative(modelToLookInto.pathToAssetReference,fileHandlerInstanceRef->GetAssetsPath()));

//                    std::string stringData = relPath.string();
//                    const char* data = stringData.c_str();
//                    ImGui::SetDragDropPayload("MODEL_PATH", data,  strlen(data) + 1 * sizeof(char));
                    ImGui::SetDragDropPayload("MATERIAL_ID", &matId,  sizeof(int));
//                     Display preview (could be anything, e.g. when dragging an image we could decide to display
                    ImGui::EndDragDropSource();
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

    void ResourcesUIHandler::DisplayMeshInfo() {

        ImGui::PushID("Inspector");
        ImGui::SetWindowSize(ImVec2(400, 400));
        ImGui::Begin("Inspector");
        
        if (modelIDInspected !=-1){

            ModelData& modelInspected = *ModelHandler::GetInstance()->allModelsOnApp.at(modelIDInspected);
            std::filesystem::path refPath(modelInspected.pathToAssetReference);
            std::string text = "Mesh Selected: " + refPath.filename().string();
            ImGui::SeparatorText(text.c_str());
            
            for (int i = 0; i < modelInspected.meshCount; ++i) {

                std::string text = "Mesh_" + std::to_string(i);
                if (ImGui::TreeNode(text.c_str())) {
                    int matIndex =modelInspected.materialIds[i];
                    Material &materialReference = *ModelHandler::GetInstance()->allMaterialsOnApp.at(matIndex);
                    std::string materialText = materialReference.name;
                    ImGui::PushID(materialReference.id);

                    if (!materialReference.materialTextures.empty()){
                        ImguiRenderSystem::GetInstance()->HandleTextureCreation(materialReference.materialTextures[0]);
                        ImGui::ImageButton(((ImTextureID)materialReference.materialTextures[0]->textureDescriptor), ImVec2{100, 100});
                    } else{
                        ImGui::Button(materialText.c_str(), ImVec2{100, 100});
                    }
                    
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("MATERIAL_ID")) {
                            int data = *(int *) payload->Data;

                            ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                            ImGui::BeginTooltip();
                            ImGui::Text("Set Material");
                            modelInspected.materialIds[i] = data;
                            ModelHandler::GetInstance()->updateMeshData = true;
                            ImGui::EndTooltip();
                        }
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::PopID();
                    if(ImGui::SliderFloat("albedo intensity",&materialReference.materialUniform.albedoIntensity, 0.0f, 3.0f,"%.3f")){
                        ModelHandler::GetInstance()->updateMaterialData= true;
                    }
                    if(ImGui::SliderFloat("roughness",&materialReference.materialUniform.roughnessIntensity, 0.0f, 3.0f,"%.3f")){
                        ModelHandler::GetInstance()->updateMaterialData= true;
                    }
                    if(ImGui::SliderFloat("reflectivity",&materialReference.materialUniform.reflectivityIntensity, 0.0f, 3.0f,"%.3f")){

                        ModelHandler::GetInstance()->updateMaterialData= true;
                    }
                    if(ImGui::SliderFloat("metalness",&materialReference.materialUniform.metallicIntensity, 0.0f, 3.0f,"%.3f")){

                        ModelHandler::GetInstance()->updateMaterialData= true;
                    }
                    if(ImGui::SliderFloat("emission base",&materialReference.materialUniform.emissionIntensity, 0.0f, 3.0f,"%.3f")){

                        ModelHandler::GetInstance()->updateMaterialData= true;
                    }
                    float baseReflection[3];
                    baseReflection[0]= materialReference.materialUniform.baseReflection.x;
                    baseReflection[1]= materialReference.materialUniform.baseReflection.y;
                    baseReflection[2]= materialReference.materialUniform.baseReflection.z;
                    if(ImGui::SliderFloat3("metalness",baseReflection, 0.0f, 1.0f,"%.3f")){
                         materialReference.materialUniform.baseReflection.x=baseReflection[0];
                         materialReference.materialUniform.baseReflection.y=baseReflection[1];
                         materialReference.materialUniform.baseReflection.z=baseReflection[2];
                         ModelHandler::GetInstance()->updateMaterialData= true;
                    }
                    if  (materialReference.materialUniform.texturesSizes<=0){
                        float matCol[3];
                        matCol[0]= materialReference.materialUniform.diffuseColor.x;
                        matCol[1]= materialReference.materialUniform.diffuseColor.y;
                        matCol[2]= materialReference.materialUniform.diffuseColor.z;
                        ImGui::ColorEdit3("Diffuse", matCol);
                    }


                    ImGui::TreePop();
                }
            }
        } else{

            ImGui::SeparatorText("There is no Model selected");
        }
        
        
        ImGui::End();
        ImGui::PopID();
    }
}
