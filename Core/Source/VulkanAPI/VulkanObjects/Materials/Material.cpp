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
            switch (pair.first) {
                case DIFFUSE:
                    materialUniform.diffuseOffset = texture->id;
                    break;
                case ROUGHNESS:
                    materialUniform.roughnessOffset = texture->id;
                    break;
                case METALLIC:
                    materialUniform.metallicOffset = texture->id;
                    break;
                case SPECULAR:
                    materialUniform.specularOffset = texture->id;
                    break;
                case NORMAL:
                    materialUniform.normalOffset = texture->id;
                    break;
            }
        }
        generated = true;
    }

    void Material::CalculateTextureOffsets(int &allTexturesOffset) {
        std::cout<<"MaterialTexturesAlready created for: "<<name<<"\n";
        materialUniform.textureIndexStart = allTexturesOffset;
        for (auto& pair : materialTextures)
        {
            pair.second->id = allTexturesOffset;
            materialUniform.texturesSizes++;
            allTexturesOffset++;
        }
        std::cout<<" New Texture sizes: "<<allTexturesOffset <<"\n";
    }
    void Material::SetTexture(TEXTURE_TYPE textureType, VKTexture *texture) {
        
        assert(texture->id>-1&&"A texture with invalid Id was tried to add");
        switch (textureType) {
            case DIFFUSE:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.diffuseOffset = texture->id;
                break;
            case ROUGHNESS:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.roughnessOffset = texture->id;
                break;
            case METALLIC:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.metallicOffset = texture->id;
                break;
            case SPECULAR:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.specularOffset = texture->id;
                break;
            case NORMAL:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.normalOffset = texture->id;
                break;
        }
        ModelHandler::GetInstance()->updateMaterialData = true;
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

        this->paths = jsonObj.at("Paths").get<std::map<TEXTURE_TYPE,std::string>>();
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
