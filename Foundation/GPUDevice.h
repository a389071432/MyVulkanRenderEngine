#pragma once
#include"vulkan/vulkan_core.h"
#include"vk_mem_alloc.h"
#include"Resource.h"
#include"CommandBuffer.h"
#include"FileHandler.h"

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
		TextureHandle createTexture(const TextureCreation createInfo);
		BufferHandle createBuffer();
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

	private:
		// File handler
		FileHandler fileHandler;

		//Vulkan instance 
		VkInstance vkInstance;

		// Swapchain size (provided by Engine)
		u32 swapChainWidth;
		u32 swapChainHeight;

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
		ResourcePool<Texture, TextureHandle> texturePool;
		ResourcePool<Buffer, BufferHandle> bufferPool;
		ResourcePool<std::vector<VkDescriptorSet>, DescriptorSetsHandle> descriptorSetsPool;
		ResourcePool<VkPipeline, GraphicsPipelineHandle> graphicsPipelinePool;
		ResourcePool<VkPipeline, ComputePipelineHandle> computePipelinePool;

		// helpers
		VkShaderModule helper_createShaderModule(const std::vector<char>& code);
	};
}
