C:\VulkanSDK\1.3.268.0\Bin\glslc.exe C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\base_shader.vert -o C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\base_shader.vert.spv
C:\VulkanSDK\1.3.268.0\Bin\glslc.exe C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\base_shader.frag -o C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\base_shader.frag.spv

C:\VulkanSDK\1.3.268.0\Bin\glslc.exe C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\ComputeShaders\compute.comp -o C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\ComputeShaders\compute.comp.spv


C:\VulkanSDK\1.3.268.0\Bin\glslc.exe  --target-env=vulkan1.3 C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\RayTracingShaders\closesthit.rchit -o  C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\RayTracingShaders\closesthit.rchit.spv
C:\VulkanSDK\1.3.268.0\Bin\glslc.exe  --target-env=vulkan1.3 C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\RayTracingShaders\miss.rmiss -o  C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\RayTracingShaders\miss.rmiss.spv
C:\VulkanSDK\1.3.268.0\Bin\glslc.exe  --target-env=vulkan1.3 C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\RayTracingShaders\raygen.rgen -o  C:\Users\carlo\Documents\GitHub\CodeRT\Core\Source\Shaders\RayTracingShaders\raygen.rgen.spv

pause
