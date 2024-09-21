//
// Created by carlo on 7/20/2024.
//

#include "Material.h"
#include "VulkanAPI/Model/ModelHandler.h"
#include "FileSystem/FileHandler.h"

namespace VULKAN{

    void Material::CreateTextures(VulkanSwapChain &swap_chain, int &allTexturesOffset) {
        std::cout<<"MaterialTexturesAlready created for: "<<name<<"\n";
        if(generated)return;
        for (auto& pair: paths)
        {
            if (!std::filesystem::exists(pair.second.c_str()))continue;
            std::string extension = HELPERS::FileHandler::GetInstance()->GetPathExtension(pair.second);
            
            if (extension != ".png" && extension != ".jpeg" && extension != ".jpg")continue;
            VKTexture* texture = new VKTexture(pair.second.c_str(), swap_chain, true);
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
                case EMISSIVE:
                    materialUniform.emissionOffset = texture->id;
                    break;
                case NORMAL:
                    materialUniform.normalOffset = texture->id;
                    break;
                case METALLICROUGHNESS:
                    materialUniform.metallicRoughnessOffset = texture->id;
                    break;
            }
        }
        generated = true;
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
            case EMISSIVE:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.emissionOffset = texture->id;
                break;
            case NORMAL:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.normalOffset = texture->id;
                break;
            case METALLICROUGHNESS:
                if (!materialTextures.contains(textureType)){
                    this->materialTextures.try_emplace(textureType, texture);
                }else{
                    materialTextures.at(textureType) = texture;
                }
                materialUniform.metallicRoughnessOffset = texture->id;
                break;
        }
        ModelHandler::GetInstance()->updateMaterialData = true;
        ModelHandler::GetInstance()->updateBottomLevelObj = true;
    }

    Material Material::Deserialize(nlohmann::json &jsonObj) {

        this->id = jsonObj.at("ID");
        this->name = jsonObj.at("Name");
        std::vector<float> diffuse;
        std::vector<float> baseReflection;
        this->materialUniform.albedoIntensity = jsonObj.at("AlbedoIntensity");
        this->materialUniform.normalIntensity = jsonObj.at("NormalIntensity");
        this->materialUniform.roughnessIntensity = jsonObj.at("RoughnessIntensity");
        diffuse = jsonObj.at("DiffuseColor").get<std::vector<float>>();
        this->materialUniform.diffuseColor.x = diffuse[0];
        this->materialUniform.diffuseColor.y = diffuse[1];
        this->materialUniform.diffuseColor.z = diffuse[2];
        this->materialUniform.diffuseColor.w = diffuse[3];
        baseReflection = jsonObj.at("BaseReflection").get<std::vector<float>>();
        this->materialUniform.baseReflection.x = baseReflection[0];
        this->materialUniform.baseReflection.y = baseReflection[1];
        this->materialUniform.baseReflection.z = baseReflection[2];
        this->materialUniform.metallicIntensity=jsonObj.at("MetallicIntensity");
        this->materialUniform.emissionIntensity = jsonObj.at("EmissionIntensity");
        this->materialUniform.roughnessOffset = jsonObj.at("RoughnessOffset");
        this->materialUniform.metallicOffset = jsonObj.at("MetallicOffset");
        this->materialUniform.emissionOffset= jsonObj.at("EmissionOffset");
        this->materialUniform.metallicRoughnessOffset=jsonObj.at("MetallicRoughnessOffset");
        this->materialUniform.alphaCutoff = jsonObj.at("AlphaCutoff");
        this->materialUniform.diffuseOffset = jsonObj.at("DiffuseOffset");
        this->materialUniform.normalOffset = jsonObj.at("NormalOffset");
        this->materialUniform.configurations = jsonObj.at("Configurations");

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
        diffuse.push_back(materialUniform.diffuseColor.w);
        std::vector<float> baseReflection;
        baseReflection.push_back(materialUniform.baseReflection.x);
        baseReflection.push_back(materialUniform.baseReflection.y);
        baseReflection.push_back(materialUniform.baseReflection.z);

        jsonData = {
                {"ID",this->id},
                {"Name",this->name},
                {"AlbedoIntensity",this->materialUniform.albedoIntensity},
                {"NormalIntensity",this->materialUniform.normalIntensity},
                {"RoughnessIntensity",this->materialUniform.roughnessIntensity},
                {"DiffuseColor",diffuse},
                {"BaseReflection",baseReflection},
                {"MetallicIntensity",this->materialUniform.metallicIntensity},
                {"EmissionIntensity",this->materialUniform.emissionIntensity},
                {"RoughnessOffset",this->materialUniform.roughnessOffset},
                {"MetallicOffset",this->materialUniform.metallicOffset},
                {"EmissionOffset",this->materialUniform.emissionOffset},
                {"MetallicRoughnessOffset",this->materialUniform.metallicRoughnessOffset},
                {"AlphaCutoff",this->materialUniform.alphaCutoff},
                {"DiffuseOffset",this->materialUniform.diffuseOffset},
                {"NormalOffset",this->materialUniform.normalOffset},
                {"Configurations",this->materialUniform.configurations},
                {"Paths",this->paths},
                {"TextureReferencePath",this->textureReferencePath},
                {"TargetPath",this->targetPath}
        };
        return jsonData;
//
    }

    void Material::RemoveTexture(TEXTURE_TYPE textureType) {
        if (!materialTextures.contains(textureType)){
            return;
        }
        switch (textureType) {
            case DIFFUSE:
                materialUniform.diffuseOffset = -1;
                break;
            case ROUGHNESS:
                materialUniform.roughnessOffset= -1;
                break;
            case METALLIC:
                materialUniform.metallicOffset = -1;
                break;
            case EMISSIVE:
                materialUniform.emissionOffset = -1;
                break;
            case NORMAL:
                materialUniform.normalOffset = -1;
                break;
            case METALLICROUGHNESS:
                materialUniform.metallicRoughnessOffset = -1;
                break;
        }
        materialTextures.erase(textureType);
        ModelHandler::GetInstance()->updateMaterialData = true;
    }

    void Material::SetConfiguration(int configData) {
        materialUniform.configurations = configData;
    }

    bool Material::GetConfigVal(CONFIG_TYPE configType) {
        bool val;
        uint32_t currentConfigs = materialUniform.configurations & 0x0000000F;
        switch (configType) {
            case ALPHA_AS_DIFFUSE:
                val = (currentConfigs & (1 << 0)) != 0;
                break;
            case USE_ALPHA_CHANNEL:
                val = (currentConfigs & (1 << 1)) != 0;
                break;
            case USE_ALPHA_OF_DIFFUSE_COLOR:
                val = (currentConfigs & (1 << 2)) != 0;
                break;
            case USE_DISNEY_BSDF:
                val = (currentConfigs & (1 << 3)) != 0;
                break;
            case THIN:
                val = (currentConfigs & (1 << 4)) != 0;
                break;
            default:
                val = false;
                assert(val && "Provide a valid configuration type");
        }
        return val;
    }

    void Material::SetConfigVal(CONFIG_TYPE configType, bool value) {
        uint32_t maskValue = 0;
        switch (configType) {
            case ALPHA_AS_DIFFUSE:
                maskValue = 1 << 0;
                break;
            case USE_ALPHA_CHANNEL:
                maskValue  = 1 << 1;
                break;
            case USE_ALPHA_OF_DIFFUSE_COLOR:
                maskValue  = 1 << 2;
                break;
            case USE_DISNEY_BSDF:
                maskValue  = 1 << 3;
                break;
            case THIN:
                maskValue  = 1 << 4;
                break;
        }
        if (value){
            materialUniform.configurations |= maskValue;
        }else{
            materialUniform.configurations &= ~maskValue;
        }

    }
}
