//
// Created by carlo on 7/20/2024.
//

#include "Material.h"
#include "VulkanAPI/Model/ModelHandler.h"

namespace VULKAN{

    void Material::CreateTextures(VulkanSwapChain &swap_chain, int &allTexturesOffset) {
        std::cout<<"MaterialTexturesAlready created for: "<<name<<"\n";
        if(generated)return;
        materialUniform.textureIndexStart = allTexturesOffset;
        for (auto& pair: paths)
        {
            if (!std::filesystem::exists(pair.second.c_str()))continue;
            materialUniform.texturesSizes++;
            VKTexture* texture = new VKTexture(pair.second.c_str(), swap_chain);
            materialTextures.try_emplace(pair.first,texture);
            ModelHandler::GetInstance()->allTexturesOnApp.try_emplace(allTexturesOffset,std::make_shared<VKTexture>(*texture));
            allTexturesOffset++;
        }
        generated = true;
        std::cout<<" New Texture sizes: "<<allTexturesOffset <<"\n";
    }

    void Material::CalculateTextureOffsets(int &allTexturesOffset) {
        std::cout<<"MaterialTexturesAlready created for: "<<name<<"\n";
        materialUniform.textureIndexStart = allTexturesOffset;
        for (int i = 0; i < materialTextures.size(); ++i)
        {
            materialUniform.texturesSizes++;
            allTexturesOffset++;
        }
        std::cout<<" New Texture sizes: "<<allTexturesOffset <<"\n";
    }

    int Material::GetTexOffsetFromTexture(TEXTURE_TYPE textureType) {
        int idToCheck= 0;
        switch (textureType) {
            case DIFFUSE:
                idToCheck=this->materialUniform.diffuseOffset;
                break;
            case ROUGHNESS:
                idToCheck=this->materialUniform.roughnessOffset;
                break;
            case METALLIC:
                idToCheck=this->materialUniform.metallicOffset;
                break;
            case SPECULAR:
                idToCheck=this->materialUniform.specularOffset;
                break;
            case NORMAL:
                idToCheck=this->materialUniform.normalOffset;
                break;
        }
        return idToCheck;
    }

    void Material::SetTexture(TEXTURE_TYPE textureType, VKTexture *texture) {
        switch (textureType) {
            case DIFFUSE:
                if (materialUniform.diffuseOffset == -1){
                    int offset = this->materialTextures.size() + 1;
                    materialUniform.diffuseOffset = offset;
                    this->materialTextures.try_emplace(offset, texture);
                }else{
                    materialTextures.at(materialUniform.diffuseOffset) = texture;
                }
                break;
            case ROUGHNESS:
                if (materialUniform.roughnessOffset == -1){
                    int offset = this->materialTextures.size() + 1;
                    materialUniform.roughnessOffset = offset;
                    this->materialTextures.try_emplace(offset, texture);
                }else{
                    materialTextures.at(materialUniform.roughnessOffset) = texture;
                }
                break;
            case METALLIC:
                if (materialUniform.metallicOffset == -1){
                    int offset = this->materialTextures.size() + 1;
                    materialUniform.metallicOffset = offset;
                    this->materialTextures.try_emplace(offset, texture);
                }else{
                    materialTextures.at(materialUniform.metallicOffset) = texture;
                }
                break;
            case SPECULAR:
                if (materialUniform.specularOffset == -1){
                    int offset = this->materialTextures.size() + 1;
                    materialUniform.specularOffset = offset;
                    this->materialTextures.try_emplace(offset, texture);
                }else{
                    materialTextures.at(materialUniform.specularOffset) = texture;
                }
                break;
            case NORMAL:
                if (materialUniform.normalOffset == -1){
                    int offset = this->materialTextures.size() + 1;
                    materialUniform.normalOffset = offset;
                    this->materialTextures.try_emplace(offset, texture);
                }else{
                    materialTextures.at(materialUniform.normalOffset) = texture;
                }
                break;
        }
    }

