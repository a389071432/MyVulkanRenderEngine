#pragma once
#include<string>
#include<vector>

namespace zzcVulkanRenderEngine {
	class FileHandler {
	public:
		FileHandler();
		~FileHandler();
		static std::vector<char> read(std::string filePath);
	};
}