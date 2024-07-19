#include "ResourcesUIHandler.h"

#include <iostream>

#include "imgui.h"
#include "FileSystem/FileHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"
#include "VulkanAPI/RenderSystems/ImguiRenderSystem.h"
#include "VulkanAPI/Utility/InputSystem/InputHandler.h"

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
        if (ImGui::IsWindowHovered() && InputHandler::GetInstance()->GetUserInput(InputHandler::BUTTON_MOUSE1, InputHandler::ACTION_DOWN)){
            ImGui::OpenPopup("ContextMenu");
        }
        if (ImGui::BeginPopup("ContextMenu"))
        {
            ImGui::SeparatorText("Options");
            if (ImGui::Selectable("Add Material")){
                ModelHandler::GetInstance()->CreateMaterial(fileHandlerInstanceRef->currentPathRelativeToAssets.string());
            }
            ImGui::EndPopup();
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
                if(AssetsHandler::GetInstance()->assetsLoaded.contains(element.path().string())){
                    if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize))){
                        pathInspected = element.path().string();
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        int modelId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                        ImGui::SetDragDropPayload("MODEL_PATH", &modelId,  sizeof(int));
                        ImGui::EndDragDropSource();
                    }
                    colCounter++;
                    ImGui::NextColumn();
                }
            }
            else if (element.path().extension()== assetInstanceRef->matFileExtension) {
                if(AssetsHandler::GetInstance()->assetsLoaded.contains(element.path().string())){
                    if (ImGui::Button(shortName.c_str(), ImVec2(iconSize, iconSize))){
                        pathInspected = element.path().string();
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        int matId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                        ImGui::SetDragDropPayload("MATERIAL_ID", &matId,  sizeof(int));
                        ImGui::EndDragDropSource();
                    }
                    colCounter++;
                    ImGui::NextColumn();
                }
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

    void ResourcesUIHandler::DisplayInspectorInfo() {

        ImGui::PushID("Inspector");
        ImGui::SetWindowSize(ImVec2(400, 400));
        ImGui::Begin("Inspector");

        if(AssetsHandler::GetInstance()->assetsLoaded.contains(pathInspected)) {
            std::string extension = HELPERS::FileHandler::GetInstance()->GetPathExtension(pathInspected);
            int id = AssetsHandler::GetInstance()->assetsLoaded.at(pathInspected);
            if(extension == AssetsHandler::GetInstance()->codeModelFileExtension){
                ModelData& modelDataRef = *ModelHandler::GetInstance()->allModelsOnApp.at(id);
                DisplayMeshInfo(modelDataRef);
            } else if(extension == AssetsHandler::GetInstance()->matFileExtension){
                Material& materialRef = *ModelHandler::GetInstance()->allMaterialsOnApp.at(id);
                DisplayMatInfo(materialRef);
            }
            
        } else {
            ImGui::SeparatorText("There is nothing inspected");
        }
        ImGui::End();
        ImGui::PopID();
    }

    void ResourcesUIHandler::DisplayMatInfo(Material &mat) {
        {
            ImGui::SeparatorText("Diffuse");
            if (!mat.materialTextures.empty()&& mat.materialUniform.diffuseOffset>-1){
                ImguiRenderSystem::GetInstance()->HandleTextureCreation(mat.materialTextures.at(mat.materialUniform.diffuseOffset));
                ImGui::ImageButton(((ImTextureID)mat.materialTextures.at(mat.materialUniform.diffuseOffset)->textureDescriptor), ImVec2{100, 100});
            } else{
                ImGui::Button(mat.name.c_str(), ImVec2{100, 100});
            }
            
            ImGui::SeparatorText("Normal");
            if (!mat.materialTextures.empty()&& mat.materialUniform.normalOffset>-1){
                ImguiRenderSystem::GetInstance()->HandleTextureCreation(mat.materialTextures.at(mat.materialUniform.normalOffset));
                ImGui::ImageButton(((ImTextureID)mat.materialTextures.at(mat.materialUniform.normalOffset)->textureDescriptor), ImVec2{100, 100});
            } else{
                ImGui::Button(mat.name.c_str(), ImVec2{100, 100});
            }
            
            ImGui::SeparatorText("Roughness");
            if (!mat.materialTextures.empty()&& mat.materialUniform.roughnessOffset>-1){
                ImguiRenderSystem::GetInstance()->HandleTextureCreation(mat.materialTextures.at(mat.materialUniform.roughnessOffset));
                ImGui::ImageButton(((ImTextureID)mat.materialTextures.at(mat.materialUniform.roughnessOffset)->textureDescriptor), ImVec2{100, 100});
            } else{
                ImGui::Button(mat.name.c_str(), ImVec2{100, 100});
            }
        }
        {
            if(ImGui::SliderFloat("albedo intensity",&mat.materialUniform.albedoIntensity, 0.0f, 3.0f,"%.3f")){
                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if(ImGui::SliderFloat("roughness",&mat.materialUniform.roughnessIntensity, 0.0f, 3.0f,"%.3f")){
                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if(ImGui::SliderFloat("reflectivity",&mat.materialUniform.reflectivityIntensity, 0.0f, 3.0f,"%.3f")){

                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if(ImGui::SliderFloat("metalness",&mat.materialUniform.metallicIntensity, 0.0f, 3.0f,"%.3f")){

                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if(ImGui::SliderFloat("emission base",&mat.materialUniform.emissionIntensity, 0.0f, 3.0f,"%.3f")){

                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            float baseReflection[3];
            baseReflection[0]= mat.materialUniform.baseReflection.x;
            baseReflection[1]= mat.materialUniform.baseReflection.y;
            baseReflection[2]= mat.materialUniform.baseReflection.z;
            if(ImGui::SliderFloat3("metalness",baseReflection, 0.0f, 1.0f,"%.3f")){
                mat.materialUniform.baseReflection.x=baseReflection[0];
                mat.materialUniform.baseReflection.y=baseReflection[1];
                mat.materialUniform.baseReflection.z=baseReflection[2];
                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if  (mat.materialUniform.texturesSizes<=0){
                float matCol[3];
                matCol[0]= mat.materialUniform.diffuseColor.x;
                matCol[1]= mat.materialUniform.diffuseColor.y;
                matCol[2]= mat.materialUniform.diffuseColor.z;
                ImGui::ColorEdit3("Diffuse", matCol);
            }

        }
    }

    void ResourcesUIHandler::DisplayMeshInfo(ModelData &modelData) {

        ModelData& materialReference = *ModelHandler::GetInstance()->allModelsOnApp.at(modelData.id);
        std::filesystem::path refPath(modelData.pathToAssetReference);
        std::string text = "Mesh Selected: " + refPath.filename().string();
        ImGui::SeparatorText(text.c_str());

        for (int i = 0; i < modelData.meshCount; ++i) {

            std::string text = "Mesh_" + std::to_string(i);
            if (ImGui::TreeNode(text.c_str())) {
                int matIndex =modelData.materialIds[i];
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
                        modelData.materialIds[i] = data;
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
    }

}

