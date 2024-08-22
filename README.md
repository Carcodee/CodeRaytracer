# CodeRT

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

## Examples

there are examples Inside "Core/Source/Resources/Assets/models", just drag and drop the model file inside the app and the model will be loaded

##Running The project

Right there is only support for cmake projects, soon I will load a easy to build visual studio solution

## Renders
![Screenshot 2024-08-19 233937](https://github.com/user-attachments/assets/2118514d-1cb8-44c1-91b4-dba49761d08a)
![Screenshot 2024-08-19 212059](https://github.com/user-attachments/assets/3612c2e4-6018-4185-a6c0-6afbb6ba0c90)
![Screenshot 2024-08-19 154754](https://github.com/user-attachments/assets/51e65995-0d81-4e79-8997-95b53958fa4b)
![Screenshot 2024-08-19 153044](https://github.com/user-attachments/assets/2a1f1125-61f8-427d-9a70-db24217cb924)
![Screenshot 2024-08-19 152347](https://github.com/user-attachments/assets/fabbe3c3-5c5d-4c9e-881c-cdb76ae79a6d)
![Screenshot 2024-08-12 000620](https://github.com/user-attachments/assets/01943b5c-ad47-4f3a-b2e9-fb375f5f8098)
![Screenshot 2024-08-10 225416](https://github.com/user-attachments/assets/c8b5ca02-7b73-45a2-ad3b-f2a09f20a798)
![Screenshot 2024-08-10 143658](https://github.com/user-attachments/assets/c3537c51-696d-4946-bc2e-d41d26c6ea73)


