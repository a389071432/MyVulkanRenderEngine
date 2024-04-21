#pragma once
#include"vulkan/vulkan_core.h"
#include"vk_mem_alloc.h"
#include"Resource.h"
#include"CommandBuffer.h"

namespace zzcVulkanRenderEngine {
	// Default configs
	const u32 defaultPoolSize = 256;
	const u32 maxFrameInFlight = 2;

	class GPUDevice {
	public:
		GPUDevice();
		~GPUDevice();

		// Resource creation
		ResourceHandle createTexture(ResourceHandle texToCreate);
		ResourceHandle createTexture(ResourceHandle texToCreate, ResourceHandle texAlias);
		ResourceHandle createBuffer();

		// Resource access
		ResourceHandle requireTexture();
		ResourceHandle requireBuffer();
		Texture& getTexture(ResourceHandle handle);
		Buffer& getBuffer(ResourceHandle handle);

	private:
		//Vulkan instance 
		VkInstance vkInstance;

		// GPU memory allocator. Currently using a default one provided by VMA
		VmaAllocator vmaAllocator;
		
		// Device related
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties deviceProperties;
		VkDevice device;

		// Command buffers
		std::vector<CommandBuffer> cmdBuffers;

		// Resource management
		ResourcePool<Texture> textures;
		ResourcePool<Buffer> buffers;

	};
}
