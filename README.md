# CodeRT Engine

## Overview

This is a path tracer for self-education with the Vulkan raytracing pipeline.

It haves the capability of adding postprocessing renderpasses easily with a custom postprocessing pipeline.

## Features

- Global illumination through frame accumulation.
- Disney BSDF rendering model for realistic material representation.
- Bindless ray tracing for efficient resource management.
- Physically-based bloom for enhanced lighting effects.
- Asynchronous loading support for GLTF and OBJ scenes/models.
- Flexible and extensible material system.
- Multithreaded asset handling to optimize I/O operations.
- Custom implementation of ImGui.
- Asset handler and free camera movement.
- Multiple postprocessing renderpasses visualization.
- Input handler abstraction on top of glfw.

## Path tracing

Path tracing is based on the Vulkan ray tracing pipeline, which is provided by VK_KHR_ray_tracing_pipeline extension.
Currently, path tracing doesn't have any runtime denosing, there is just a simple result accumulation when camera the doesn't move.


##Running The project for visual studio users

Build the solution inside "VStudio_Build" and after the build is done set editor as stardup project

## Examples

there are examples Inside "Core/Source/Resources/Assets/models", just drag and drop the model file inside the app and the model will be loaded

## Renders


![Screenshot 2024-09-19 171644](https://github.com/user-attachments/assets/002b5e95-c494-49a8-ae59-0e47a90c946e)
![Screenshot 2024-09-19 171414](https://github.com/user-attachments/assets/9e191277-c535-4c51-9b5a-67e6be7b4904)
![Screenshot 2024-09-19 173226](https://github.com/user-attachments/assets/08812224-47f4-4284-b920-1beae3528f82)
![Screenshot 2024-09-19 174137](https://github.com/user-attachments/assets/c8748c4e-12b1-4240-8e75-aedec36f6df2)
![Screenshot 2024-09-19 174438](https://github.com/user-attachments/assets/2b9ca34e-fe5e-460a-b65f-28764ed94a12)
![Screenshot 2024-09-19 000507](https://github.com/user-attachments/assets/f899e887-6ec7-4d18-844c-c32291d9c622)
![Screenshot 2024-09-19 170855](https://github.com/user-attachments/assets/90c946cb-af1a-47fa-8b58-ad12544cce94)
