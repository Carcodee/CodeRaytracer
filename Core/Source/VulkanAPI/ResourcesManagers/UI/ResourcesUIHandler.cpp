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
            if (fileHandlerInstanceRef->currentPathRelativeToAssets == fileHandlerInstanceRef->GetAssetsPath())
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
                if (ImGui::ImageButton((ImTextureID)ImguiRenderSystem::GetInstance()->folderThumbnail->textureDescriptor, ImVec2(iconSize, iconSize)))
                {
                    fileHandlerInstanceRef->currentPathRelativeToAssets = std::filesystem::path(element.path());
                }
                ImGui::Text(shortName.c_str());
                
                ImGui::NextColumn();
                colCounter++;
            }
            else if (element.path().extension()== assetInstanceRef->codeModelFileExtension)
            {
                if(AssetsHandler::GetInstance()->assetsLoaded.contains(element.path().string())){
                    if (ImGui::ImageButton((ImTextureID)ImguiRenderSystem::GetInstance()->modelThumbnail->textureDescriptor, ImVec2(iconSize, iconSize))){
                        pathInspected = element.path().string();
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        int modelId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                        ImGui::SetDragDropPayload("MODEL_PATH", &modelId,  sizeof(int));
                        ImGui::EndDragDropSource();
                    }
                    ImGui::Text(shortName.c_str());
                    colCounter++;
                    ImGui::NextColumn();
                }
            }
            else if (element.path().extension()== assetInstanceRef->matFileExtension) {
                if(AssetsHandler::GetInstance()->assetsLoaded.contains(element.path().string())){
                    int matId= AssetsHandler::GetInstance()->assetsLoaded.at(element.path().string());
                    assert(ModelHandler::GetInstance()->allMaterialsOnApp.contains(matId)&&"The material is not loaded and it should");
                    Material& matRef = *ModelHandler::GetInstance()->allMaterialsOnApp.at(matId);
                    if(matRef.materialTextures.contains(TEXTURE_TYPE::DIFFUSE)){
                        ImguiRenderSystem::GetInstance()->HandleTextureCreation(matRef.materialTextures.at(TEXTURE_TYPE::DIFFUSE));
                        if (ImGui::ImageButton((ImTextureID)matRef.materialTextures.at(TEXTURE_TYPE::DIFFUSE)->textureDescriptor, ImVec2(iconSize, iconSize))){
                            pathInspected = element.path().string();
                        }      
                    }else{
                        if (ImGui::Button("MAT", ImVec2(iconSize, iconSize))){
                            pathInspected = element.path().string();
                        }
                    }
                  
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        ImGui::SetDragDropPayload("MATERIAL_ID", &matId,  sizeof(int));
                        ImGui::EndDragDropSource();
                    }
                    ImGui::Text(shortName.c_str());
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
                if (modelDataRef.generated){
                    DisplayMeshInfo(modelDataRef);
                }
            } else if(extension == AssetsHandler::GetInstance()->matFileExtension){
                Material& materialRef = *ModelHandler::GetInstance()->allMaterialsOnApp.at(id);
                DisplayMatInfo(materialRef, ImVec2{100, 100});
            }
            
        } else {
            ImGui::SeparatorText("There is nothing inspected");
        }
        ImGui::End();
        ImGui::PopID();
    }

    void ResourcesUIHandler::DisplayMatInfo(Material &mat, ImVec2 iconSize) {
        {
            DisplayMatTexture(mat,DIFFUSE, iconSize, "Diffuse");
            DisplayMatTexture(mat,NORMAL, iconSize, "Normal");
            DisplayMatTexture(mat,METALLIC, iconSize, "Metallic");
            DisplayMatTexture(mat,ROUGHNESS, iconSize, "Roughness");
            DisplayMatTexture(mat,METALLICROUGHNESS, iconSize, "Metallic_Roughness");
            DisplayMatTexture(mat,EMISSIVE, iconSize, "Emissive");
        }
        {
            if(ImGui::SliderFloat("roughness",&mat.materialUniform.roughnessIntensity, 0.0f, 3.0f,"%.3f")){
                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if(ImGui::SliderFloat("reflectivity",&mat.materialUniform.reflectivityIntensity, 0.0f, 3.0f,"%.3f")){

                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if(ImGui::SliderFloat("metalness",&mat.materialUniform.metallicIntensity, 0.0f, 3.0f,"%.3f")){

                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            if(ImGui::SliderFloat("emission base",&mat.materialUniform.emissionIntensity, 0.0f, 14.0f,"%.3f")){

                ModelHandler::GetInstance()->updateMaterialData= true;
            }
            float baseReflection[3];
            baseReflection[0]= mat.materialUniform.baseReflection.x;
            baseReflection[1]= mat.materialUniform.baseReflection.y;
            baseReflection[2]= mat.materialUniform.baseReflection.z;
            if(ImGui::SliderFloat3("Base reflectivity",baseReflection, 0.0f, 1.0f,"%.3f")){
                mat.materialUniform.baseReflection.x=baseReflection[0];
                mat.materialUniform.baseReflection.y=baseReflection[1];
                mat.materialUniform.baseReflection.z=baseReflection[2];
                ModelHandler::GetInstance()->updateMaterialData= true;
            }

            float matCol[3];
            matCol[0]= mat.materialUniform.diffuseColor.x;
            matCol[1]= mat.materialUniform.diffuseColor.y;
            matCol[2]= mat.materialUniform.diffuseColor.z;
            if (ImGui::ColorEdit3("Diffuse", matCol)){
                mat.materialUniform.diffuseColor= glm::make_vec3(matCol);
                ModelHandler::GetInstance()->updateMaterialData= true;
            }

            if(ImGui::SliderFloat("Albedo Intensity",&mat.materialUniform.albedoIntensity, 0.0f, 3.0f,"%.3f")){

                ModelHandler::GetInstance()->updateMaterialData= true;
            }


        }
    }

    void ResourcesUIHandler::DisplayMeshInfo(ModelData &modelData) {

        std::filesystem::path refPath(modelData.pathToAssetReference);
        std::string text = "Mesh Selected: " + refPath.filename().string();
        ImGui::SeparatorText(text.c_str());
        if (ImGui::SliderFloat3("Position", positionInspected,-10.0f , 10.0f, "%.3f")){
            modelData.bottomLevelObjRef->pos = glm::make_vec3(positionInspected);
            modelData.bottomLevelObjRef->UpdateMatrix();
            ModelHandler::GetInstance()->updateBottomLevelObj = true;
        }
        if(ImGui::SliderFloat3("Rotation", rotationInspected,0.0f , 360.0f, "%.3f")){
            modelData.bottomLevelObjRef->rot = glm::make_vec3(rotationInspected);
            modelData.bottomLevelObjRef->UpdateMatrix();
            ModelHandler::GetInstance()->updateBottomLevelObj = true;
        }
        if(ImGui::SliderFloat3("Scale", scaleInspected,-10.0f , 10.0f, "%.3f")){
            modelData.bottomLevelObjRef->scale = glm::make_vec3(scaleInspected);
            modelData.bottomLevelObjRef->UpdateMatrix();
            ModelHandler::GetInstance()->updateBottomLevelObj = true;
        }
        static float overallScale = 1.0f;
        if(ImGui::SliderFloat("Scale 3 axis", &overallScale,-10.0f , 10.0f, "%.3f")){
            modelData.bottomLevelObjRef->scale = glm::vec3(overallScale);
            modelData.bottomLevelObjRef->UpdateMatrix();
            ModelHandler::GetInstance()->updateBottomLevelObj = true;
        }

        for (int i = 0; i < modelData.meshCount; ++i) {

            std::string text = "Mesh_" + std::to_string(i);
            if (ImGui::TreeNode(text.c_str())) {
                int matIndex =modelData.materialIds[i];
                Material &materialReference = *ModelHandler::GetInstance()->allMaterialsOnApp.at(matIndex);
                std::string materialText = materialReference.name;
                ImGui::PushID(materialReference.id);
                if (materialReference.materialTextures.contains(TEXTURE_TYPE::DIFFUSE)){
                    ImguiRenderSystem::GetInstance()->HandleTextureCreation(materialReference.materialTextures.at(TEXTURE_TYPE::DIFFUSE));
                    ImGui::ImageButton(((ImTextureID)materialReference.materialTextures.at(TEXTURE_TYPE::DIFFUSE)->textureDescriptor), ImVec2{100, 100});
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
                        ModelHandler::GetInstance()->updateBottomLevelObj = true;
                        ImGui::EndTooltip();
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
                DisplayMatInfo(materialReference, ImVec2{35, 35});
                
                ImGui::TreePop();
            }
        }
    }

    void ResourcesUIHandler::DisplayBLASesInfo() {

        ImGui::SetWindowSize(ImVec2(400, 400));
        ImGui::Begin("BLASes");
        ImGui::SeparatorText("Meshes loaded:");

        static Sphere* sphereSelected= nullptr;
        static int modelSelectedKey= -1;
        
        for (auto& model :ModelHandler::GetInstance()->allModelsOnApp) {
            if(!model.second->generated)continue;
            
            std::filesystem::path modelPath(model.second.get()->pathToAssetReference);
            if(modelSelectedKey == -1){
                if (ImGui::Selectable(modelPath.filename().string().c_str())){
                    modelSelectedKey = model.first;
                    sphereSelected = nullptr;
                }
            }else{
                if (ImGui::Selectable(modelPath.filename().string().c_str(), modelSelectedKey == model.first)){
                    modelSelectedKey = model.first;
                    sphereSelected = nullptr;
                }               
            }

        }
        for (auto& sphere :ModelHandler::GetInstance()->allSpheresOnApp) {
            int id =sphere.id;
            std::string name = "Sphere: " + std::to_string(id);
            if (sphereSelected == nullptr){
                if (ImGui::Selectable(name.c_str())){
                    sphereSelected = &sphere;
                    modelSelectedKey = -1;
                }               
            }else{
                if (ImGui::Selectable(name.c_str(), sphereSelected == &sphere)){
                    sphereSelected = &sphere;
                    modelSelectedKey = -1;
                }               
            }

        }
        if(modelSelectedKey!= -1 && sphereSelected == nullptr){
            assert(ModelHandler::GetInstance()->allModelsOnApp.contains(modelSelectedKey) &&"Model is not in allModels map");
            DisplayMeshInfo(*ModelHandler::GetInstance()->allModelsOnApp.at(modelSelectedKey).get());

        }
        else if (modelSelectedKey== -1 && sphereSelected != nullptr){
            DisplayMeshInfo(*sphereSelected);
        }else{

        }


        ImGui::End();
        

    }

    void ResourcesUIHandler::HandleDrop(TEXTURE_TYPE textureType, Material &mat) {
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("TEXTURE_ID")) {
                VKTexture* data = *(VKTexture **)payload->Data;
                mat.SetTexture(textureType, data);
                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                ImGui::BeginTooltip();
                ImGui::Text("Set Material");
                ImGui::EndTooltip();
            }
            ImGui::EndDragDropTarget();
        }
    }

    void ResourcesUIHandler::HandleDrag(TEXTURE_TYPE textureType, Material& mat) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
//            int id= mat.GetTexOffsetFromTexture(textureType);
//            assert(id>-1&&"there is no texture to drag");
            VKTexture* tex= mat.materialTextures.at(textureType);
            ImGui::SetDragDropPayload("TEXTURE_ID", &tex,  sizeof(tex));
            ImGui::EndDragDropSource();
        }

    }

    void ResourcesUIHandler::HandleDrag(VKTexture *texture) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("TEXTURE_ID", &texture,  sizeof(texture));
            ImGui::EndDragDropSource();
        }

    }
    void ResourcesUIHandler::DisplayTexturesTab() {
        ImGui::SetWindowSize(ImVec2(400, 400));
        ImGui::Begin("Textures");
        int counter = 0;
        for (auto& texturesOnApp: ModelHandler::GetInstance()->allTexturesOnApp) {
            std::filesystem::path textPath(texturesOnApp.second->path);
            std::string counterText = "Texture: "+ textPath.filename().string();
            ImGui::SeparatorText(counterText.c_str());
            ImguiRenderSystem::GetInstance()->HandleTextureCreation(texturesOnApp.second.get());
            ImGui::ImageButton((ImTextureID)texturesOnApp.second->textureDescriptor, ImVec2(100,100));
            HandleDrag(texturesOnApp.second.get());
            counter++;
        }
        ImGui::End();
    }

    void ResourcesUIHandler::DisplayMeshInfo(Sphere &sphereData) {
        int id =sphereData.id;
        std::string name = "Sphere: " + std::to_string(id);
        std::string namePos = "Position: " + std::to_string(id);
        std::string nameRadius = "Radius: " + std::to_string(id);

        ImGui::SeparatorText(name.c_str());
        float pos [3] = {sphereData.sphereUniform.pos.x, sphereData.sphereUniform.pos.y, sphereData.sphereUniform.pos.z};
        if (ImGui::SliderFloat3(namePos.c_str(), pos,-10.0f , 10.0f, "%.3f")){
            sphereData.sphereUniform.pos = glm::make_vec3(pos);
            ModelHandler::GetInstance()->updateBottomLevelObj = true;
        }
        if(ImGui::SliderFloat(nameRadius.c_str(), &sphereData.sphereUniform.radius,-10.0f , 10.0f, "%.3f")){
            ModelHandler::GetInstance()->updateBottomLevelObj = true;
        }

        Material& material = *ModelHandler::GetInstance()->allMaterialsOnApp.at(sphereData.sphereUniform.matId).get();
        ImGui::PushID(material.id);
        std::string materialText = material.name;
        if (material.materialTextures.contains(TEXTURE_TYPE::DIFFUSE)){
            ImguiRenderSystem::GetInstance()->HandleTextureCreation(material.materialTextures.at(TEXTURE_TYPE::DIFFUSE));
            ImGui::ImageButton(((ImTextureID)material.materialTextures.at(TEXTURE_TYPE::DIFFUSE)->textureDescriptor), ImVec2{100, 100});
        } else{
            ImGui::Button(materialText.c_str(), ImVec2{100, 100});
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("MATERIAL_ID")) {
                int data = *(int *) payload->Data;

                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                ImGui::BeginTooltip();
                ImGui::Text("Set Material");
                sphereData.sphereUniform.matId = data;
                ModelHandler::GetInstance()->updateBottomLevelObj = true;
                ImGui::EndTooltip();
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();
        DisplayMatInfo(material, ImVec2{50, 50});
    }

    void ResourcesUIHandler::DisplayViewportFrameBuffers(std::vector<VKTexture *> framebuffers) {

        assert(!framebuffers.empty()&&"It must be at least one framebuffer to display");
        ImGui::SetWindowSize(ImVec2(400, 400));
        ImGui::Begin("Framebuffers");
        ImVec2 size = ImGui::GetContentRegionAvail();
        float ySize = size.y/static_cast<float>(framebuffers.size());
        for (auto& framebuffer: framebuffers) {
            ImguiRenderSystem::GetInstance()->HandleTextureCreation(framebuffer);
            ImGui::Image((ImTextureID) framebuffer->textureDescriptor, ImVec2{size.x,ySize});
        }

        ImGui::End();

    }

    void ResourcesUIHandler::DisplayMatTexture(Material &mat, TEXTURE_TYPE textureType, ImVec2& iconSize,std::string texName) {
        ImGui::SeparatorText(texName.c_str());
        ImGui::PushID(texName.c_str());
        if (!mat.materialTextures.empty()&& mat.materialTextures.contains(textureType)){
            ImguiRenderSystem::GetInstance()->HandleTextureCreation(mat.materialTextures.at(textureType));
            ImGui::ImageButton(((ImTextureID)mat.materialTextures.at(textureType)->textureDescriptor), iconSize);
            HandleDrag(textureType,mat);

        } else{
            if (ImGui::Button(mat.name.c_str(), iconSize)){
                ImGui::OpenPopup("Set Texture");
            }
        }
        HandleDrop(textureType,mat);
        ImGui::PopID();
        if(mat.materialTextures.contains(textureType)){
            if (ImGui::Button("Remove Texture")){
                mat.RemoveTexture(textureType);
            }
        }

    }


}

