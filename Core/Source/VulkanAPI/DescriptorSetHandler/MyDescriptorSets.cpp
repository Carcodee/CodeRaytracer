#include "MyDescriptorSets.h"

namespace VULKAN
{

    MyDescriptorSets::~MyDescriptorSets()
    {
        for (size_t i = 0; i < descriptorData.size(); i++) {
            for (size_t j = 0; j < 2; j++) {
	            if (descriptorData[i].uniformBuffers.size()!=0)
	            {
	                vkDestroyBuffer(myDevice.device(), descriptorData[i].uniformBuffers[j], nullptr);
		            vkFreeMemory(myDevice.device(), descriptorData[i].uniformBuffersMemory[j], nullptr);
		            
	            }
            }
            vkDestroyDescriptorPool(myDevice.device(), descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(myDevice.device(), descriptorSetLayout[i], nullptr);
        }

    }

    VkDescriptorSetLayoutBinding MyDescriptorSets::CreateDescriptorBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, int bindingCount, int descriptorCount)
    {
        if (descriptorCount != descriptorSetLayout.size())
        {
            descriptorSetLayout.resize(descriptorCount);
            descriptorData.resize(descriptorCount);
        }
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = bindingCount;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.stageFlags = stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;

        return layoutBinding;
    }

    //VkDescriptorSetLayout MyDescriptorSets::CreateDescriptorSetLayout(int descriptorCount)
    //{
    //}

    //VkDescriptorPool MyDescriptorSets::CreateDescriptorPool(int descriptorCount, int maxFramesInFlight)
    //{
    //}

    //VkDescriptorSet MyDescriptorSets::CreateDescriptorSetsTrue(int descriptorCount, int maxFramesInFlight,
	   // VkDescriptorPool pool)
    //{
    //}


    //VkDescriptorBufferInfo MyDescriptorSets::CreateBufferInfo(int descriptorCount, int maxFramesInFlight, VkDescriptorType descriptorType)
    //{
    //}

    //VkDescriptorImageInfo MyDescriptorSets::CreateImageInfo(int descriptorCount, int maxFramesInFlight,
	   // VKTexture& texture)
    //{

    //}

    void MyDescriptorSets::CreateDescriptorSets(int descriptorCount, int maxFramesInFlight)
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
    }
}
