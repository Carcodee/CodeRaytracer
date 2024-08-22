#include "FileHandler.h"
#include "VulkanAPI/ResourcesManagers/Assets/AssetsHandler.h"

#include <fstream>
#include <iostream>


namespace HELPERS
{

	FileHandler* FileHandler::instance = nullptr;

	FileHandler::FileHandler()
	{

		workingDir = std::filesystem::current_path();
        //TODO: fix this when is launched from exe
		projectPath = workingDir.parent_path();
		assetPath = projectPath / "Core"/"Source" / "Resources" / "Assets";
        engineResourcesPath = projectPath / "Core"/"Source" / "Resources" / "EngineResources";
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

	void FileHandler::DeleteLinesFromFile(std::string path, const std::vector<int>& linesToDelete)
	{
		std::ifstream inputFile(path);
		if (!inputFile)
		{
			std::cout << "filename: " << path << " could not opened \n";
		}
		int lineNumber;
		std::string line;
		std::vector<std::string> newFile;
		while (std::getline(inputFile, line))
		{
			for (auto value : linesToDelete)
			{
				if (value!=lineNumber)
				{
					newFile.push_back(line);
				}
				
			}
			lineNumber++;
			
		}
		std::ofstream outputFile(path);
		if (!outputFile)
		{
			std::cout << "filename: " << path << " could not be changed \n";
		}
		for (auto line : newFile)
		{
			outputFile << line;
		}
		std::cout << "filename: " << path << " changed lines successfully \n";

	}

	void FileHandler::CreateFile(std::string path, std::string data)
	{
		std::ofstream outFile(path.c_str());
		if (!outFile)
		{
			std::cout << "filename: " << path << " could not be created \n";
            outFile.close();
            return;
		}
		outFile << data << "\n";

		outFile.close();

		std::cout << "filename: " << path << " was successfully created \n";

	}

	void FileHandler::AppendToFile(std::string path, std::string data)
	{
		std::ofstream outFile(path.c_str(), std::ios::app);
		if (!outFile)
		{
			std::cout << "filename: " << path << " could not be appended \n";
            outFile.close();
            return;
		}
		outFile << data << "\n";

		outFile.close();

		std::cout << "filename: " << path << " was successfully appended \n";

	}

	std::string FileHandler::ReadFile(std::string path)
	{
		std::ifstream inFile(path);
		std::string data;


		while (std::getline(inFile, data))
		{
			
		}
		return data;
		
	}

	bool FileHandler::IsPathInAssets(std::string path)
	{
		std::filesystem::path relativeToAssetsPath(path);
		relativeToAssetsPath = assetPath / relativeToAssetsPath;

		if (!std::filesystem::exists(relativeToAssetsPath))
		{
			return false;
		}
		return true;

	}

	bool FileHandler::IsPathAbsolute(std::string path)
	{
		if (!std::filesystem::exists(path))
		{
			return false;
		}
		return true;
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
		if (!VULKAN::AssetsHandler::GetInstance()->IsValidModelFormat(extension))
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
		std::string extension = path.erase(0, extensionPos);
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

    std::string FileHandler::RemovePathExtension(std::filesystem::path path) {
        std::string strPath= path.string();
        
        size_t extensionPos= strPath.find_last_of(".");
        strPath = strPath.substr(0, extensionPos);
        
        return strPath;
    }

    std::string FileHandler::GetEngineResourcesPath() {
        return engineResourcesPath.string();
    }
}
