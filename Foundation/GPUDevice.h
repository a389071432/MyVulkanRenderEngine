#pragma once
#include"vulkan/vulkan_core.h"
#include"vk_mem_alloc.h"
#include"Resource.h"
#include"CommandBuffer.h"

namespace zzcVulkanRenderEngine {
	// Default configs
	const u32 defaultPoolSize = 256;
	const u32 maxFrameInFlight = 2;

	// TODO: fill in this 
	struct GPUDeviceCreation {

	};

	class GPUDevice {
	public:
		GPUDevice(GPUDeviceCreation createInfo);
		~GPUDevice();

		// Resource creation
		TextureHandle createTexture(TextureCreation createInfo);
		BufferHandle createBuffer();

		// Resource access
		TextureHandle requireTexture();
		BufferHandle requireBuffer();
		Texture& getTexture(TextureHandle handle);
		Buffer& getBuffer(BufferHandle handle);

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

		// Descriptor pool
		VkDescriptorPool descriptorPool;

		// Resource management
		ResourcePool<Texture,TextureHandle> texturePool;
		ResourcePool<Buffer,BufferHandle> bufferPool;
		ResourcePool<std::vector<VkDescriptorSet>, DescriptorSetsHandle> descriptorSetsPool;

	};
}
