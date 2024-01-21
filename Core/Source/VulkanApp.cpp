#include "VulkanApp.h"
#include <stdexcept>
#include <array>
namespace VULKAN{
	
	void VulkanApp::Run()
	{
		while (!initWindow.ShouldClose())
		{
			glfwPollEvents();
			DrawFrame();
		}
		vkDeviceWaitIdle(myDevice.device());
	}

	VulkanApp::VulkanApp()
	{
		LoadModels();
		CreatePipelineLayout();
		CreatePipeline();
		CreateCommandBuffer();
	}

	VulkanApp::~VulkanApp()
	{
		vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);

	}

	void VulkanApp::LoadModels()
	{
		std::vector<MyModel::Vertex> vertices{
			{{1.0f, 1.0f}},
			{{0.0f, -1.0f}},
			{{-1.0f, 1.0f}},
		};

		std::vector<triangle> triangles;

		triangle tri{};
		tri.vertices = vertices;
		triangles.push_back(tri);

		triangles = FindTriangles(triangles,5);

		std::vector<MyModel::Vertex> newVertices = GetAllVertexFlatten(triangles);

		std::cout <<"Size of vertices: "<< newVertices.size() << "\n";

		
		myModel = std::make_unique<MyModel>(myDevice, newVertices);


	}

	void VulkanApp::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(myDevice.device(),&pipelineLayoutInfo, nullptr, &pipelineLayout)!= VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void VulkanApp::CreatePipeline()
	{
		auto pipelineConfig = PipelineReader::DefaultPipelineDefaultConfigInfo(WIDTH, HEIGHT);
		pipelineConfig.renderPass = swapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineReader = std::make_unique<PipelineReader>(
			myDevice,
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.vert.spv",
			"C:/Users/carlo/Documents/GitHub/CodeRT/Core/Source/Shaders/base_shader.frag.spv",
			pipelineConfig

		);
	}

	void VulkanApp::CreateCommandBuffer()
	{
		commandBuffer.resize(swapChain.imageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = myDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());

		if (vkAllocateCommandBuffers(myDevice.device(), &allocInfo, commandBuffer.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
		for (int i = 0; i < commandBuffer.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(commandBuffer[i], &beginInfo)!= VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = swapChain.getRenderPass();
			renderPassBeginInfo.framebuffer = swapChain.getFrameBuffer(i);
			renderPassBeginInfo.renderArea.offset = { 0,0 };
			renderPassBeginInfo.renderArea.extent = swapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(commandBuffer[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


			pipelineReader->bind(commandBuffer[i]);

			//vkCmdDraw(commandBuffer[i], 3, 1, 0, 0);
			myModel->Bind(commandBuffer[i]);
			myModel->Draw(commandBuffer[i]);
			


			vkCmdEndRenderPass(commandBuffer[i]);
			if (vkEndCommandBuffer(commandBuffer[i])!= VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer!");
			}
		}
		 
	}

	void VulkanApp::DrawFrame()
	{
		uint32_t imageIndex;
		auto result = swapChain.acquireNextImage(&imageIndex);

		if (result!= VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		result = swapChain.submitCommandBuffers(&commandBuffer[imageIndex], &imageIndex);
		if (result!= VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}


	}

	std::vector<triangle> VulkanApp::FindTriangles(std::vector<triangle>  myTriangle,int deep)
	{
		std::vector<triangle> triangles;
		if (deepness>=deep)
		{
			std::cout << "Deepnesss: " <<deepness << "\n";

			return myTriangle;
		}
		else
		{
			int tris = 0;
			for (size_t i = 0; i < myTriangle.size(); i++)
			{
				std::vector<triangle> newTris = FindTriMidPoint(myTriangle[i]);

				for (size_t j = 0; j < newTris.size(); j++)
				{
					triangles.push_back(newTris[j]);

					tris++;
				}
			}
			deepness++;
			return FindTriangles(triangles, deep);
		}

	}

	std::vector<triangle> VulkanApp::FindTriMidPoint(triangle myTriangle)
	{
		std::vector<triangle> triangles;

		MyModel::Vertex vertex3{};
		MyModel::Vertex vertex4{};
		MyModel::Vertex vertex5{};

		vertex3.position =((myTriangle.vertices[0].position + myTriangle.vertices[1].position) / 2.0f);
		vertex4.position = ((myTriangle.vertices[1].position + myTriangle.vertices[2].position) / 2.0f);
		vertex5.position = ((myTriangle.vertices[2].position + myTriangle.vertices[0].position) / 2.0f);

		// Triangle at the corner 0
		triangle tri0{};
		tri0.vertices.push_back(myTriangle.vertices[0]);
		tri0.vertices.push_back(vertex3);
		tri0.vertices.push_back(vertex5);
		triangles.push_back(tri0);

		// Triangle at the corner 1
		triangle tri1{};
		tri1.vertices.push_back(vertex3);
		tri1.vertices.push_back(myTriangle.vertices[1]);
		tri1.vertices.push_back(vertex4);
		triangles.push_back(tri1);

		// Triangle at the corner 2
		triangle tri2{};
		tri2.vertices.push_back(vertex5);
		tri2.vertices.push_back(vertex4);
		tri2.vertices.push_back(myTriangle.vertices[2]);
		triangles.push_back(tri2);



		return triangles;
	}

	std::vector<MyModel::Vertex> VulkanApp::GetAllVertexFlatten(std::vector<triangle> triangles)
	{
		std::vector<MyModel::Vertex> vertices;
		for (size_t i = 0; i < triangles.size(); i++)
		{
			std::cout << "TRIANGLE= " << i << "\n";
			for (size_t j = 0; j < 3; j++)
			{
				vertices.push_back(triangles[i].vertices[j]);
				

			}
			std::cout << "\n";

		}

		return vertices;
	}

}

