#include "MyDescriptorSets.h"

namespace VULKAN
{

    MyDescriptorSets::~MyDescriptorSets()
    {
        for (size_t i = 0; i < descriptorData.size(); i++) {
            for (size_t j = 0; j < 2; j++) {
                vkDestroyBuffer(myDevice.device(), descriptorData[i].uniformBuffers[j], nullptr);
                vkFreeMemory(myDevice.device(), descriptorData[i].uniformBuffersMemory[j], nullptr);
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


}