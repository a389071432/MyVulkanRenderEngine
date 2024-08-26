#pragma once
#include"Resource.h"
#include"CommandBuffer.h"
#include"FileHandler.h"
#include<unordered_map>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include"utils/utils.h"


namespace zzcVulkanRenderEngine {
	// Default configs
	const u32 DEFAULT_POOL_SIZE = 512;
	const u32 MAX_UNIFORM_BUFFER_DESCRIPTORS = 100;
	const u32 MAX_IMAGE_SAMPLER_DESCRIPTORS = 100;
	const u32 MAX_SETS = 50;
	const u32 MAX_FRAME_IN_FLIGHT = 2;
	const u32 SWAPCHAIN_IMAGES = 3;

	// TODO: fill in this 
	struct GPUDeviceCreation {
		u32 requireQueueFamlies = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
		std::vector<const char*> requiredLayers = { "VK_LAYER_KHRONOS_validation" };
#ifdef ENABLE_RAYTRACING
		std::vector<const char*> requiredExtensions = { 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
	};
#else
		std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#endif // ENABLE_RAYTRACING

	};

	struct QueueInfo {
		u32 familyIndex;
		u32 queueIndex;
	};

	// TODO: add more types
	// is it necessary to add a separate queue for raytracing? given that it is performed by compute queue
	struct QueueFamilyInfos {
		QueueInfo mainQueue;
		QueueInfo computeQueue;
		QueueInfo raytracingQueue;
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
		void init();

		// methods for picking or setting member variables, used by Engine
		VkDevice getDevice();
		VkSwapchainKHR& getSwapChain();
		CommandBuffer& getCommandBuffer(u32 index);
		VkQueue getMainQueue();
		VkQueue getPresentQueue();
		void setWindow(GLFWwindow* window);

		// Resource creation
		TextureHandle createTexture(const TextureCreation createInfo);
		BufferHandle createBuffer(const BufferCreation createInfo);
		DescriptorSetLayoutsHandle createDescriptorSetLayouts(const DescriptorSetLayoutsCreation createInfo);
		DescriptorSetsHandle createDescriptorSets(const DescriptorSetsAlloc allocInfo);
		void writeDescriptorSets(const std::vector<DescriptorSetWrite>& writes, DescriptorSetsHandle setsHandle);
		RenderPassHandle createRenderPass(const RenderPassCreation createInfo);
		PipelineLayoutHandle createPipelineLayout(const PipelineLayoutCreation createInfo);
		GraphicsPipelineHandle createGraphicsPipeline(const GraphicsPipelineCreation createInfo);
		RayTracingPipelineHandle createRayTracingPipeline(const RayTracingPipelineCreation createInfo);
		FramebufferHandle createFramebuffer(const FramebufferCreation createInfo);
		VkSampler createSampler(SamplerCreation createInfo);

		// Resource access
		TextureHandle requireTexture();
		BufferHandle requireBuffer();
		DescriptorSetLayoutsHandle requireDescriptorSetLayouts();
		DescriptorSetsHandle requireDescriptorSets();
		RenderPassHandle requireRenderPass();
		FramebufferHandle requireFramebuffer();
		PipelineLayoutHandle requirePipelineLayout();
		GraphicsPipelineHandle requireGraphicsPipeline();
		RayTracingPipelineHandle requireRayTracingPipeline();
		Texture& getTexture(TextureHandle handle);
		Buffer& getBuffer(BufferHandle handle);
		std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts(DescriptorSetLayoutsHandle handle);
		std::vector<VkDescriptorSet>& getDescriptorSets(DescriptorSetsHandle handle);
		VkRenderPass& getRenderPass(RenderPassHandle handle);
		VkFramebuffer& getFramebuffer(FramebufferHandle handle);
		VkPipelineLayout& getPipelineLayout(PipelineLayoutHandle handle);
		VkPipeline& getGraphicsPipeline(GraphicsPipelineHandle handle);
		VkPipeline& getRayTracingPipeline(RayTracingPipelineHandle handle);

		// Resource removal
		void removeBuffer(BufferHandle handle);

		// Swapchain access
		TextureHandle getSwapChainImageByIndex(u32 index);
		VkExtent2D getSwapChainExtent();

		// queue family index access
		u32 getGraphicsQueueFamilyIndex();
		u32 getComputeQueueFamilyIndex();
		u32 getRaytracingQueueFamilyIndex();
		u32 getPresentQueueFamilyIndex();

