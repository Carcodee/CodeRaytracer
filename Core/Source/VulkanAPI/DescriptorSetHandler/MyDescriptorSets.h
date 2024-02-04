#pragma once

#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/SwapChain/VulkanSwap_chain.hpp"

#include <vector>
#include <chrono>

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
        VkDescriptorPool descriptorPool;
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
        MyDescriptorSets(MyVulkanDevice& myVkDevice, VulkanSwapChain& swapChain) :
        myDevice(myVkDevice), mySwapChain(swapChain)
        {
        }
        MyDescriptorSets(const MyDescriptorSets&) = delete;
        MyDescriptorSets& operator=(const MyDescriptorSets&) = delete;
        ~MyDescriptorSets();
        

        
        void CreateLayoutBinding(int binding, int descriptorCount);
        
        
        template<typename BufferObject>
        void CreateUniformBuffers(BufferObject bufferObject, int descriptorCount);

        template<typename BufferObject>
        void UpdateUniformBuffer(uint32_t currentImage, int descriptorCount, BufferObject bufferObject);

        void CreateDescriptorPool(int descriptorCount, int setSize);

        void CreateDescriptorSets(int descriptorCount, int setSize);
        
        MyVulkanDevice& myDevice;
        VulkanSwapChain& mySwapChain;
        std::vector<DescriptorSetGroup> descriptorData;
        std::vector <VkDescriptorSetLayout> descriptorSetLayout;
        


    };

    template <typename BufferObject>
    void MyDescriptorSets::CreateUniformBuffers(BufferObject bufferObject, int descriptorCount)
    {

            VkDeviceSize bufferSize = sizeof(bufferObject);

            descriptorData[descriptorCount].uniformBuffers.resize(mySwapChain.MAX_FRAMES_IN_FLIGHT);
            descriptorData[descriptorCount].uniformBuffersMemory.resize(mySwapChain.MAX_FRAMES_IN_FLIGHT);
            descriptorData[descriptorCount].uniformBuffersMapped.resize(mySwapChain.MAX_FRAMES_IN_FLIGHT);
		
            for (size_t j = 0; j < mySwapChain.MAX_FRAMES_IN_FLIGHT ; j++)
            {
                myDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    descriptorData[descriptorCount].uniformBuffers[j], descriptorData[descriptorCount].uniformBuffersMemory[j]);
			
                vkMapMemory(myDevice.device(), descriptorData[descriptorCount].uniformBuffersMemory[j], 0, bufferSize, 0, &descriptorData[descriptorCount].uniformBuffersMapped[j]);

            }
        
    }

    template <typename BufferObject>
    void MyDescriptorSets::UpdateUniformBuffer(uint32_t currentImage, int descriptorCount, BufferObject bufferObject)
    {

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


        // if (typeid(BufferObject)==typeid(UniformBufferObjectData))
        // {
            BufferObject ubo{};
            ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.projection = glm::perspective(glm::radians(45.0f), mySwapChain.width()/ (float)mySwapChain.height(), 0.1f, 10.0f);
            ubo.projection[1][1] *= -1;
        // }


        memcpy(descriptorData[descriptorCount].uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }
}

