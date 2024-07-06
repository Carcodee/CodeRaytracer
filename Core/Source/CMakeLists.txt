cmake_minimum_required(VERSION 3.26)
project(Core)


set(CORE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CORE_FILES "${CORE_BASE_DIR}/Core/*.h" "${CORE_BASE_DIR}/Core/*.cpp")
file(GLOB_RECURSE FILESYSTEM "${CORE_BASE_DIR}/FileSystem/*.h" "${CORE_BASE_DIR}/FileSystem/*.cpp")
file(GLOB_RECURSE CAMERA "${CORE_BASE_DIR}/VulkanAPI/Camera/*.h" "${CORE_BASE_DIR}/VulkanAPI/Camera/*.cpp")
file(GLOB_RECURSE DESCRIPTOR_SET_HANDLER "${CORE_BASE_DIR}/VulkanAPI/DescriptorSetHandler/*.h" "${CORE_BASE_DIR}/VulkanAPI/DescriptorSetHandler/*.cpp")
file(GLOB_RECURSE VULKAN_DEVICE "${CORE_BASE_DIR}/VulkanAPI/DevicePipeline/*.h" "${CORE_BASE_DIR}/VulkanAPI/DevicePipeline/*.cpp")
file(GLOB_RECURSE HEADERS "${CORE_BASE_DIR}/VulkanAPI/Headers/*.h")
file(GLOB_RECURSE MODEL "${CORE_BASE_DIR}/VulkanAPI/Model/*.h" "${CORE_BASE_DIR}/VulkanAPI/Model/*.cpp")
file(GLOB_RECURSE MODEL_LOADER "${CORE_BASE_DIR}/VulkanAPI/ModelLoader/*.h" "${CORE_BASE_DIR}/VulkanAPI/ModelLoader/*.cpp")
file(GLOB_RECURSE OBJ_LOADER "${CORE_BASE_DIR}/VulkanAPI/ObjectLoader/*.h" "${CORE_BASE_DIR}/VulkanAPI/ObjectLoader/*.cpp")
file(GLOB_RECURSE RENDERER "${CORE_BASE_DIR}/VulkanAPI/Renderer/*.h" "${CORE_BASE_DIR}/VulkanAPI/Renderer/*.cpp")
file(GLOB_RECURSE RENDER_SYSTEMS "${CORE_BASE_DIR}/VulkanAPI/RenderSystems/*.h" "${CORE_BASE_DIR}/VulkanAPI/RenderSystems/*.cpp")
file(GLOB_RECURSE RESOURCES_MANAGERS_ASSETS "${CORE_BASE_DIR}/VulkanAPI/ResourcesManagers/Assets/*.h" "${CORE_BASE_DIR}/VulkanAPI/ResourcesManagers/Assets/*.cpp")
file(GLOB_RECURSE RESOURCES_MANAGERS_UI "${CORE_BASE_DIR}/VulkanAPI/ResourcesManagers/UI/*.h" "${CORE_BASE_DIR}/VulkanAPI/ResourcesManagers/UI/*.cpp")
file(GLOB_RECURSE SWAP_CHAIN "${CORE_BASE_DIR}/VulkanAPI/SwapChain/*.h" "${CORE_BASE_DIR}/VulkanAPI/SwapChain/*.cpp")
file(GLOB_RECURSE UTILITY "${CORE_BASE_DIR}/VulkanAPI/Utility/*.h" "${CORE_BASE_DIR}/VulkanAPI/Utility/*.cpp")
file(GLOB_RECURSE UTILITY_INPUT_SYSTEM "${CORE_BASE_DIR}/VulkanAPI/Utility/InputSystem/*.h" "${CORE_BASE_DIR}/VulkanAPI/Utility/InputSystem/*.cpp")
file(GLOB_RECURSE VULKAN_INIT "${CORE_BASE_DIR}/VulkanAPI/VulkanInit/*.h" "${CORE_BASE_DIR}/VulkanAPI/VulkanInit/*.cpp")
file(GLOB_RECURSE VULKAN_OBJ_BUFFERS "${CORE_BASE_DIR}/VulkanAPI/VulkanObjects/Buffers/*.h" "${CORE_BASE_DIR}/VulkanAPI/VulkanObjects/Buffers/*.cpp")
file(GLOB_RECURSE VULKAN_OBJ_RESOURCE_INTERFACE "${CORE_BASE_DIR}/VulkanAPI/VulkanObjects/ResourceInterface/*.h" "${CORE_BASE_DIR}/VulkanAPI/VulkanObjects/ResourceInterface/*.cpp")
file(GLOB_RECURSE VULKAN_OBJ_TEXTURES "${CORE_BASE_DIR}/VulkanAPI/VulkanObjects/Textures/*.h" "${CORE_BASE_DIR}/VulkanAPI/VulkanObjects/Textures/*.cpp")
file(GLOB_RECURSE VULKAN_VULKAN_PIPELINE "${CORE_BASE_DIR}/VulkanAPI/VulkanPipeline/*.h" "${CORE_BASE_DIR}/VulkanAPI/VulkanPipeline/*.cpp")