		//helper functions for application-level requests
		template<typename T>
		BufferHandle& createBufferFromData(const std::vector<T>& data, BufferUsage usage) {

			VkDeviceSize bufferSize = VkDeviceSize(sizeof(T) * data.size());

			//create vertex buffer (inaccessable by host, exclusive by GPU)
			BufferCreation mainCI{};
			mainCI.setSize(bufferSize)
				.setUsage(util_getBufferUsage(usage) | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
				.setProp(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				.setShareMode(ResourceSharingMode::EXCLUSIVE);
			BufferHandle main = createBuffer(mainCI);
			Buffer& mainBuffer = getBuffer(main);

			//create staging buffer (accessible by host, only used for transfering vertex data from host to GPU)
			BufferCreation stageCI{};
			stageCI.setSize(bufferSize)
				.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
				.setProp(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
				.setShareMode(ResourceSharingMode::EXCLUSIVE);
			BufferHandle stage = createBuffer(stageCI);
			Buffer& stageBuffer = getBuffer(stage);

			//fill in the staging buffer
			void* pData;
			vkMapMemory(device, stageBuffer.mem, 0, bufferSize, 0, &pData);
			memcpy(pData, data.data(), bufferSize);
			vkUnmapMemory(device, stageBuffer.mem);

			auxiCmdBuffer.reset();
			auxiCmdBuffer.begin();
			//transfer the vertices data from staging buffer to vertex buffer (by sumbitting commands)
			transferBufferInDevice(auxiCmdBuffer,stage, main, bufferSize);
			auxiCmdBuffer.end();

			// submit to queue
			submitCmds(mainQueue, auxiCmdBuffer.getCmdBuffer());
			vkQueueWaitIdle(mainQueue);

			//release the stagging buffer and free the memory
			removeBuffer(stage);

			return main;
		}

		template<typename T>
		TextureHandle& createTexture2DFromData(const std::vector<T>& data, u16 width, u16 height, u16 nMips, DataFormat format) {
			//create a staging buffer
			BufferCreation stageCI{};
			VkDeviceSize bufferSize = VkDeviceSize(sizeof(T) * data.size());
			stageCI.setSize(bufferSize)
				.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
				.setProp(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
				.setShareMode(ResourceSharingMode::EXCLUSIVE);
			BufferHandle stage = createBuffer(stageCI);
			Buffer& stageBuffer = getBuffer(stage);

			//fill in the staging buffer
			void* pData;
			vkMapMemory(device, stageBuffer.mem, 0, bufferSize, 0, &pData);
			memcpy(pData, data.data(), bufferSize);
			vkUnmapMemory(device, stageBuffer.mem);

			// create the image
			TextureCreation texCI{};
			texCI.setType(TextureType::Texture2D)
				.setUsage(VK_IMAGE_USAGE_SAMPLED_BIT)    // typically, texture created from data is used to sample from
				.setFormat(format)
				.setSize(width, height, 1)
				.setMipLevels(nMips);
			TextureHandle texHandle = createTexture(texCI);
			Texture& tex = getTexture(texHandle);

			auxiCmdBuffer.reset();
			auxiCmdBuffer.begin();
			// transfer data in device
			transferBufferToImage2DInDevice(auxiCmdBuffer,stage, texHandle, width, height);

			// release the staging buffer
			removeBuffer(stage);

			// generate mip levels if requried
			if (nMips > 1) {
				// set layout transition for all mip levels
				// so that the i-th mip level will be transitioned to the layout COPY_DST when performing the copy from i-1 to i
				imageLayoutTransition(auxiCmdBuffer, texHandle, GraphResourceAccessType::COPY_DST, 0, nMips);
				helper_generateMipMaps(auxiCmdBuffer, texHandle, nMips);
			}
			// since the texture will be read by shader during rendering
			// insert barrier to transition it to appropriate layout to be ready for sampling
			imageLayoutTransition(auxiCmdBuffer, texHandle, GraphResourceAccessType::READ_TEXTURE, 0, nMips);
			auxiCmdBuffer.end();

			// submit commands to queue
			submitCmds(mainQueue, auxiCmdBuffer.getCmdBuffer());
			vkQueueWaitIdle(mainQueue);

			return texHandle;
		}

		BufferHandle createRayTracingShaderBindingTable(
			RayTracingShaderBindingTableCreation sbtCI,
			VkStridedDeviceAddressRegionKHR* rgenShaderRegion,
		    VkStridedDeviceAddressRegionKHR* missShaderRegion,
		    VkStridedDeviceAddressRegionKHR* hitShaderRegion
			);

		//void transferBufferInDevice(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize copySize);

		// need to specify which cmdBuffer to use 
		void transferBufferInDevice(CommandBuffer& cmdBuffer, BufferHandle srcBuffer, BufferHandle dstBuffer, VkDeviceSize copySize);
		void transferImageInDevice(CommandBuffer& cmdBuffer, TextureHandle src, TextureHandle dst, VkExtent2D copyExtent);
		void transferBufferToImage2DInDevice(CommandBuffer& cmdBuffer, BufferHandle buffer, TextureHandle tex, u32 width, u32 height);
		float queryMaxAnisotropy();
		void submitCmds(VkQueue queue, std::vector<VkCommandBuffer> cmdBuffes, std::vector<VkSemaphore> waitSemas, std::vector<VkPipelineStageFlags> waitStages, std::vector<VkSemaphore> signalSemas, VkFence fence);
		void submitCmds(VkQueue queue, VkCommandBuffer cmdBuffe);
		void imageLayoutTransition(CommandBuffer& cmdBuffer, TextureHandle tex, GraphResourceAccessType targetAccess, u16 baseMip, u16 nMips);
		//internal hepler functions
		uint32_t helper_findSuitableMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


	private:
		// Default settings 
		// may be later moved to Engine class
		const u32 poolSize = DEFAULT_POOL_SIZE;

		const u32 frameInFlight = MAX_FRAME_IN_FLIGHT;
		const u32 nSwapChainImages = SWAPCHAIN_IMAGES;

		// store the settings
		u32 enabledQueueFamlies = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
		std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::vector<const char*> enabledLayers = { "VK_LAYER_KHRONOS_validation" };

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
		//VmaAllocator vmaAllocator;

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
#ifdef ENABLE_RAYTRACING
		VkQueue raytracingQueue;
#endif // ENABLE_RAYTRACING


		// Command buffer related
		VkCommandPool commandPool;
		std::vector<CommandBuffer> cmdBuffers;
		CommandBuffer auxiCmdBuffer;  //auxiliary command buffer

		// Descriptor pool
		VkDescriptorPool descriptorPool;
		u32 maxUBDescritors = MAX_UNIFORM_BUFFER_DESCRIPTORS;
		u32 maxSamplerDescritors = MAX_IMAGE_SAMPLER_DESCRIPTORS;
		u32 maxSets = MAX_SETS;

		// Function pointers for extensions
#ifdef ENABLE_RAYTRACING
		PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
		PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
#endif // ENABLE_RAYTRACING


		// Resource management
		ResourcePool<Texture, TextureHandle> texturePool;
		ResourcePool<Buffer, BufferHandle> bufferPool;
		ResourcePool<std::vector<VkDescriptorSetLayout>, DescriptorSetLayoutsHandle> descriptorSetLayoutsPool;
		ResourcePool<std::vector<VkDescriptorSet>, DescriptorSetsHandle> descriptorSetsPool;
		ResourcePool<VkRenderPass, RenderPassHandle> renderPassPool;
		ResourcePool<VkFramebuffer, FramebufferHandle> framebufferPool;
		ResourcePool<VkPipelineLayout, PipelineLayoutHandle> pipelineLayoutPool;
		ResourcePool<VkPipeline, GraphicsPipelineHandle> graphicsPipelinePool;
		ResourcePool<VkPipeline, ComputePipelineHandle> computePipelinePool;
		ResourcePool<VkPipeline, RayTracingPipelineHandle> rayTracingPipelinePool;

		// helpers (invisible to application-level programmers)
		bool helper_checkQueueSatisfication(VkPhysicalDevice phyDevice,u32 requiredQueues);
		bool helper_checkExtensionSupport(VkPhysicalDevice phyDevice, const std::vector<const char*>& requiredExtensions);
		SwapChainSupportDetails helper_querySwapChainSupport(VkPhysicalDevice phyDevice);
		VkSurfaceFormatKHR helper_selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR helper_selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes);
		VkExtent2D helper_selectSwapExtent(const VkSurfaceCapabilitiesKHR capabilities);
		QueueFamilyInfos helper_selectQueueFamilies(VkPhysicalDevice phyDevice, u32 requiredQueues);
		std::vector<VkDeviceQueueCreateInfo> helper_getQueueCreateInfos(QueueFamilyInfos queueInfos,u32 requiredQueues);
		VkShaderModule helper_createShaderModule(const std::vector<char>& code);
		void helper_generateMipMaps(CommandBuffer& cmdBuffer, TextureHandle tex, u16 nMips);
		bool helper_checkInstanceLayerSupport(const std::vector<const char*>& requiredLayers);
		std::vector<const char*> helper_getRequiredInstanceExtensions(bool enableValidation);
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR helper_getPhyDeviceRayTracingProperties(VkPhysicalDevice& phyDevice);
	};
}
