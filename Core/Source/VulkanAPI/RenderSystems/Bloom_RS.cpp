//
// Created by carlo on 7/26/2024.
//

#include "Bloom_RS.h"
#include "FileSystem/FileHandler.h"

namespace VULKAN {

    Bloom_RS::Bloom_RS(MyVulkanDevice &vulkanDevice, VulkanRenderer& vulkanRenderer): myVulkanDevice (vulkanDevice), myVulkanRenderer(vulkanRenderer)
    {
        quadVertices = UIVertex::GetQuad();
        quadIndices = UIVertex::GetQuadIndices();
    }

    void Bloom_RS::CreateDescriptorSets() {
        std::vector<VkDescriptorPoolSize> poolSize =
                {
                        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
                };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
        poolInfo.pPoolSizes = poolSize.data();
        poolInfo.maxSets = 10;

        vkCreateDescriptorPool(myVulkanDevice.device(), &poolInfo, nullptr, &postProPool);

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =INITIALIZERS::descriptorSetAllocateInfo(postProPool, &descriptorSetLayout, 1);
        descriptors.reserve(storageImages.size());
        for (int i = 0; i < descriptors.size(); ++i) {
            VkDescriptorSet descriptorSet;
            if (vkAllocateDescriptorSets(myVulkanDevice.device(), &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }    
            descriptors.push_back(descriptorSet);
        }
        

        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(storageImages.size());
        for (int i = 0; i < storageImages.size(); ++i) {

            VkDescriptorImageInfo imageToAdd{};
            imageToAdd.imageView = storageImages[i].imageView;
            imageToAdd.sampler = storageImages[i].sampler;
            imageToAdd.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageInfos.push_back(imageToAdd);

            VkWriteDescriptorSet imageWrite = INITIALIZERS::writeDescriptorSet(descriptors[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, i, &imageInfos.back());
            writeDescriptorSets.push_back(imageWrite);

        }

        vkUpdateDescriptorSets(myVulkanDevice.device(), writeDescriptorSets.size(),writeDescriptorSets.data(), 0, nullptr);

    }

    void Bloom_RS::CreatePipelineUpSample() {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (int i = 0; i <storageImages.size() ; ++i) {
            VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
            resultImageLayoutBinding.binding = i;
            resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            resultImageLayoutBinding.descriptorCount = 1;
            resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings.push_back(resultImageLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = INITIALIZERS::descriptorSetLayoutCreateInfo(bindings);

        if (vkCreateDescriptorSetLayout(myVulkanDevice.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayout)!=VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create descriptor set layouts");
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
        if (vkCreatePipelineLayout(myVulkanDevice.device(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create pipeline layout");
        }
        PipelineConfigInfo pipelineConfig{};

        PipelineReader::UIPipelineDefaultConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPassRef;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        upSamplePipelineReader = std::make_unique<PipelineReader>(myVulkanDevice);

//        VkFormat format = myVulkanRenderer.GetSwapchain().getSwapChainImageFormat();
//        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        const VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR
        };

        upSamplePipelineReader->CreateFlexibleGraphicPipeline<UIVertex>(
                vertexPath,
                fragmentPathUpSample,
                pipelineConfig, true, pipelineRenderingCreateInfo);
    }

    void Bloom_RS::CreatePipelineDownSample() {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (int i = 0; i <storageImages.size() ; ++i) {
            VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
            resultImageLayoutBinding.binding = i;
            resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            resultImageLayoutBinding.descriptorCount = 1;
            resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings.push_back(resultImageLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = INITIALIZERS::descriptorSetLayoutCreateInfo(bindings);

        if (vkCreateDescriptorSetLayout(myVulkanDevice.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayout)!=VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create descriptor set layouts");
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
        if (vkCreatePipelineLayout(myVulkanDevice.device(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create pipeline layout");
        }
        PipelineConfigInfo pipelineConfig{};

        PipelineReader::UIPipelineDefaultConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPassRef;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        downSamplePipelineReader = std::make_unique<PipelineReader>(myVulkanDevice);

//        VkFormat format = myVulkanRenderer.GetSwapchain().getSwapChainImageFormat();
//        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        const VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR
        };

        downSamplePipelineReader->CreateFlexibleGraphicPipeline<UIVertex>(
                vertexPath,
                fragmentPathDownSample,
                pipelineConfig, true, pipelineRenderingCreateInfo);

    }

    void Bloom_RS::CreateBuffers() {

        myVulkanDevice.createBuffer(
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &vertexBuffer,
                quadVertices.size() * sizeof(UIVertex),
                quadVertices.data());
        myVulkanDevice.createBuffer(
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &indexBuffer,
                quadIndices.size() * sizeof(uint16_t),
                quadIndices.data());

    }
    void Bloom_RS::Draw(VkCommandBuffer &currentCommandBuffer) {

        assert(ready &&"Postprocessing is not ready to draw");

        VkDeviceSize offsets[1] = { 0 };

        downSamplePipelineReader->bind(currentCommandBuffer);
        for (int i = 0; i < descriptors.size(); ++i) {
            vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptors[i], 0, nullptr);
            vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(currentCommandBuffer, quadIndices.size(), 1, 0, 0, 0);
        }

        upSamplePipelineReader->bind(currentCommandBuffer);
        
        for (int i = 0; i < descriptors.size(); ++i) {
            vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptors[i], 0, nullptr);
            vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(currentCommandBuffer, quadIndices.size(), 1, 0, 0, 0);
        }

    }

    void Bloom_RS::InitRS() {
        assert(renderPassRef != VK_NULL_HANDLE && "Renderpass must not be null");
        assert(!storageImages.empty() && "storageImages must not be null");
        vertexPath=HELPERS::FileHandler::GetInstance()->GetShadersPath() + "\\PostPro\\postpro.vert.spv";
        fragmentPathDownSample=HELPERS::FileHandler::GetInstance()->GetShadersPath() + "\\PostPro\\DownSample.frag.spv";
        fragmentPathUpSample=HELPERS::FileHandler::GetInstance()->GetShadersPath() + "\\PostPro\\UpSample.frag.spv";
                
        CreateBuffers();
        CreatePipelineDownSample();
        CreatePipelineUpSample();
        CreateDescriptorSets();
        ready = true;
    }

    void Bloom_RS::AddTextureImageToShader(VkImageView imageView, VkSampler sampler) {

        SimpleImageData imageData{};
        imageData.imageView = imageView;
        imageData.sampler = sampler;
        storageImages.push_back(imageData);
    }


}