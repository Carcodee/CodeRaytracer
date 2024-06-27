#include "FileHandler.h"

#include <iostream>

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

	bool FileHandler::IsPathInAssets(std::string path)
	{
		std::filesystem::path relativeToAssetsPath(path);
		relativeToAssetsPath = assetPath / relativeToAssetsPath;

		if (!std::filesystem::exists(relativeToAssetsPath))
		{
			return false;
		}

	}

	bool FileHandler::IsPathAbsolute(std::string path)
	{
		if (!std::filesystem::exists(path))
		{
			return false;
		}
	}

	std::string FileHandler::HandleModelFilePath(std::string path)
	{
		std::filesystem::path newPath = "";

		std::string extension = GetPathExtension(path);
		if (extension!= "obj")
		{
			std::cout << "Path is not the expected extension: " + extension << "\n";
			return "";
		}
		if (IsPathInAssets(path))
		{
			newPath = assetPath / std::filesystem::path(path);
			return newPath.string();
		}
		if (IsPathAbsolute(path))
		{
			return path;
		}
	}

	std::string FileHandler::GetPathExtension(std::string path)
	{
		size_t extensionPos = path.find_last_of(".");
		if (extensionPos > path.size())
		{
			return "";
		}
		std::string extension = path.erase(0, extensionPos + 1);
		return extension;



	}
}
