#pragma once
#include"vulkan/vulkan_core.h"
#include"vk_mem_alloc.h"
#include"Resource.h"
#include"CommandBuffer.h"
#include"FileHandler.h"
#include<unordered_map>

namespace zzcVulkanRenderEngine {
	// Default configs
	const u32 DEFAULT_POOL_SIZE = 256;
	const u32 MAX_FRAME_IN_FLIGHT = 2;
	const u32 SWAPCHAIN_IMAGES = 3;

	// TODO: fill in this 
	struct GPUDeviceCreation {
		u32 requireQueueFamlies = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
		std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};

	struct QueueInfo {
		u32 familyIndex;
		u32 queueIndex;
	};
	// TODO: add more types
	struct QueueFamilyInfos {
		QueueInfo mainQueue;
		QueueInfo computeQueue;
		QueueInfo transferQueue;
		QueueInfo presentQueue;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class GPUDevice {
	public:
		GPUDevice(GPUDeviceCreation createInfo);
		~GPUDevice();

		// methods for picking members, used by Engine
		VkDevice getDevice();
		VkSwapchainKHR getSwapChain();
		CommandBuffer& getCommandBuffer(u32 index);
		VkQueue getMainQueue();
		VkQueue getPresentQueue();

		// Resource creation
		TextureHandle createTexture(const TextureCreation createInfo);
		BufferHandle createBuffer(const BufferCreation createInfo);
		DescriptorSetLayoutsHandle createDescriptorSetLayouts(const DescriptorSetLayoutsCreation createInfo);
		DescriptorSetsHandle createDescriptorSets(const DescriptorSetsAlloc allocInfo);
		void writeDescriptorSets(const std::vector<DescriptorSetWrite>& writes, DescriptorSetsHandle setsHandle);
		RenderPassHandle createRenderPass(const RenderPassCreation createInfo);
		PipelineLayoutHandle createPipelineLayout(const PipelineLayoutCreation createInfo);
		GraphicsPipelineHandle createGraphicsPipeline(const GraphicsPipelineCreation createInfo);
		FramebufferHandle createFramebuffer(const FramebufferCreation createInfo);

		// Resource access
		TextureHandle requireTexture();
		BufferHandle requireBuffer();
		DescriptorSetLayoutsHandle requireDescriptorSetLayouts();
		DescriptorSetsHandle requireDescriptorSets();
		RenderPassHandle requireRenderPass();
		FramebufferHandle requireFramebuffer();
		PipelineLayoutHandle requirePipelineLayout();
		GraphicsPipelineHandle requireGraphicsPipeline();
		Texture& getTexture(TextureHandle handle);
		Buffer& getBuffer(BufferHandle handle);
		std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts(DescriptorSetLayoutsHandle handle);
		std::vector<VkDescriptorSet>& getDescriptorSets(DescriptorSetsHandle handle);
		VkRenderPass& getRenderPass(RenderPassHandle handle);
		VkFramebuffer& getFramebuffer(FramebufferHandle handle);
		VkPipelineLayout& getPipelineLayout(PipelineLayoutHandle& handle);
		VkPipeline& getGraphicsPipeline(GraphicsPipelineHandle handle);

		// Resource removal
		void removeBuffer(BufferHandle handle);

		// Swapchain access
		TextureHandle getSwapChainImageByIndex(u32 index);
		VkExtent2D getSwapChainExtent();

		//helper functions for application-level operations
		template<typename T>
		BufferHandle& createBufferFromData(const std::vector<T>& data);
		void transferBufferInDevice(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize copySize);
		void transferImageInDevice(TextureHandle src, TextureHandle dst, VkExtent2D copyExtent);

		//internal hepler functions
		uint32_t helper_findSuitableMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


	private:
		// Default settings 
		// may be later moved to Engine class
		const u32 frameInFlight = MAX_FRAME_IN_FLIGHT;
		const u32 nSwapChainImages = SWAPCHAIN_IMAGES;

		// File handler
		FileHandler fileHandler;

		//Vulkan instance 
		VkInstance vkInstance;

		// Window surface
		GLFWwindow* window;
		VkSurfaceKHR windowSurface;

		// Swapchain related
		u32 swapChainWidth;
		u32 swapChainHeight;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainFormat;
		VkExtent2D swapChainExtent;
		std::unordered_map<u32, u32>index2handle_swapchain;

		// GPU memory allocator. Currently using a default one provided by VMA
		VmaAllocator vmaAllocator;

		// Device related
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties deviceProperties;
		VkDevice device;

		// Queue related
		QueueFamilyInfos queueFamilyInfos;
		VkQueue mainQueue;
		VkQueue computeQueue;
		VkQueue transferQueue;
		VkQueue presentQueue;

		// Command buffer related
		VkCommandPool commandPool;
		std::vector<CommandBuffer> cmdBuffers;
		CommandBuffer auxiCmdBuffer;  //auxiliary command buffer

		// Descriptor pool
		VkDescriptorPool descriptorPool;

		// Resource management
		ResourcePool<Texture, TextureHandle> texturePool;
		ResourcePool<Buffer, BufferHandle> bufferPool;
		ResourcePool<std::vector<VkDescriptorSet>, DescriptorSetsHandle> descriptorSetsPool;
		ResourcePool<VkPipeline, GraphicsPipelineHandle> graphicsPipelinePool;
		ResourcePool<VkPipeline, ComputePipelineHandle> computePipelinePool;

		// helpers
		bool helper_checkQueueSatisfication(VkPhysicalDevice phyDevice,u32 requiredQueues);
		bool helper_checkExtensionSupport(VkPhysicalDevice phyDevice, const std::vector<const char*>& requiredExtensions);
		SwapChainSupportDetails helper_querySwapChainSupport(VkPhysicalDevice phyDevice);
		VkSurfaceFormatKHR helper_selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR helper_selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes);
		VkExtent2D helper_selectSwapExtent(const VkSurfaceCapabilitiesKHR capabilities);
		QueueFamilyInfos helper_selectQueueFamilies(VkPhysicalDevice phyDevice, u32 requiredQueues);
		std::vector<VkDeviceQueueCreateInfo>& helper_getQueueCreateInfos(QueueFamilyInfos queueInfos,u32 requiredQueues);
		VkShaderModule helper_createShaderModule(const std::vector<char>& code);
	};
}