    Material Material::Deserialize(nlohmann::json &jsonObj) {

        this->id = jsonObj.at("ID");
        this->name = jsonObj.at("Name");
        std::vector<int> diffuse;
        std::vector<int> baseReflection;
        this->materialUniform.albedoIntensity = jsonObj.at("AlbedoIntensity");
        this->materialUniform.normalIntensity = jsonObj.at("NormalIntensity");
        this->materialUniform.specularIntensity = jsonObj.at("SpecularIntensity");
        this->materialUniform.roughnessIntensity = jsonObj.at("RoughnessIntensity");
        //16
        diffuse = jsonObj.at("DiffuseColor").get<std::vector<int>>();
        this->materialUniform.diffuseColor.x = diffuse[0];
        this->materialUniform.diffuseColor.y = diffuse[1];
        this->materialUniform.diffuseColor.z = diffuse[2];
        this->materialUniform.reflectivityIntensity = jsonObj.at("ReflectivityIntensity");
        //32

        //base reflection
        baseReflection = jsonObj.at("BaseReflection").get<std::vector<int>>();
        this->materialUniform.baseReflection.x = baseReflection[0];
        this->materialUniform.baseReflection.y = baseReflection[1];
        this->materialUniform.baseReflection.z = baseReflection[2];
        this->materialUniform.metallicIntensity=jsonObj.at("MetallicIntensity");
        //48
        this->materialUniform.emissionIntensity==jsonObj.at("EmissionIntensity");
        this->materialUniform.roughnessOffset = jsonObj.at("RoughnessOffset");
        this->materialUniform.metallicOffset = jsonObj.at("MetallicOffset");
        this->materialUniform.specularOffset= jsonObj.at("SpecularOffset");
        //60
        this->materialUniform.textureIndexStart=jsonObj.at("TextureIndexStart");
        this->materialUniform.texturesSizes = 0;
//                this->materialUniform.texturesSizes = jsonObj.at("TextureSizes");
        this->materialUniform.diffuseOffset = jsonObj.at("DiffuseOffset");
        this->materialUniform.normalOffset = jsonObj.at("NormalOffset");
        //80

        this->paths = jsonObj.at("Paths").get<std::map<int,std::string>>();
        this->textureReferencePath = jsonObj.at("TextureReferencePath");
        this->targetPath = jsonObj.at("TargetPath");
        return *this;
    }

    nlohmann::json Material::Serialize() {
        nlohmann::json jsonData;
        std::vector<float> diffuse;
        diffuse.push_back(materialUniform.diffuseColor.x);
        diffuse.push_back(materialUniform.diffuseColor.y);
        diffuse.push_back(materialUniform.diffuseColor.z);
        std::vector<float> baseReflection;
        baseReflection.push_back(materialUniform.baseReflection.x);
        baseReflection.push_back(materialUniform.baseReflection.y);
        baseReflection.push_back(materialUniform.baseReflection.z);

        jsonData = {
                {"ID",this->id},
                {"Name",this->name},
                {"AlbedoIntensity",this->materialUniform.albedoIntensity},
                {"NormalIntensity",this->materialUniform.normalIntensity},
                {"SpecularIntensity",this->materialUniform.specularIntensity},
                {"RoughnessIntensity",this->materialUniform.roughnessIntensity},
                //16
                {"DiffuseColor",diffuse},
                {"ReflectivityIntensity", this->materialUniform.reflectivityIntensity},
                //32
                {"BaseReflection",baseReflection},
                {"MetallicIntensity",this->materialUniform.metallicIntensity},
                //48
                {"EmissionIntensity",this->materialUniform.emissionIntensity},
                {"RoughnessOffset",this->materialUniform.roughnessOffset},
                {"MetallicOffset",this->materialUniform.metallicOffset},
                {"SpecularOffset",this->materialUniform.specularOffset},
                //60
                {"TextureIndexStart",this->materialUniform.textureIndexStart},
                {"TextureSizes",this->materialUniform.texturesSizes},
                {"DiffuseOffset",this->materialUniform.diffuseOffset},
                {"NormalOffset",this->materialUniform.normalOffset},
                //80
                {"Paths",this->paths},
                {"TextureReferencePath",this->textureReferencePath},
                {"TargetPath",this->targetPath}
        };
        return jsonData;
//
    }
}
