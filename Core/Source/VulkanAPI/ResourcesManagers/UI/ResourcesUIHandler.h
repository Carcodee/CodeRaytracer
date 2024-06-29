#pragma once
#include <filesystem>


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

		float iconSize = 50;
		float thumbnailSpace = 20;
		int rows = 10;
		int cols = 10;
		int columns;

	};

}
