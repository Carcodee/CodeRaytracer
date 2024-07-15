#pragma once
#include <filesystem>
#include "VulkanAPI/Utility/Utility.h"


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
        void DisplayMeshInfo();
        int modelIDInspected=-1;
        int materialIDInspected=-1;

		float iconSize = 50;
		float thumbnailSpace = 20;
		int rows = 10;
		int cols = 10;
		int columns;

	};

}
