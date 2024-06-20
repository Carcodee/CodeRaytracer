#include "FileHandler.h"

namespace HELPERS
{

	FileHandler* FileHandler::instance = nullptr;

	FileHandler::FileHandler()
	{

		workingDir = std::filesystem::current_path();
		projectPath = workingDir.parent_path();
		assetPath = projectPath / "Core"/"Source" / "Resources" / "Assets";
		shadersPath = projectPath / "Core"/ "Source" / "Shaders";

	}

	std::string FileHandler::GetShadersPath()
	{
		return shadersPath.string();
	}

	std::string FileHandler::GetAssetsPath()
	{
		return assetPath.string();
	}

	FileHandler* FileHandler::GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new FileHandler;
		}
		return instance;
	}
}
