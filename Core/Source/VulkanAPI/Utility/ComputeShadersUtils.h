#pragma once

#include "VulkanAPI/VulkanInit/VulkanInit.h"
#include <array>

namespace VULKAN {

	struct Particle
	{
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec4 color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Particle);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Particle, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Particle, color);
            return attributeDescriptions;
        }
	};

    void InitParticles(std::vector<Particle>& particles)
    {
		for (auto& particle : particles)
		{
			particle.position = glm::vec2(0.0f, 0.0f);

            float xV = static_cast<float>(rand()) / RAND_MAX;
            float yV = static_cast<float>(rand()) / RAND_MAX;

            particle.velocity = glm::vec2(xV*1000, yV*1000);
            particle.color = glm::vec4(
                static_cast<float>(rand()) / RAND_MAX, // Red
                static_cast<float>(rand()) / RAND_MAX, // Green
                static_cast<float>(rand()) / RAND_MAX, // Blue
                1.0f); // Alpha
		}
    }



}
