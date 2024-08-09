//
// Created by carlo on 7/26/2024.
//

#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include "VulkanAPI/DevicePipeline/Vulkan_Device.h"
#include "VulkanAPI/Renderer/VulkanRenderer.h"
#include "VulkanAPI/VulkanPipeline/PipelineReader.h"

#ifndef EDITOR_BLOOM_RS_H
#define EDITOR_BLOOM_RS_H


namespace VULKAN{

    class Bloom_RS{

        struct SimpleImageData {
            VkImageView imageView;
            VkSampler sampler;
        };
        
    public:
        
        Bloom_RS(MyVulkanDevice& vulkanDevice,VulkanRenderer &vulkanRenderer);
        void CreateDescriptorSets();
        void CreatePipelineDownSample();
        void CreatePipelineUpSample();
        void CreateBuffers();
        void InitRS();
        void Draw(VkCommandBuffer &currentCommandBuffer);
        void AddTextureImageToShader(VkImageView imageView, VkSampler sampler);

        std::vector<SimpleImageData> storageImages;
        VkRenderPass upSampleRenderPassRef = VK_NULL_HANDLE;
        VkRenderPass downSampleRenderPassRef = VK_NULL_HANDLE;
        std::string vertexPath;
        std::string fragmentPathDownSample;
        std::string fragmentPathUpSample;
        PushConstantBlock_Bloom pc;

    private:
        VkDescriptorSetLayout descriptorSetLayout;
        std::vector<UIVertex> quadVertices;
        std::vector<uint16_t> quadIndices;
        MyVulkanDevice &myVulkanDevice;
        VulkanRenderer &myVulkanRenderer;
        std::unique_ptr<PipelineReader> downSamplePipelineReader;
        std::unique_ptr<PipelineReader> upSamplePipelineReader;
        VkDescriptorPool postProPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptors;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets{};
        VkPipelineLayout pipelineLayout;

        Buffer vertexBuffer;
        Buffer indexBuffer;
        bool ready = false;

    };

}

#endif //EDITOR_BLOOM_RS_H