set(CORE_SOURCES
        VulkanApp.h
        VulkanApp.cpp
        ${CORE_FILES}
        ${FILESYSTEM}
        ${CAMERA}
        ${DESCRIPTOR_SET_HANDLER}
        ${VULKAN_DEVICE}
        ${HEADERS}
        ${MODEL}
        ${MODEL_LOADER}
        ${OBJ_LOADER}
        ${RENDERER}
        ${RENDER_SYSTEMS}
        ${RESOURCES_MANAGERS_ASSETS}
        ${RESOURCES_MANAGERS_UI}
        ${SWAP_CHAIN}
        ${UTILITY}
        ${UTILITY_INPUT_SYSTEM}
        ${VULKAN_INIT}
        ${VULKAN_OBJ_BUFFERS}
        ${VULKAN_OBJ_RESOURCE_INTERFACE}
        ${VULKAN_OBJ_TEXTURES}
        ${VULKAN_VULKAN_PIPELINE}
)
set(IMGUI_SOURCES
        "${CORE_BASE_DIR}/Libraries/imgui-docking/imgui.cpp"
        "${CORE_BASE_DIR}/Libraries/imgui-docking/imgui_demo.cpp"
        "${CORE_BASE_DIR}/Libraries/imgui-docking/imgui_draw.cpp"
        "${CORE_BASE_DIR}/Libraries/imgui-docking/imgui_tables.cpp"
        "${CORE_BASE_DIR}/Libraries/imgui-docking/imgui_widgets.cpp"
        "${CORE_BASE_DIR}/Libraries/imgui-docking/imgui_impl_glfw.cpp"
)

add_library(Core STATIC
        ${CORE_SOURCES}
        ${IMGUI_SOURCES}
)

# GLFW
set(GLFW_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/glfw-3.3.9.bin.WIN64/include")
set(GLFW_LIB_DIR "${CORE_BASE_DIR}/Libraries/glfw-3.3.9.bin.WIN64/lib-mingw-w64")

# Vulkan
set(VULKAN_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/VulkanInclude")
set(VULKAN_LIB_DIR "${CORE_BASE_DIR}/Libraries/VulkanInclude/Lib")

# stb
set(STB_MASTER_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/stb-master")

# nlohmann json lib
set(NLOHMANN_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/nlohmann")

# imgui
set(IMGUI_DOCKING_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/imgui-docking")

set(GLM_BUILD_LIBRARY OFF)
add_subdirectory("${CORE_BASE_DIR}/Libraries/glm")
add_subdirectory("${CORE_BASE_DIR}/Libraries/tinyobjloader-release")

#set(TINYGLTF_HEADER_ONLY ON)
add_subdirectory("${CORE_BASE_DIR}/Libraries/tinygltf-release")

set (GLM_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/glm")
set (TINY_OBJ_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/tinyobjloader-release")
set (TINY_GLTF_INCLUDE_DIR "${CORE_BASE_DIR}/Libraries/tinygltf-release")


target_include_directories(Core PUBLIC
        ${GLFW_INCLUDE_DIR}
        ${VULKAN_INCLUDE_DIR}
        ${STB_MASTER_INCLUDE_DIR}
        ${NLOHMANN_INCLUDE_DIR}
        ${IMGUI_DOCKING_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${TINY_OBJ_INCLUDE_DIR}
        ${TINY_GLTF_INCLUDE_DIR}
        ${CORE_BASE_DIR}
)

target_link_directories(Core PUBLIC
        ${GLFW_LIB_DIR}
        ${VULKAN_LIB_DIR}
)
target_link_libraries(Core PUBLIC
        glfw3
        vulkan-1
        tinyobjloader
)

