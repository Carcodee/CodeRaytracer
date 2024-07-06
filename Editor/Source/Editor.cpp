
#include<iostream>
#include <functional>

#include <VulkanApp.h>



void ImguiContext() {



}
int main() {
    VULKAN::VulkanApp app(true, true);

    app.RunDynamicRendering(ImguiContext);

	return 0;
} 