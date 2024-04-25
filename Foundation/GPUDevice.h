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
		TextureHandle createTexture(TextureCreation createInfo);
		BufferHandle createBuffer();
		DescriptorSetLayoutsHandle createDescriptorSetLayouts(DescriptorSetLayoutsCreation createInfo);
		DescriptorSetsHandle createDescriptorSets(DescriptorSetsAlloc allocInfo);
		void writeDescriptorSets(std::vector<DescriptorSetWrite>& writes, DescriptorSetsHandle setsHandle);
		PipelineLayoutHandle createPipelineLayout(PipelineLayoutCreation createInfo);
		GraphicsPipelineHandle createGraphicsPipeline(GraphicsPipelineCreation createInfo);

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
