//
// Created by carlo on 7/20/2024.
//
#include <iostream>
#include <glm/glm.hpp>
#include "VulkanAPI/Utility/ISerializable.h"
#include "VulkanAPI/VulkanObjects/Textures/VKTexture.h"

#ifndef EDITOR_MATERIAL_H
#define EDITOR_MATERIAL_H

namespace VULKAN{

    enum TEXTURE_TYPE {
        DIFFUSE,
        ROUGHNESS,
        METALLIC,
        SPECULAR,
        NORMAL
    };

    struct MaterialUniformData
    {
        float albedoIntensity;
        float normalIntensity;
        float specularIntensity;
        float roughnessIntensity = 0.5f;
        glm::vec3 diffuseColor;
        float reflectivityIntensity = 0.5f;
        //32
        glm::vec3 baseReflection;
        float metallicIntensity;
        //48
        float emissionIntensity;
        int roughnessOffset = -1;
        int metallicOffset = -1;
        int specularOffset = -1;
        //64
        int textureIndexStart = -1;
        int texturesSizes = 0;
        int diffuseOffset = -1;
        int normalOffset = -1;
        //80
    };

    struct Material: ISerializable<Material>
		{
            MaterialUniformData materialUniform{};
            //the key is the texture offset in order correlate them 
			std::map<int,std::string> paths;
			std::map<int,VKTexture*> materialTextures;
            std::string textureReferencePath="";
            std::string name="";
            std::string targetPath="";
            bool generated = false;
            int id= 0;
			void CreateTextures(VulkanSwapChain& swap_chain, int& allTexturesOffset);
            void CalculateTextureOffsets(int& allTexturesOffset);
            int GetTexOffsetFromTexture(TEXTURE_TYPE textureType);
            void SetTexture(TEXTURE_TYPE textureType,VKTexture* texture);
            ~Material(){
                for (auto& pair : materialTextures) {
                    delete pair.second;
                }
            };
            nlohmann::json Serialize() override;
            Material Deserialize(nlohmann::json &jsonObj) override;
            void SaveData() override{
                
            }
		};

}
#endif //EDITOR_MATERIAL_H
