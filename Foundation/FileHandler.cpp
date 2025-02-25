#include <fstream>
#include"FileHandler.h"

namespace zzcVulkanRenderEngine {
	FileHandler::FileHandler() {

	}

	FileHandler::~FileHandler() {

	}

	std::vector<char> FileHandler::read(std::string filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
	}
}