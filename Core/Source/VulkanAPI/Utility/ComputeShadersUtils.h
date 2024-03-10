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
            int xV = rand() / RAND_MAX;
            int yV = rand() / RAND_MAX;

			particle.velocity = glm::vec2(xV, yV);
			particle.color = glm::vec4(rand() / RAND_MAX, rand() / RAND_MAX, rand() / RAND_MAX, 1.0f);
		}
    }



}
