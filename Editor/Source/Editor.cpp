
#include<iostream>
#include <imgui.h>
#include <functional>
#include<imgui_impl_vulkan.h>
#include<imgui_impl_glfw.h>
#include <imgui_internal.h>

#include "../../Core/Source/VulkanApp.h"



void ImguiContext() {


     bool show_demo_window = true;
    ImGui::ShowDemoWindow(&show_demo_window);

}
int main() {
    VULKAN::VulkanApp app(true);
    app.RunDynamicRendering(ImguiContext);

	return 0;
} 