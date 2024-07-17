#pragma once
#include<string>
#include<vector>

namespace zzcVulkanRenderEngine {
	class FileHandler {
	public:
		FileHandler();
		~FileHandler();
		std::vector<char> read(std::string filePath);
	};
}