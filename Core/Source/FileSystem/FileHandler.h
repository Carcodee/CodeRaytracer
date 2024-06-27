#pragma once
#include <filesystem>

namespace HELPERS 
{

	class FileHandler
	{

	protected:
		static FileHandler* instance;


		FileHandler();

	public:


		std::string GetShadersPath();
		std::string GetAssetsPath();
		bool IsPathInAssets(std::string path);
		bool IsPathAbsolute(std::string path);
		std::string HandleModelFilePath(std::string path);
		std::string GetPathExtension(std::string path);
		static FileHandler* GetInstance();
		FileHandler(FileHandler& other) = delete;
		void operator=(const FileHandler&) = delete;

	private:

		std::filesystem::path workingDir;
		std::filesystem::path projectPath;
		std::filesystem::path assetPath;
		std::filesystem::path shadersPath;
	};

}
