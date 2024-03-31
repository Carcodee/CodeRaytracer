
#include<iostream>
#include <imgui.h>
#include <functional>
#include<imgui_impl_vulkan.h>
#include<imgui_impl_glfw.h>
#include <imgui_internal.h>

#include "../../Core/Source/VulkanApp.h"



void ImguiContext(bool& show_demo_window, VkCommandBuffer commandBuffer) {

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if (true)
        ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, 0);
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).


}

void SetUpImgui(VULKAN::VulkanApp& app)
{

    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    (vkCreateDescriptorPool(app.myDevice.device(), &pool_info, nullptr, &imguiPool));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    io.Fonts->AddFontDefault();
    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Setup Dear ImGui style

    io.DisplaySize = ImVec2(800 , 600); // Set to actual window size
    ImGui::StyleColorsDark();

     // Setup Platform/Renderer backends
     bool result=ImGui_ImplGlfw_InitForVulkan(app.initWindow.window, true);
	 if (result)
	 {
         std::cout << "Imgui window init success" << "\n";
	 }
     ImGui_ImplVulkan_InitInfo init_info = {};
     init_info.Instance = VULKAN::MyVulkanDevice::g_Instance;
     init_info.PhysicalDevice = VULKAN::MyVulkanDevice::g_PhysicalDevice;
     init_info.Device = VULKAN::MyVulkanDevice::g_Device;
     init_info.QueueFamily = VULKAN::MyVulkanDevice::g_QueueFamily;
     init_info.Queue = VULKAN::MyVulkanDevice::g_Queue;
     init_info.PipelineCache = VULKAN::MyVulkanDevice::g_PipelineCache;
     init_info.DescriptorPool =imguiPool;
     init_info.RenderPass = app.renderer.GetSwapchainRenderPass();
     init_info.Subpass = 0;
     init_info.MinImageCount = VULKAN::MyVulkanDevice::g_MinImageCount;
     init_info.ImageCount = app.renderer.GetImageCount();
     init_info.MSAASamples = app.myDevice.msaaSamples;
     init_info.Allocator = VULKAN::MyVulkanDevice::g_Allocator;
     init_info.CheckVkResultFn = check_vk_result;
     ImGui_ImplVulkan_Init(&init_info);
	std::cout << "Hello Editor" << std::endl;
    ImGui_ImplVulkan_CreateFontsTexture();
  
}

int main() {
    VULKAN::VulkanApp app;
    app.InitConfigsCache();

	SetUpImgui(app);
   

    app.RunEngine_EDITOR(ImguiContext);
    
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	return 0;
} 