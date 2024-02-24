#pragma once

#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"
#include "VulkanAPI/VulkanObjects/Textures/VKTexture.h"

#include <vector>
#include <chrono>
#include <memory>
#include <array>

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


    class MyDescriptorSets
    {

    public:
        MyDescriptorSets(MyVulkanDevice& myVkDevice) :
            myDevice(myVkDevice)
        {
        }
        ~MyDescriptorSets();


        VkDescriptorSetLayoutBinding CreateDescriptorBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, int bindingCount, int descriptorCount);

        template<std::size_t N>
        void CreateLayoutBinding(std::array<VkDescriptorSetLayoutBinding, N>& bindings, int DescriptorSetCount);


        template<typename BufferObject>
        void CreateUniformBuffers(int descriptorCount, int  MaxFramesInFlight);

        template<typename BufferObject>
        void UpdateUniformBuffer(uint32_t currentImage, int descriptorCount);

        template <typename BufferObject, std::size_t N>
        void CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight);
        template <typename BufferObject, std::size_t N>
        void CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight,VKTexture& texture);


        template<std::size_t N>
        void CreateDescriptorPool(std::array<VkDescriptorSetLayoutBinding, N>& bindings, int maxFramesInFlight);


        MyVulkanDevice& myDevice;
        std::vector<DescriptorSetGroup> descriptorData;
        std::vector <VkDescriptorSetLayout> descriptorSetLayout;
        VkDescriptorPool descriptorPool;


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
    inline void MyDescriptorSets::CreateDescriptorPool(std::array<VkDescriptorSetLayoutBinding, N>& bindings, int maxFramesInFlight)
    {
        std::array<VkDescriptorPoolSize, N> poolSize{};
        for (size_t i = 0; i < N; i++)
        {
            poolSize[i].type = bindings[i].descriptorType;
            poolSize[i].descriptorCount = static_cast<uint32_t>(maxFramesInFlight)*2;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
        poolInfo.pPoolSizes = poolSize.data();

        poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);

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
    void MyDescriptorSets::UpdateUniformBuffer(uint32_t currentImage, int descriptorCount)
    {

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


        // if (typeid(BufferObject)==typeid(UniformBufferObjectData))
        // {
        BufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.projection = glm::perspective(glm::radians(45.0f), 800 / (float)600, 0.1f, 10.0f);
        ubo.projection[1][1] *= -1;
        // }


        memcpy(descriptorData[descriptorCount - 1].uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    template <typename BufferObject, std::size_t N>
    void MyDescriptorSets::CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight)
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
                descriptorWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite[j].descriptorCount = descriptorCount;
                descriptorWrite[j].pImageInfo = nullptr; // Optional
                descriptorWrite[j].pTexelBufferView = nullptr; // Optional
     

            }


            vkUpdateDescriptorSets(myDevice.device(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
        }

    }
    template <typename BufferObject, std::size_t N>
    void MyDescriptorSets::CreateDescriptorSets(std::array<VkDescriptorSetLayoutBinding, N> bindings, int descriptorCount, int maxFramesInFlight,
        VKTexture& texture)
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


            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.textureImageView;
            imageInfo.sampler = texture.textureSampler;


            std::array<VkWriteDescriptorSet, N> descriptorWrite{};
            for (size_t j = 0; j < descriptorWrite.size(); j++)
            {
                if (bindings[j].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                {
                    descriptorWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite[j].dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
                    descriptorWrite[j].dstBinding = j;
                    descriptorWrite[j].dstArrayElement = 0;
                    descriptorWrite[j].pBufferInfo = &bufferInfo;
                    descriptorWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrite[j].descriptorCount = descriptorCount;
                    descriptorWrite[j].pImageInfo = nullptr; // Optional
                    descriptorWrite[j].pTexelBufferView = nullptr; // Optional
                }
                if (bindings[j].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                {
                    descriptorWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite[j].dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
                    descriptorWrite[j].dstBinding = j;
                    descriptorWrite[j].dstArrayElement = 0;
                    descriptorWrite[j].pBufferInfo = &bufferInfo;
                    descriptorWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite[j].descriptorCount = descriptorCount;
                    descriptorWrite[j].pImageInfo = &imageInfo;
                    descriptorWrite[j].pTexelBufferView = nullptr; // Optional
                }

            }


            vkUpdateDescriptorSets(myDevice.device(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
        }


    }
}

