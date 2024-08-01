#pragma once
#include <filesystem>
#include <vector>

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
        std::string GetEngineResourcesPath();
		bool IsPathInAssets(std::string path);
		bool IsPathAbsolute(std::string path);
		bool IsValidPath(std::string path);
		std::string HandleModelFilePath(std::string path);
		std::string GetPathExtension(std::string path);
		uintmax_t GetFileSize(std::string path);
		static FileHandler* GetInstance();
		FileHandler(FileHandler& other) = delete;
		void DeleteLinesFromFile(std::string path, const std::vector<int>& linesToDelete);
		void operator=(const FileHandler&) = delete;
		std::filesystem::path currentPathRelativeToAssets;

		void CreateFile(std::string path,std::string data = "");
		void AppendToFile(std::string path,std::string data);
        std::string RemovePathExtension(std::filesystem::path path);
		std::string ReadFile(std::string path);


	private:

		std::filesystem::path workingDir;
		std::filesystem::path projectPath;
		std::filesystem::path assetPath;
        std::filesystem::path engineResourcesPath;
		std::filesystem::path shadersPath;
		
		};

}
