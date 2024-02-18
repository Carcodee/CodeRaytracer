
#include<iostream>
#include <imgui.h>
#include <functional>
#include<imgui_impl_vulkan.h>
#include<imgui_impl_glfw.h>
#include "../../Core/Source/VulkanApp.h"


void ImguiContext(bool& show_demo_window, bool& show_another_window, const ImGuiIO& io) {

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImDrawData* main_draw_data = ImGui::GetDrawData();
    const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }


    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

}

int main() {
    VULKAN::VulkanApp app;




    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
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
     auto ImguiRender = [&show_demo_window ,&show_another_window, &io]() -> void {
        ImguiContext(show_demo_window, show_another_window, io);

     };

     // Setup Platform/Renderer backends
     ImGui_ImplGlfw_InitForVulkan(app.initWindow.GetWindow(), true);
     ImGui_ImplVulkan_InitInfo init_info = {};
     init_info.Instance = app.myDevice.instance;
     init_info.PhysicalDevice = app.myDevice.physicalDevice;
     init_info.Device = app.myDevice.device();
     init_info.QueueFamily = app.myDevice.findPhysicalQueueFamilies().graphicsFamily;
     init_info.Queue = app.myDevice.graphicsQueue();
     init_info.PipelineCache = VK_NULL_HANDLE;
     init_info.DescriptorPool = app.descriptorSetsHandler->descriptorPool;
     init_info.RenderPass = app.renderer.GetSwapchainRenderPass();
     init_info.Subpass = 0;
     init_info.MinImageCount =2;
     init_info.ImageCount = app.renderer.GetImageCount();
     init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
     init_info.Allocator = VK_NULL_HANDLE;
     init_info.CheckVkResultFn = VK_NULL_HANDLE;
     ImGui_ImplVulkan_Init(&init_info);
	std::cout << "Hello Editor" << std::endl;

    VkCommandBuffer commandBuffer = app.myDevice.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture();
    app.myDevice.endSingleTimeCommands(commandBuffer);



    app.RunEngine_EDITOR(ImguiRender);
    
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	return 0;
} 