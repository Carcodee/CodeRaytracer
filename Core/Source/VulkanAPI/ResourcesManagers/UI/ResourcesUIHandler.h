#pragma once
#include <filesystem>
#include "VulkanAPI/Utility/Utility.h"
#include "VulkanAPI/Utility/ISerializable.h"


namespace VULKAN
{
	
	class ResourcesUIHandler
	{
	protected:
		static ResourcesUIHandler* instance;
		ResourcesUIHandler();

	public:

		static ResourcesUIHandler* GetInstance();
		void DisplayDirInfo();
        void DisplayInspectorInfo();
        void DisplayTexturesTab();
        void DisplayBLASesInfo();
        void DisplayMatInfo(Material& mat, ImVec2 iconSize);
        void DisplayMatTexture(Material& mat, TEXTURE_TYPE textureType, ImVec2& iconSize, std::string texName);
        void DisplayMeshInfo(ModelData& modelData);
        void DisplayMeshInfo(Sphere& sphereData);
        void DisplayViewportFrameBuffers(std::vector<VKTexture*> framebuffers);
        void HandleDrag(TEXTURE_TYPE textureType, Material& mat);
        void HandleDrag(VKTexture* texture);
        void HandleDrop(TEXTURE_TYPE textureType, Material& mat);
        
        float positionInspected[3] = {0.0f,0.0f,0.0f};
        float scaleInspected[3] = {1.0f,1.0f,1.0f};
        float rotationInspected[3] = {0.0f,0.0f,0.0f};
        
        
        std::string pathInspected ="";
		float iconSize = 50;
		float thumbnailSpace = 20;
		int rows = 10;
		int cols = 10;
		int columns;

	};

}
