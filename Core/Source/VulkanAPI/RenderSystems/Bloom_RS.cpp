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
        descriptors.reserve(storageImages.size()-1);
        for (int i = 0; i < storageImages.size()-1; ++i) {
            VkDescriptorSet descriptorSet;
            if (vkAllocateDescriptorSets(myVulkanDevice.device(), &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            descriptors.push_back(descriptorSet);
        }


        for (int i = 0; i < descriptors.size(); ++i) {

            writeDescriptorSets.clear();
            VkDescriptorImageInfo srcImage{};
            srcImage.imageView = storageImages[i].imageView;
            srcImage.sampler = storageImages[i].sampler;
            srcImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkDescriptorImageInfo modifiedImage{};
            modifiedImage.imageView = storageImages[i + 1].imageView;
            modifiedImage.sampler = storageImages[i + 1].sampler;
            modifiedImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            
            VkWriteDescriptorSet srcImageWrite = INITIALIZERS::writeDescriptorSet(descriptors[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &srcImage);
            VkWriteDescriptorSet modifiedImageWrite = INITIALIZERS::writeDescriptorSet(descriptors[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &modifiedImage);
            
            writeDescriptorSets.push_back(srcImageWrite);
            writeDescriptorSets.push_back(modifiedImageWrite);

            vkUpdateDescriptorSets(myVulkanDevice.device(), writeDescriptorSets.size(),writeDescriptorSets.data(), 0, nullptr);
        }


    }

    void Bloom_RS::CreatePipelineUpSample() {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        
        VkDescriptorSetLayoutBinding srcTexBinding{};
        
        srcTexBinding.binding = 0;
        srcTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        srcTexBinding.descriptorCount = 1;
        srcTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding upSampleTexBinding{};
        upSampleTexBinding.binding = 1;
        upSampleTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        upSampleTexBinding.descriptorCount = 1;
        upSampleTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings.push_back(srcTexBinding);
        bindings.push_back(upSampleTexBinding);

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = INITIALIZERS::descriptorSetLayoutCreateInfo(bindings);

        if (vkCreateDescriptorSetLayout(myVulkanDevice.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayout)!=VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create descriptor set layouts");
        }

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantBlock_Bloom);

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutCI.pushConstantRangeCount = 1;
        if (vkCreatePipelineLayout(myVulkanDevice.device(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create pipeline layout");
        }
        PipelineConfigInfo pipelineConfig{};

        PipelineReader::UIPipelineDefaultConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = upSampleRenderPassRef;
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
        VkDescriptorSetLayoutBinding srcTexBinding{};
        srcTexBinding.binding = 0;
        srcTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        srcTexBinding.descriptorCount = 1;
        srcTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding upSampleTexBinding{};
        upSampleTexBinding.binding = 1;
        upSampleTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        upSampleTexBinding.descriptorCount = 1;
        upSampleTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings.push_back(srcTexBinding);
        bindings.push_back(upSampleTexBinding);
        

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = INITIALIZERS::descriptorSetLayoutCreateInfo(bindings);

        if (vkCreateDescriptorSetLayout(myVulkanDevice.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayout)!=VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create descriptor set layouts");
        }
        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantBlock_Bloom);

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutCI.pushConstantRangeCount = 1;
        pipelineLayoutCI.pPushConstantRanges = &pushConstantRange; 
        if (vkCreatePipelineLayout(myVulkanDevice.device(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create pipeline layout");
        }
        PipelineConfigInfo pipelineConfig{};

        PipelineReader::UIPipelineDefaultConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = downSampleRenderPassRef;
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
        VkRenderingInfo renderingInfoPostProc{};
        renderingInfoPostProc.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfoPostProc.layerCount = 1;
        renderingInfoPostProc.colorAttachmentCount = 0;
        renderingInfoPostProc.renderArea.offset = { 0, 0};
        renderingInfoPostProc.renderArea.extent = myVulkanRenderer.GetSwapchain().getSwapChainExtent();

        VkMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        VkDeviceSize offsets[1] = { 0 };

        uint32_t currentWidth =myVulkanRenderer.GetSwapchain().getSwapChainExtent().width;
        uint32_t currentHeight =myVulkanRenderer.GetSwapchain().getSwapChainExtent().height;
        VkExtent2D currentExtend {currentWidth, currentHeight};

        downSamplePipelineReader->bind(currentCommandBuffer);
        myVulkanRenderer.BeginDynamicRenderPass(currentCommandBuffer,renderingInfoPostProc);
        for (int i = 0; i < descriptors.size(); ++i) {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(currentExtend.width);
            viewport.height = static_cast<float>(currentExtend.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{ {0, 0 }, currentExtend};
            vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);

            vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptors[i], 0, nullptr);
            vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(currentCommandBuffer, quadIndices.size(), 1, 0, 0, 0);
            vkCmdPushConstants(
                    currentCommandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(PushConstantBlock_Bloom),
                    &pc
            );


            currentExtend.width /=2;
            currentExtend.height /=2;
            
        }

        myVulkanRenderer.EndDynamicRenderPass(currentCommandBuffer);
//        
//        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
//        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
//        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

        vkCmdPipelineBarrier(
                currentCommandBuffer,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                1, &barrier,
                0, nullptr,
                0, nullptr);

        upSamplePipelineReader->bind(currentCommandBuffer);
        myVulkanRenderer.BeginDynamicRenderPass(currentCommandBuffer,renderingInfoPostProc);
        for (int i =descriptors.size()-1; i >= 0 ; --i) {

            currentExtend.width *=2;
            currentExtend.height *=2;
            
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(currentExtend.width);
            viewport.height = static_cast<float>(currentExtend.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{ {0, 0 }, currentExtend};
            vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);

            vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptors[i], 0, nullptr);
            vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(currentCommandBuffer, quadIndices.size(), 1, 0, 0, 0);
            vkCmdPushConstants(
                    currentCommandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(PushConstantBlock_Bloom),
                    &pc
            );

        }
        myVulkanRenderer.EndDynamicRenderPass(currentCommandBuffer);

        vkCmdPipelineBarrier(
                currentCommandBuffer,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                1, &barrier,
                0, nullptr,
                0, nullptr);
        
        
    }

    void Bloom_RS::InitRS() {
        assert(upSampleRenderPassRef != VK_NULL_HANDLE && "Renderpass must not be null");
        assert(downSampleRenderPassRef != VK_NULL_HANDLE && "Renderpass must not be null");
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