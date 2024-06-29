#include "FileHandler.h"

#include <iostream>

namespace HELPERS
{

	FileHandler* FileHandler::instance = nullptr;
	std::filesystem::path FileHandler::currentPathRelativeToAssets = std::filesystem::path();

	FileHandler::FileHandler()
	{

		workingDir = std::filesystem::current_path();
		projectPath = workingDir.parent_path();
		assetPath = projectPath / "Core"/"Source" / "Resources" / "Assets";
		shadersPath = projectPath / "Core"/ "Source" / "Shaders";
		currentPathRelativeToAssets = assetPath;

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

	bool FileHandler::IsValidPath(std::string path)
	{
		if (IsPathAbsolute(path) || IsPathInAssets(path))
		{
			return true;
		}
		std::cout << path << "Not exist \n";
		return false;
	}

	std::string FileHandler::HandleModelFilePath(std::string path)
	{
		std::filesystem::path newPath = "";

		std::string extension = GetPathExtension(path);

		if (!IsValidPath(path))
		{
			return "";
		}
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

		return path;
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

	uintmax_t FileHandler::GetFileSize(std::string path)
	{
		if (!std::filesystem::exists(path) && IsValidPath(path))
		{
			return 0;
		}
		if (!std::filesystem::is_regular_file(path))
		{
			std::cout << "Path: " << path << " Is not a file" << "\n";
			return 0;
		}
		uintmax_t size = std::filesystem::file_size(path);

		return size;

	}
}
