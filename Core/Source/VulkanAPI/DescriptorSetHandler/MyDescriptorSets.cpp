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

    void MyDescriptorSets::CreateLayoutBinding(int binding, int descriptorCount)
    {
        descriptorSetLayout.resize(descriptorCount);
        descriptorData.resize(descriptorCount);
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        layoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = descriptorCount;
        layoutInfo.pBindings = &layoutBinding;

        if (vkCreateDescriptorSetLayout(myDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorCount - 1]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void MyDescriptorSets::CreateDescriptorPool(int descriptorCount, int setSize, int maxFramesInFlight)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(descriptorCount * maxFramesInFlight);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = setSize;
        poolInfo.pPoolSizes = &poolSize;

        poolInfo.maxSets = static_cast<uint32_t>(descriptorCount * maxFramesInFlight);

        if (vkCreateDescriptorPool(myDevice.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }


}

//TODO: Add the following to the MyDescriptorSets class:
// struct DescriptorSetData {
//     std::vector<VkBuffer> uniformBuffers;
//     std::vector<VkDeviceMemory> uniformBuffersMemory;
//     std::vector<void*> uniformBuffersMapped; // Note: Use with caution, proper synchronization needed
//     VkDescriptorPool descriptorPool;
//     std::vector<VkDescriptorSet> descriptorSets;
// };
// Then, you can create an array or a vector of this struct if you need to manage multiple sets of these resources, for example, when rendering objects that have different materials, each requiring its own set of descriptors.
//
// cpp
// Copy code
// std::vector<DescriptorSetData> allDescriptorSets;
// Handling Multiple Uniforms in the Same Set
// To have multiple uniforms within the same descriptor set, you should:
//
// Define Descriptor Set Layouts: For each type of uniform buffer (or other resources like samplers, image views, etc.) you want in the descriptor set, you need a corresponding descriptor set layout binding. Each binding should have a unique binding number within the set.
//
// Create Descriptor Set Layout: Use these layout bindings to create a descriptor set layout that defines the structure of your descriptor set.
//
// Allocate Descriptor Sets: When allocating descriptor sets that use this layout, each set will have slots for all the resources defined by your layout bindings.
//
// Update Descriptor Sets: To associate your actual Vulkan resources (like uniform buffers) with the descriptor set, you need to create VkDescriptorBufferInfo (for buffers) or VkDescriptorImageInfo (for images) for each resource, and then populate a VkWriteDescriptorSet structure for each resource you're updating in the set. These structures are then passed to vkUpdateDescriptorSets() to bind the resources to the descriptor set.
//
// Here's a simplified example of setting up a descriptor set layout for two uniform buffers:
//
// cpp
// Copy code
// VkDescriptorSetLayoutBinding uboLayoutBinding = {};
// uboLayoutBinding.binding = 0; // First uniform buffer
// uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
// uboLayoutBinding.descriptorCount = 1;
// uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Assuming vertex shader
// uboLayoutBinding.pImmutableSamplers = nullptr; // Not used for buffers
//
// VkDescriptorSetLayoutBinding anotherUboLayoutBinding = {};
// anotherUboLayoutBinding.binding = 1; // Second uniform buffer
// anotherUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
// anotherUboLayoutBinding.descriptorCount = 1;
// anotherUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Assuming fragment shader
// anotherUboLayoutBinding.pImmutableSamplers = nullptr;
//
// std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, anotherUboLayoutBinding};
// VkDescriptorSetLayoutCreateInfo layoutInfo = {};
// layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
// layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
// layoutInfo.pBindings = bindings.data();
//
// VkDescriptorSetLayout descriptorSetLayout;
// vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);