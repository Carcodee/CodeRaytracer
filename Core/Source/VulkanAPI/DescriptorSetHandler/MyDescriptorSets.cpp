#include "MyDescriptorSets.h"

namespace VULKAN
{

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
