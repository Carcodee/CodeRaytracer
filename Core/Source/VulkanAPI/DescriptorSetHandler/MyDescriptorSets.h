#pragma once

#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"

#include <vector>
#include <chrono>
#include <memory>

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

        // Consider adding a destructor for clean-up of Vulkan resources
        // ~DescriptorSetGroup()
        // {
        //     
        // }

        // You may also want to add methods for initializing, updating, and cleaning up the resources
    };
    class MyDescriptorSets
    {

    public:
        MyDescriptorSets(MyVulkanDevice& myVkDevice) :
        myDevice(myVkDevice)
        {
        }
        //MyDescriptorSets(const MyDescriptorSets&) = delete;
        //MyDescriptorSets& operator=(const MyDescriptorSets&) = delete;
        ~MyDescriptorSets();
        

        
        void CreateLayoutBinding(int binding, int descriptorCount);
        
        
        template<typename BufferObject>
        void CreateUniformBuffers( int descriptorCount,int  MaxFramesInFlight);

        template<typename BufferObject>
        void UpdateUniformBuffer(uint32_t currentImage, int descriptorCount);

        template <typename BufferObject>
        void CreateDescriptorSets(int descriptorCount, int setSize, int maxFramesInFlight);

        void CreateDescriptorPool(int descriptorCount, int setSize, int maxFramesInFlight);

        
        MyVulkanDevice& myDevice;
        std::vector<DescriptorSetGroup> descriptorData;
        std::vector <VkDescriptorSetLayout> descriptorSetLayout;
        VkDescriptorPool descriptorPool;



    };

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
            ubo.projection = glm::perspective(glm::radians(45.0f), 800/ (float)600, 0.1f, 10.0f);
            ubo.projection[1][1] *= -1;
        // }


        memcpy(descriptorData[descriptorCount - 1].uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    template <typename BufferObject>
    void MyDescriptorSets::CreateDescriptorSets(int descriptorCount, int setSize, int maxFramesInFlight)
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

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorData[descriptorCount - 1].descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(myDevice.device(), 1, &descriptorWrite, 0, nullptr);
        }
    }
}

