#pragma once

#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"
#include "VulkanAPI/VulkanObjects/Textures/VKTexture.h"

#include <vector>
#include <chrono>
#include <memory>
#include <array>

#include "VulkanAPI/Camera/Camera.h"

namespace VULKAN
{
    struct UniformBufferObjectData {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };
    struct DescriptorSetGroup {
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;
        std::vector<VkDescriptorSet> descriptorSets;

    };
    struct Desc_SetGroup
    {
	    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		VkDescriptorSetLayout descriptorSetLayout;  
		VkDescriptorPool descriptorPool;
		std::vector<VkWriteDescriptorSet> descriptorWrite;
	};


    class MyDescriptorSets
    {

    public:
        MyDescriptorSets(MyVulkanDevice& myVkDevice) :
            myDevice(myVkDevice)
        {
        }
        //~MyDescriptorSets()=delete;


//NEW ABSTRACTED FUNCTIONS
    	void CreateDeletors()
        {
            for (size_t i = 0; i < descriptorData.size(); i++) {
                for (size_t j = 0; j < 2; j++) {
                    if (descriptorData[i].uniformBuffers.size() != 0)
                    {
                        myDevice.deletionQueue.push_function([this, i, j]()
                            {
                                vkDestroyBuffer(myDevice.device(), descriptorData[i].uniformBuffers[j], nullptr);
                            });
                        myDevice.deletionQueue.push_function([this, i, j]()
                            {
                                vkFreeMemory(myDevice.device(), descriptorData[i].uniformBuffersMemory[j], nullptr);
                            });
                    }
                }
            }
          //  myDevice.deletionQueue.push_function([this]() {vkDestroyDescriptorPool(myDevice.device(), descriptorPool, nullptr);});
            for (size_t i = 0; i < descriptorSetLayout.size(); i++)
            {
                myDevice.deletionQueue.push_function([this, i]()
                    {
                        if (descriptorSetLayout[i] == nullptr)return;
                        vkDestroyDescriptorSetLayout(myDevice.device(), descriptorSetLayout[i], nullptr);
                    });
            }

        }
        template<std::size_t N>
        void CreateLayoutBinding(std::array<VkDescriptorSetLayoutBinding, N>& bindings, int DescriptorSetCount);

        VkDescriptorSetLayoutBinding CreateDescriptorBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, int bindingCount, int descriptorCount);
//OLD ABSTRACTED FUNCTIONS
        template<typename BufferObject>
        void CreateUniformBuffers(int descriptorCount, int  MaxFramesInFlight);

        template<typename BufferObject>
        void UpdateUniformBuffer(uint32_t currentImage, int descriptorCount, float speed);

