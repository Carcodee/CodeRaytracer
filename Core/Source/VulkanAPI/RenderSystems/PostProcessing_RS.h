//
// Created by carlo on 7/21/2024.
//
#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"

#ifndef EDITOR_POSTPROCESSING_RS_H
#define EDITOR_POSTPROCESSING_RS_H

namespace VULKAN {

    class PostProcessing_RS {
        struct SimpleImageData{
            VkImageView imageView;
            VkSampler sampler;
        };

    public:
        PostProcessing_RS(MyVulkanDevice& vulkanDevice, VulkanRenderer& vulkanRenderer);
        void CreateDescriptorSets();
        void CreatePipeline();
        void CreateBuffers();
        void InitRS(std::string vertPath, std::string fragPath);
        void Draw(VkCommandBuffer& currentCommandBuffer);
        void AddTextureImageToShader(VkImageView imageView, VkSampler sampler);

        std::vector<SimpleImageData> storageImages;
        VkRenderPass renderPassRef = VK_NULL_HANDLE;
        std::string vertexPath;
        std::string fragmentPath;
        
    private:
        VkDescriptorSetLayout descriptorSetLayout;
        std::vector<UIVertex>quadVertices;
        std::vector<uint16_t>quadIndices;
        MyVulkanDevice& myVulkanDevice;
        VulkanRenderer& myVulkanRenderer;
        std::unique_ptr<PipelineReader> pipelineReader;
        VkDescriptorPool postProPool = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets{};
        VkPipelineLayout pipelineLayout;

        Buffer vertexBuffer;
        Buffer indexBuffer;
        bool ready = false;
        
    };
    
} // VULKAN

#endif //EDITOR_POSTPROCESSING_RS_H