        template <typename BufferObject, std::size_t N>
        void CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight, VkDescriptorType descriptorType);
        template <typename BufferObject, std::size_t N>
        void CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight,VKTexture& texture,VKTexture& texture2,VKTexture& texture3, VkDescriptorType descriptorType);

    	void CreateDescriptorSets(int descriptorCount, int maxFramesInFlight);

        template<std::size_t N>
        void CreateDescriptorPool(std::array<VkDescriptorSetLayoutBinding, N>& bindings, int maxFramesInFlight, VkStructureType type);


        MyVulkanDevice& myDevice;
        std::vector<DescriptorSetGroup> descriptorData;
        std::vector <VkDescriptorSetLayout> descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        std::vector<VkWriteDescriptorSet>descriptorWrite{};
        Camera cam{ glm::vec3(2.0f, 2.0f, 2.0f) };
    };


    template<std::size_t N>
    inline void MyDescriptorSets::CreateLayoutBinding(std::array<VkDescriptorSetLayoutBinding, N>& bindings, int DescriptorSetCount)
    {

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(myDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout[DescriptorSetCount - 1]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

    }

    template<std::size_t N>
    inline void MyDescriptorSets::CreateDescriptorPool(std::array<VkDescriptorSetLayoutBinding, N>& bindings, int maxFramesInFlight, VkStructureType type)
    {
        std::array<VkDescriptorPoolSize, N> poolSize{};
        for (size_t i = 0; i < N; i++)
        {
            poolSize[i].type = bindings[i].descriptorType;
            if (poolSize[i].type==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
				poolSize[i].descriptorCount = static_cast<uint32_t>(maxFramesInFlight)*2;
                continue;   
            }
            poolSize[i].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = type;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
        poolInfo.pPoolSizes = poolSize.data();

        poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight)*2;

        if (vkCreateDescriptorPool(myDevice.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    template <typename BufferObject>
    void MyDescriptorSets::CreateUniformBuffers(int descriptorCount, int MaxFramesInFlight)
    {

        VkDeviceSize bufferSize = sizeof(BufferObject);

        descriptorData[descriptorCount - 1].uniformBuffers.resize(MaxFramesInFlight);
        descriptorData[descriptorCount - 1].uniformBuffersMemory.resize(MaxFramesInFlight);
        descriptorData[descriptorCount - 1].uniformBuffersMapped.resize(MaxFramesInFlight);

        for (size_t j = 0; j < MaxFramesInFlight; j++)
        {
            myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                descriptorData[descriptorCount - 1].uniformBuffers[j], descriptorData[descriptorCount - 1].uniformBuffersMemory[j]);

            vkMapMemory(myDevice.device(), descriptorData[descriptorCount - 1].uniformBuffersMemory[j], 0, bufferSize, 0, &descriptorData[descriptorCount - 1].uniformBuffersMapped[j]);

        }

    }



    template <typename BufferObject>
    void MyDescriptorSets::UpdateUniformBuffer(uint32_t currentImage, int descriptorCount, float speed)
    {

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        // if (typeid(BufferObject)==typeid(UniformBufferObjectData))
        // {
        BufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f)* speed, glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.projection[1][1] *= -1;

  
        cam.currentMode = CameraMode::E_Free;
        cam.SetPerspective(45.0f, (float)800 / (float)600, 0.1f, 512.0f);
        //cam.position = (glm::vec3(1.5f, 1.5f, 1.0f));

        cam.SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        ubo.view = cam.matrices.view;
        ubo.projection = cam.matrices.perspective;
        ubo.projection[1][1] *= -1;
        // }

        memcpy(descriptorData[descriptorCount - 1].uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    template <typename BufferObject, std::size_t N>
    void MyDescriptorSets::CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight, VkDescriptorType descriptorType)
    {
        std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout[descriptorCount - 1]);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();

        descriptorData[descriptorCount - 1].descriptorSets.resize(maxFramesInFlight);
        if (vkAllocateDescriptorSets(myDevice.device(), &allocInfo, descriptorData[descriptorCount - 1].descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        for (size_t i = 0; i < maxFramesInFlight; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = descriptorData[descriptorCount - 1].uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(BufferObject);

            std::array<VkWriteDescriptorSet, N> descriptorWrite{};
            for (size_t j = 0; j < descriptorWrite.size(); j++)
            {
                descriptorWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite[j].dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
                descriptorWrite[j].dstBinding = j;
                descriptorWrite[j].dstArrayElement = 0;
                descriptorWrite[j].pBufferInfo = &bufferInfo;
                descriptorWrite[j].descriptorType = descriptorType;
                descriptorWrite[j].descriptorCount = descriptorCount;
                descriptorWrite[j].pImageInfo = nullptr; // Optional
                descriptorWrite[j].pTexelBufferView = nullptr; // Optional

            }


            vkUpdateDescriptorSets(myDevice.device(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
        }
        CreateDeletors();
    }



    template <typename BufferObject, std::size_t N>
    void MyDescriptorSets::CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight,
        VKTexture& texture,VKTexture& texture2,VKTexture& texture3, VkDescriptorType descriptorType)
    {
        std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout[descriptorCount - 1]);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();

        descriptorData[descriptorCount - 1].descriptorSets.resize(maxFramesInFlight);
        if (vkAllocateDescriptorSets(myDevice.device(), &allocInfo, descriptorData[descriptorCount - 1].descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        for (size_t i = 0; i < maxFramesInFlight; i++)
        {
        	std::array<VkWriteDescriptorSet, N> descriptorWrite{};

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = descriptorData[descriptorCount - 1].uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(BufferObject);


            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.textureImageView;
			imageInfo.sampler = texture.textureSampler;

            VkDescriptorImageInfo imageInfo2{};
            imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo2.imageView = texture2.textureImageView;
            imageInfo2.sampler = texture2.textureSampler;

            VkDescriptorImageInfo imageInfo3{};
            imageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo3.imageView = texture3.textureImageView;
            imageInfo3.sampler = texture3.textureSampler;


            descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[0].dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
            descriptorWrite[0].dstBinding = 0;
            descriptorWrite[0].dstArrayElement = 0;
            descriptorWrite[0].pBufferInfo = &bufferInfo;
            descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite[0].descriptorCount = descriptorCount;
            descriptorWrite[0].pImageInfo = nullptr; // Optional
            descriptorWrite[0].pTexelBufferView = nullptr; // Optional


            descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[1].dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
            descriptorWrite[1].dstBinding = 1;
            descriptorWrite[1].dstArrayElement = 0;
            descriptorWrite[1].pBufferInfo = &bufferInfo;
            descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[1].descriptorCount = descriptorCount;
            descriptorWrite[1].pImageInfo = &imageInfo;
            descriptorWrite[1].pTexelBufferView = nullptr; // Optional

            descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[2].dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
            descriptorWrite[2].dstBinding = 2;
            descriptorWrite[2].dstArrayElement = 0;
            descriptorWrite[2].pBufferInfo = &bufferInfo;
            descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[2].descriptorCount = descriptorCount;
            descriptorWrite[2].pImageInfo = &imageInfo2;
            descriptorWrite[2].pTexelBufferView = nullptr; // Optional
                       
            descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[3].dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
            descriptorWrite[3].dstBinding = 3;
            descriptorWrite[3].dstArrayElement = 0;
            descriptorWrite[3].pBufferInfo = &bufferInfo;
            descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[3].descriptorCount = descriptorCount;
            descriptorWrite[3].pImageInfo = &imageInfo3;
            descriptorWrite[3].pTexelBufferView = nullptr; // Optional
                       


            


            vkUpdateDescriptorSets(myDevice.device(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
        }

    }


}

