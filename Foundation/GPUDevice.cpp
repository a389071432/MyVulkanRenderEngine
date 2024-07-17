#include"GPUDevice.h"
#include"assert.h"
#include"utils/utils.h"
#include<array>
#include<algorithm>
#include<Windows.h>
#include<vulkan/vulkan_win32.h>
#include<array>
#include<limits>

namespace zzcVulkanRenderEngine {
	GPUDevice::GPUDevice(GPUDeviceCreation createInfo) 
		:texturePool(poolSize)
		,bufferPool(poolSize)
	    ,descriptorSetsPool(poolSize)
	    ,graphicsPipelinePool(poolSize)
	    ,computePipelinePool(poolSize)
	    ,descriptorSetLayoutsPool(poolSize)
	    ,renderPassPool(poolSize)
	    ,framebufferPool(poolSize)
	    ,pipelineLayoutPool(poolSize)
	{

		// TODO: fill in createInfos

		cmdBuffers.resize(frameInFlight);

		// TODO: create windowSurface
		//VkWin32SurfaceCreateInfoKHR surfaceCI{};
		//surfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		//surfaceCI.hwnd = glfwGetWin32Window(window);
		//surfaceCI.hinstance = GetModuleHandle(nullptr);
		//ASSERT(vkCreateWin32SurfaceKHR(vkInstance, &surfaceCI, nullptr, &windowSurface) == VK_SUCCESS,
		//	"Failed to create the window surface!"
		//);
		
		ASSERT(glfwCreateWindowSurface(vkInstance, window, nullptr, &windowSurface) != VK_SUCCESS,
			"Failed to create the window surface!"
		);
		
		
		// CREATE VULKAN INSTANCE
		// TODO: glfw extensions
		// enable the validation layer
		ASSERT(
			helper_checkInstanceLayerSupport(createInfo.requiredLayers) == true,
			"Assertion failed: required layers are not fully available!"
		);

		std::vector<const char*> instanceExtensions = helper_getRequiredInstanceExtensions(true);

		VkInstanceCreateInfo instanceCI{};
		instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCI.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		instanceCI.ppEnabledExtensionNames = instanceExtensions.data();
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_3;
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pApplicationName = "zzc Vulkan Render Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "zzc Vulkan Render Engine";
		instanceCI.pApplicationInfo = &appInfo;
		instanceCI.enabledLayerCount = createInfo.requiredLayers.size();
		instanceCI.ppEnabledLayerNames = createInfo.requiredLayers.data();


		ASSERT(
			vkCreateInstance(&instanceCI, nullptr, &vkInstance) == VK_SUCCESS,
			"Assertion failed: CreateInstanceFailed!"
		);

		// PICK PHYSICAL DEVICE
		// TODO: check for extension support
		uint32_t phyDeviceCnt;
		ASSERT(
			vkEnumeratePhysicalDevices(vkInstance, &phyDeviceCnt, nullptr)==VK_SUCCESS,
			"Assertion failed: EnumeratePhysicalDevices failed!"
		);
		std::vector<VkPhysicalDevice>physicalDevices;
		ASSERT(
			vkEnumeratePhysicalDevices(vkInstance, &phyDeviceCnt, physicalDevices.data()) == VK_SUCCESS,
			"Assertion failed: EnumeratePhysicalDevices failed!"
		);
		VkPhysicalDevice discreteGPU = VK_NULL_HANDLE;
		VkPhysicalDevice integrateGPU = VK_NULL_HANDLE;
		for (const auto& phyDevice : physicalDevices) {
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceProperties(phyDevice, &properties);
			vkGetPhysicalDeviceFeatures(physicalDevice, &features);
			if (
				properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
				&& helper_checkQueueSatisfication(phyDevice,createInfo.requireQueueFamlies)
				&& helper_checkExtensionSupport(phyDevice,createInfo.requiredExtensions)) {
				discreteGPU = phyDevice;
				break;
			}
			if (
				properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU 
				&& helper_checkQueueSatisfication(phyDevice, createInfo.requireQueueFamlies)
				&& helper_checkExtensionSupport(phyDevice, createInfo.requiredExtensions)) {
				integrateGPU = phyDevice;
			}
		}
		if (discreteGPU != VK_NULL_HANDLE) {
			physicalDevice = discreteGPU;
		}
		else if (integrateGPU != VK_NULL_HANDLE) {
			physicalDevice = integrateGPU;
		}
		else {
			ASSERT(false, "Assertion failed: no GPU detected!");
		}

		// CREATE LOGICAL DEVICE (study details before doing this)
		// TODO: enable a set of extensions (study Raptor for details)
		queueFamilyInfos = helper_selectQueueFamilies(physicalDevice, createInfo.requireQueueFamlies);
		std::vector<VkDeviceQueueCreateInfo> queueCIs = helper_getQueueCreateInfos(queueFamilyInfos,createInfo.requireQueueFamlies);
		VkDeviceCreateInfo deviceCI{};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size());
		deviceCI.pQueueCreateInfos = queueCIs.data();
		deviceCI.enabledExtensionCount = static_cast<uint32_t>(createInfo.requiredExtensions.size());
		deviceCI.ppEnabledExtensionNames = createInfo.requiredExtensions.data();

		ASSERT(
			vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device) == VK_SUCCESS,
			"Assertion failed: CreateDevice failed!"
		);

		vkGetDeviceQueue(device, queueFamilyInfos.presentQueue.familyIndex, queueFamilyInfos.presentQueue.queueIndex, &presentQueue);
		if (createInfo.requireQueueFamlies & VK_QUEUE_GRAPHICS_BIT) 
			vkGetDeviceQueue(device, queueFamilyInfos.mainQueue.familyIndex, queueFamilyInfos.mainQueue.queueIndex, &mainQueue);
		if (createInfo.requireQueueFamlies & VK_QUEUE_COMPUTE_BIT)
			vkGetDeviceQueue(device, queueFamilyInfos.computeQueue.familyIndex, queueFamilyInfos.computeQueue.queueIndex, &computeQueue);
		if (createInfo.requireQueueFamlies & VK_QUEUE_TRANSFER_BIT)
			vkGetDeviceQueue(device, queueFamilyInfos.transferQueue.familyIndex, queueFamilyInfos.transferQueue.queueIndex, &transferQueue);


		//// Create VMA allocator
		//VmaAllocatorCreateInfo allocatorCI = {};
		//allocatorCI.physicalDevice = physicalDevice;
		//allocatorCI.device = device;
		//allocatorCI.instance = vkInstance;
		//ASSERT(
		//	vmaCreateAllocator(&allocatorCI, &vmaAllocator) == VK_SUCCESS,
		//	"Assertion failed: VMA Allocator creation failed!"
		//);

		// TODO: Create descriptorPool
		std::array<VkDescriptorPoolSize, 2> poolSizes;

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = maxUBDescritors;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = maxSamplerDescritors;

		VkDescriptorPoolCreateInfo descPoolCI{};
		descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCI.poolSizeCount = 2;
		descPoolCI.pPoolSizes = poolSizes.data();
		descPoolCI.maxSets = maxSets;
		ASSERT(
			vkCreateDescriptorPool(device, &descPoolCI, nullptr, &descriptorPool) == VK_SUCCESS,
			"Assertion failed: failed to create descriptor pool"
		);

		// CREATE COMMANDPOOL
		VkCommandPoolCreateInfo cmdPoolCI{};
		cmdPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCI.queueFamilyIndex = static_cast<uint32_t>(queueFamilyInfos.mainQueue.familyIndex);
		cmdPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ASSERT(
			vkCreateCommandPool(device, &cmdPoolCI, nullptr, &commandPool) == VK_SUCCESS,
			"Assertion failed: CreateCommandPool failed!"
		);

        // CREATE COMMAND BUFFERS
		// TODO: multithreads
		// TODO: secondary command buffer
		// TODO: (study this for details: https://github.com/ARM-software/vulkan_best_practice_for_mobile_developers/blob/master/samples/performance/command_buffer_usage/command_buffer_usage_tutorial.md)
		cmdBuffers.resize(frameInFlight);
		for (u32 i = 0; i < cmdBuffers.size(); i++) {
			VkCommandBufferAllocateInfo cmdAllocInfo{};
			cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAllocInfo.commandBufferCount = 1;
			cmdAllocInfo.commandPool = commandPool;
			cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			VkCommandBuffer cb = cmdBuffers.at(i).getCmdBuffer();
			ASSERT(
				vkAllocateCommandBuffers(device, &cmdAllocInfo, &cb) == VK_SUCCESS,
				"Assertion failed: AllocateCommandBuffers failed!"
			);
		}

		// CREATE THE AUXILIARY COMMAND BUFFER
		VkCommandBufferAllocateInfo cmdAllocInfo{};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.commandPool = commandPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkCommandBuffer cb = auxiCmdBuffer.getCmdBuffer();
		ASSERT(
			vkAllocateCommandBuffers(device, &cmdAllocInfo, &cb) == VK_SUCCESS,
			"Assertion failed: Allocate the auxiliary command buffer failed!"
		);

		// TODO: CREATE SWAPCHAIN
		SwapChainSupportDetails swapChainSupport = helper_querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = helper_selectSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = helper_selectSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = helper_selectSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = nSwapChainImages > swapChainSupport.capabilities.minImageCount + 1 ? 
			nSwapChainImages : swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapChainCI{};
		swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCI.surface = windowSurface;

		swapChainCI.minImageCount = imageCount;
		swapChainCI.imageFormat = surfaceFormat.format;
		swapChainCI.imageColorSpace = surfaceFormat.colorSpace;
		swapChainCI.imageExtent = extent;
		swapChainCI.imageArrayLayers = 1;
		swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChainCI.preTransform = swapChainSupport.capabilities.currentTransform;
		swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCI.presentMode = presentMode;
		swapChainCI.clipped = VK_TRUE;
		swapChainCI.oldSwapchain = VK_NULL_HANDLE;

		if (queueFamilyInfos.mainQueue.familyIndex != queueFamilyInfos.presentQueue.familyIndex) {
			swapChainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCI.queueFamilyIndexCount = 2;
			std::vector<uint32_t>indices = { queueFamilyInfos.mainQueue.familyIndex ,queueFamilyInfos.presentQueue.familyIndex };
			swapChainCI.pQueueFamilyIndices = indices.data();
		}
		else {
			swapChainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		ASSERT(
			vkCreateSwapchainKHR(device, &swapChainCI, nullptr, &swapChain) == VK_SUCCESS,
			"Assertion failed: CreateSwapchain failed!"
		);

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainFormat = surfaceFormat.format;
		swapChainExtent = extent;

		// swapchain images are also managed through texture pool
		for (int i = 0; i < swapChainImages.size();i++) {
			VkImage& swapImage = swapChainImages[i];
			TextureHandle handle = requireTexture();
			Texture& texture = getTexture(handle);

			texture.format = surfaceFormat.format;
			texture.sampler = VK_NULL_HANDLE;
			texture.access[0] = GraphResourceAccessType::PRESENT;  
			texture.image = swapChainImages[i];

			index2handle_swapchain.insert({i,handle});
		}
	}

	GPUDevice::~GPUDevice() {

	}

	VkDevice GPUDevice::getDevice() {
		return device;
	}

	VkSwapchainKHR GPUDevice::getSwapChain() {
		return swapChain;
	}

	TextureHandle GPUDevice::getSwapChainImageByIndex(u32 index) {
		auto it = index2handle_swapchain.find(index);
		ASSERT(it != index2handle_swapchain.end(), "Invalid external input resource (buffer)", index);
		return it->second;
	}

	VkExtent2D GPUDevice::getSwapChainExtent() {
		return swapChainExtent;
	}

	CommandBuffer& GPUDevice::getCommandBuffer(u32 index) {
		ASSERT(index >= 0 && index < cmdBuffers.size(), "Invalid index for getting commandBuffer!");
		return cmdBuffers.at(index);
	}

	VkQueue GPUDevice::getMainQueue() {
		return mainQueue;
	}

	VkQueue GPUDevice::getPresentQueue() {
		return presentQueue;
	}

	TextureHandle GPUDevice::requireTexture() {
		return texturePool.require_resource();
	}

	BufferHandle GPUDevice::requireBuffer() {
		return bufferPool.require_resource();
	}

	DescriptorSetLayoutsHandle GPUDevice::requireDescriptorSetLayouts() {
		return descriptorSetLayoutsPool.require_resource();
	}

	RenderPassHandle GPUDevice::requireRenderPass() {
		return renderPassPool.require_resource();
	}

	FramebufferHandle GPUDevice::requireFramebuffer() {
		return framebufferPool.require_resource();
	}

	PipelineLayoutHandle GPUDevice::requirePipelineLayout() {
		return pipelineLayoutPool.require_resource();
	}

	GraphicsPipelineHandle GPUDevice::requireGraphicsPipeline() {
		return graphicsPipelinePool.require_resource();
	}

	DescriptorSetsHandle GPUDevice::requireDescriptorSets() {
		return descriptorSetsPool.require_resource();
	}

	Texture& GPUDevice::getTexture(TextureHandle handle) {
		return texturePool.get_resource(handle);
	}

	Buffer& GPUDevice::getBuffer(BufferHandle handle) {
		return bufferPool.get_resource(handle);
	}

	std::vector<VkDescriptorSetLayout>& GPUDevice::getDescriptorSetLayouts(DescriptorSetLayoutsHandle handle) {
		return descriptorSetLayoutsPool.get_resource(handle);
	}

	VkRenderPass& GPUDevice::getRenderPass(RenderPassHandle handle) {
		return renderPassPool.get_resource(handle);
	}

	VkFramebuffer& GPUDevice::getFramebuffer(FramebufferHandle handle) {
		return framebufferPool.get_resource(handle);
	}

	VkPipelineLayout& GPUDevice::getPipelineLayout(PipelineLayoutHandle handle) {
		return pipelineLayoutPool.get_resource(handle);
	}

	VkPipeline& GPUDevice::getGraphicsPipeline(GraphicsPipelineHandle handle) {
		return graphicsPipelinePool.get_resource(handle);
	}

	std::vector<VkDescriptorSet>& GPUDevice::getDescriptorSets(DescriptorSetsHandle handle) {
		return descriptorSetsPool.get_resource(handle);
	}

	void GPUDevice::removeBuffer(BufferHandle handle) {
		Buffer& buffer = bufferPool.get_resource(handle);
		vkDestroyBuffer(device, buffer.buffer, nullptr);
		vkFreeMemory(device, buffer.mem, nullptr);
		
		bufferPool.release_resource(handle);
	}

	VkSampler GPUDevice::createSampler(SamplerCreation createInfo) {
		VkSampler sampler;
		VkSamplerCreateInfo samplerCI{};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.minFilter = createInfo.minFilter;
		samplerCI.magFilter = createInfo.magFilter;
		samplerCI.mipmapMode = createInfo.mipMode;
		samplerCI.addressModeU = createInfo.address_mode_u;
		samplerCI.addressModeV = createInfo.address_mode_v;
		samplerCI.addressModeW = createInfo.address_mode_w;
		samplerCI.unnormalizedCoordinates = VK_FALSE;
		samplerCI.borderColor= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCI.anisotropyEnable = createInfo.enableAnisotropy;
		if (createInfo.enableAnisotropy) {
			float maxAniso = queryMaxAnisotropy();
			ASSERT(
				createInfo.anisotropy < maxAniso,
				"Assertion failed: required anisotropy exceeds the value supported by device!"
			);
		}
		samplerCI.compareEnable = createInfo.enableCompare;
		samplerCI.compareOp = createInfo.compareOp;
		samplerCI.minLod = createInfo.minLod;
		samplerCI.maxLod = createInfo.maxLod;
		samplerCI.mipLodBias = createInfo.lodBias;
		ASSERT(
			vkCreateSampler(device, &samplerCI, nullptr, &sampler) == VK_SUCCESS,
			"Assertion failed: sampler creation failed!"
		);
		return sampler;
	}

	float GPUDevice::queryMaxAnisotropy() {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		return properties.limits.maxSamplerAnisotropy;
	}

	TextureHandle GPUDevice::createTexture(const TextureCreation createInfo) {
		// Require a resource first
		TextureHandle handle = requireTexture();
		Texture& texture = getTexture(handle);
		
		// Fill in the structure 
		texture.format = util_getFormat(createInfo.format);
		texture.sampler = VK_NULL_HANDLE;
		for (u16 i = 0; i < createInfo.nMipLevels; i++) {
			texture.access[i] = GraphResourceAccessType::UNDEFINED;    //initialized to be undefined
		}

		// Create image
		VkImageCreateInfo imageCI;
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.arrayLayers = 1;
		imageCI.format = util_getFormat(createInfo.format);
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.mipLevels = createInfo.nMipLevels;
		imageCI.extent.width = createInfo.width;
		imageCI.extent.height = createInfo.height;
		imageCI.extent.depth = createInfo.depth;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.imageType = util_getImageType(createInfo.type);
		imageCI.flags = createInfo.flags;

		// set image usage
		imageCI.usage = 0;
		imageCI.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // Default usage since we only create textures for output of render passes
		imageCI.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;             // Default to be readable
		if (createInfo.resourceType == GraphResourceType::DEPTH_MAP) {
			imageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}

		//// Allocate memory on device
		//VmaAllocationCreateInfo allocInfo{};
		//allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		//if (createInfo.aliasTexture == INVALID_TEXTURE_HANDLE) {
		//	ASSERT(
		//		vmaCreateImage(vmaAllocator, &imageCI, &allocInfo, &texture.image, &texture.vmaAlloc, nullptr) == VK_SUCCESS,
		//		"Assertion failed: VMA create image failed!"
		//	);
		//}
		//else {
		//	Texture& aliasTex = getTexture(createInfo.aliasTexture);
		//	ASSERT(
		//		vmaCreateAliasingImage(vmaAllocator, aliasTex.vmaAlloc,&imageCI,&texture.image) == VK_SUCCESS,
		//		"Assertion failed: VMA create image failed!"
		//	);
		//}

		ASSERT(
			vkCreateImage(device, &imageCI, nullptr, &texture.image) == VK_SUCCESS,
			"Assertion failed: create Image failed!"
		);

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, texture.image, &memoryRequirements);

		uint32_t memoryTypeIndex = helper_findSuitableMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		allocInfo.allocationSize = memoryRequirements.size;
		if (VK_SUCCESS != vkAllocateMemory(device, &allocInfo, nullptr, &texture.mem)) {
			throw std::runtime_error("failed to allocate memory for image!");
		}

		if (VK_SUCCESS != vkBindImageMemory(device, texture.image, texture.mem, 0)) {
			throw std::runtime_error("failed to bind image memory!");
		}
		
		// Create imageView
		VkImageViewCreateInfo viewCI{};
		viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCI.image = texture.image;
		viewCI.format = util_getFormat(createInfo.format);
		viewCI.subresourceRange.layerCount = 1;
		viewCI.subresourceRange.baseArrayLayer = 0;
		viewCI.subresourceRange.baseMipLevel = createInfo.baseMipLevel;
		viewCI.subresourceRange.levelCount = createInfo.nMipLevels;
		viewCI.subresourceRange.aspectMask = createInfo.resourceType == (GraphResourceType::DEPTH_MAP) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewCI.viewType = util_getImageViewType(createInfo.type);
		ASSERT(
			vkCreateImageView(device, &viewCI, nullptr, &texture.imageView) == VK_SUCCESS,
			"Assertion failed: create image view faield!"
		);

		// TODO: create sampler for texture (study details before this) 
		SamplerCreation samplerCI{};
		texture.sampler = createSampler(samplerCI);

		return handle;
	}

	// TODO: allocate mem using VMA
	BufferHandle GPUDevice::createBuffer(const BufferCreation createInfo) {
		// Require a resource first
		BufferHandle handle = requireTexture();
		Buffer& buffer = getBuffer(handle);

		//create the buffer object
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = createInfo.size;
		bufferInfo.usage = createInfo.usage;
		bufferInfo.sharingMode = util_getSharingMode(createInfo.shareMode);
		if (VK_SUCCESS != vkCreateBuffer(device, &bufferInfo, nullptr, &buffer.buffer)) {
			throw std::runtime_error("failed to create buffer!");
		}

		//query required memory type for the buffer
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, buffer.buffer, &memoryRequirements);

		//find suitable memory type for the buffer
		uint32_t memoryTypeIndex = helper_findSuitableMemoryType(memoryRequirements.memoryTypeBits, createInfo.prop);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		allocInfo.allocationSize = memoryRequirements.size;
		if (VK_SUCCESS != vkAllocateMemory(device, &allocInfo, nullptr, &buffer.mem)) {
			throw std::runtime_error("failed to allocate memory for buffer!");
		}

		//bind buffer with memory
		if (VK_SUCCESS != vkBindBufferMemory(device, buffer.buffer, buffer.mem, 0)) {
			throw std::runtime_error("failed to bind buffer memory!");
		}

		return handle;
	}

	DescriptorSetLayoutsHandle GPUDevice::createDescriptorSetLayouts(const DescriptorSetLayoutsCreation createInfo) {
		// Require a resource first
		DescriptorSetLayoutsHandle handle = requireDescriptorSetLayouts();
		std::vector<VkDescriptorSetLayout>& layouts = getDescriptorSetLayouts(handle);
		
		// Count for the required number of descriptor sets
		sizet maxSet = 0;
		for (u32 i = 0; i < createInfo.bindings.size(); i++) {
			const BindingDesc& binding = createInfo.bindings.at(i);
			maxSet = binding.groupId > maxSet ? binding.groupId : maxSet;
		}

		layouts.resize(maxSet + 1);

		// Group bindings into sets
		std::vector<std::vector<BindingDesc>> groupBindingDescs;
		groupBindingDescs.resize(maxSet+1);
		for (u32 i = 0; i < createInfo.bindings.size(); i++) {
			groupBindingDescs[createInfo.bindings.at(i).groupId].push_back(createInfo.bindings.at(i));
		}

		// Create descriptorSetLayout for each set
		for (u32 set = 0; set < layouts.size(); set++) {
			const sizet setSize = groupBindingDescs.at(set).size();
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			bindings.resize(setSize);
			std::vector<BindingDesc>& descs = groupBindingDescs.at(set);
			for (u32 i = 0; i < descs.size(); i++) {
				auto desc = descs.at(i);
				VkDescriptorSetLayoutBinding layoutBinding{};
				layoutBinding.binding = desc.binding;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = util_getShaderStageFlags(desc.accessStage);
				layoutBinding.descriptorType = util_getDescriptorType(desc.type);
				bindings.push_back(layoutBinding);
			}
			VkDescriptorSetLayoutCreateInfo layoutCI{};
			layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutCI.bindingCount = static_cast<u32>(bindings.size());
			layoutCI.pBindings = bindings.data();
			ASSERT(
				vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &layouts[set]) == VK_SUCCESS,
				"Assertion failed: create DescriptorSetLayout failed!"
			);
		}
		return handle;
	}

	DescriptorSetsHandle GPUDevice::createDescriptorSets(DescriptorSetsAlloc allocInfo) {
		DescriptorSetsHandle handle = requireDescriptorSets();
		std::vector<VkDescriptorSet>& descriptorSets = getDescriptorSets(handle);
		std::vector<VkDescriptorSetLayout>& layouts = getDescriptorSetLayouts(allocInfo.layoutsHandle);
		descriptorSets.resize(layouts.size());

		for (u32 i = 0; i < layouts.size();i++) {
			VkDescriptorSetAllocateInfo alloc{};
			alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc.descriptorPool = descriptorPool;
			alloc.descriptorSetCount = 1;
			alloc.pSetLayouts = &layouts.at(i);
			ASSERT(
				vkAllocateDescriptorSets(device, &alloc, &descriptorSets.at(i)) == VK_SUCCESS,
				"Assertion failed: Allocate DescriptorSets failed!"
			);
		}
		return handle;
	}

	void GPUDevice::writeDescriptorSets(const std::vector<DescriptorSetWrite>& writes, DescriptorSetsHandle setsHandle) {
		std::vector<VkDescriptorSet>& sets = getDescriptorSets(setsHandle);

		std::vector<VkWriteDescriptorSet>updates;
		updates.resize(writes.size());
		for (u32 i = 0; i < writes.size(); i++) {
			const DescriptorSetWrite& write = writes.at(i);
			VkWriteDescriptorSet& update = updates.at(i);
			update.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			update.descriptorCount = 1;
			update.dstBinding = write.dstBinding;
			update.dstSet = sets.at(write.dstSetId);
			if (write.type == BindingType::IMAGE_SAMPLER) {
				Texture& texture = getTexture(write.resource.texHandle);
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.sampler = texture.sampler;
				imageInfo.imageView = texture.imageView;
				update.pImageInfo = &imageInfo;
			}
			else {
				Buffer& buffer = getBuffer(write.resource.bufferhandle);
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = buffer.buffer;
				bufferInfo.offset = 0;                           //´æÒÉ
				bufferInfo.range = buffer.size;
				update.pBufferInfo = &bufferInfo;
			}
		}
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(updates.size()), updates.data(), 0, nullptr);
	}

	RenderPassHandle GPUDevice::createRenderPass(const RenderPassCreation createInfo) {
		RenderPassHandle handle = requireRenderPass();
		VkRenderPass& renderPass = getRenderPass(handle);

		const std::vector<RenderAttachmentInfo>& attachmentInfos = createInfo.attachmentInfos;

		std::vector<VkAttachmentDescription> attachDescs;
		attachDescs.resize(attachmentInfos.size());

		std::vector<VkAttachmentReference> colorAttachRefs;
		VkAttachmentReference depthRef{};
		
		// create attachmentDescs
		u32 depthMapIndex = -1;
		for (u32 i = 0; i < attachDescs.size(); i++) {
			VkAttachmentDescription& desc = attachDescs.at(i);
			auto attachInfo = createInfo.attachmentInfos.at(i);

			desc.format = util_getFormat(attachInfo.format);
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			if (attachInfo.resourceType == GraphResourceType::TEXTURE) {
				desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

				VkAttachmentReference colorRef{};
				colorRef.attachment = static_cast<uint32_t>(i);
				colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachRefs.push_back(colorRef);
			}
			else if (attachInfo.resourceType == GraphResourceType::DEPTH_MAP) {
				desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;;
				desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				depthMapIndex = i;

				depthRef.attachment= static_cast<uint32_t>(i);
				depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
			}
		}

		// create subpass
		VkSubpassDescription subpassDesc;
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = static_cast<uint32_t>(colorAttachRefs.size());
		subpassDesc.pColorAttachments = colorAttachRefs.data();
		subpassDesc.pDepthStencilAttachment = &depthRef;

		// dependency between subpasses
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		if (depthMapIndex != -1) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		// finally create the renderpass
		VkRenderPassCreateInfo passCI{};
		passCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passCI.attachmentCount = static_cast<uint32_t>(attachDescs.size());
		passCI.pAttachments = attachDescs.data();
		passCI.subpassCount = 1;
		passCI.pSubpasses = &subpassDesc;
		passCI.dependencyCount = 1;
		passCI.pDependencies = &dependency;
		
		ASSERT(
			vkCreateRenderPass(device, &passCI, nullptr, &renderPass) == VK_SUCCESS,
			"Assertion failed: CreateRenderPass failed!"
		);
		return handle;
	}

	// TODO: add support for push constants
	PipelineLayoutHandle GPUDevice::createPipelineLayout(const PipelineLayoutCreation createInfo) {
		PipelineLayoutHandle handle = requirePipelineLayout();
		VkPipelineLayout& pipelineLayout = getPipelineLayout(handle);

		std::vector<VkDescriptorSetLayout> combinedLayouts;
		for (auto handle : createInfo.descLayoutsHandles) {
			std::vector<VkDescriptorSetLayout>& layouts = getDescriptorSetLayouts(handle);
			combinedLayouts.insert(combinedLayouts.end(), layouts.begin(), layouts.end());
		}
		VkPipelineLayoutCreateInfo layoutCI{};
		layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCI.pushConstantRangeCount = 0;                     // by default no push constants
		layoutCI.pPushConstantRanges = VK_NULL_HANDLE;
		layoutCI.setLayoutCount = static_cast<uint32_t>(combinedLayouts.size());
		layoutCI.pSetLayouts = combinedLayouts.data();
		
		ASSERT(
			vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout) == VK_SUCCESS,
			"Assertion failed: CreatePipelineLayout failed!"
		);
		return handle;
	}

	// TODO: add dynamic states support
	// TODO: add support for pipelineCache
	GraphicsPipelineHandle GPUDevice::createGraphicsPipeline(const GraphicsPipelineCreation createInfo) {
		GraphicsPipelineHandle handle = requireGraphicsPipeline();
		VkPipeline& pipeline = getGraphicsPipeline(handle);

		// FOR SHADERS
		auto shaderInfo = createInfo.shaders;
		auto vertShaderCode = fileHandler.read(shaderInfo.vertShaderPath);
		auto fragShaderCode = fileHandler.read(shaderInfo.fragShaderPath);

		VkShaderModule vertShaderModule = helper_createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = helper_createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// FOR VERTEX INPUT
		auto vertexInput = createInfo.vertexInput;
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = static_cast<uint32_t>(vertexInput.bindingDesc.binding);
		bindingDesc.stride = static_cast<uint32_t>(vertexInput.bindingDesc.stride);
		bindingDesc.inputRate = util_getVertexInputRate(vertexInput.bindingDesc.inputRate);

		std::vector<VkVertexInputAttributeDescription> attributes;
		attributes.resize(vertexInput.attributes.size());
		for (u32 i = 0; i < attributes.size(); i++) {
			VkVertexInputAttributeDescription& attr = attributes.at(i);
			auto desc = vertexInput.attributes.at(i);
			attr.binding = static_cast<uint32_t>(desc.binding);
			attr.location = static_cast<uint32_t>(desc.location);
			attr.offset = static_cast<uint32_t>(desc.offset);
			attr.format = util_getFormat(desc.format);
		}
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

		// FOR INPUT ASSEMBLY
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;       // only triangle permitted
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// FOR VIEWPORTS AND SCISSORS
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainWidth * 1.0);
		viewport.height = static_cast<float>(swapChainHeight * 1.0);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { swapChainWidth, swapChainWidth };

		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
	            VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		// FOR RASTERIZAER
		auto rasterInfo = createInfo.rasterInfo;
		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = util_getCullMode(rasterInfo.cullMode);
		rasterizer.frontFace = util_getFrontFace(rasterInfo.frontFace);
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; 
		rasterizer.depthBiasClamp = 0.0f; 
		rasterizer.depthBiasSlopeFactor = 0.0f; 

		// TODO: add support for MSAA
		VkPipelineMultisampleStateCreateInfo msaaInfo = {};
		msaaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		msaaInfo.sampleShadingEnable = VK_FALSE;
		msaaInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		msaaInfo.minSampleShading = 1.0f; 
		msaaInfo.pSampleMask = nullptr; 
		msaaInfo.alphaToCoverageEnable = VK_FALSE; 
		msaaInfo.alphaToOneEnable = VK_FALSE; 

		// FOR DEPTHSTENCIL
		// stencil is by default disabled
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		if (createInfo.depthStencil.enableDepth) {
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds = 0.0f; // Optional
			depthStencil.maxDepthBounds = 1.0f; // Optional
			depthStencil.stencilTestEnable = VK_FALSE;
			depthStencil.front = {}; // Optional
			depthStencil.back = {}; // Optional
		}

		// FOR COLOR BLENDING (currently disabled)
		// TODO: add color blending support
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
		colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingInfo.logicOpEnable = VK_FALSE;
		colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY; 
		colorBlendingInfo.attachmentCount = 1;
		colorBlendingInfo.pAttachments = &colorBlendAttachment;
		colorBlendingInfo.blendConstants[0] = 0.0f; // Optional
		colorBlendingInfo.blendConstants[1] = 0.0f; // Optional
		colorBlendingInfo.blendConstants[2] = 0.0f; // Optional
		colorBlendingInfo.blendConstants[3] = 0.0f; // Optional

		// Finally create it 
		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shaderStages;
		pipelineCI.pVertexInputState = &vertexInputInfo;
		pipelineCI.pViewportState = &viewportState;
		pipelineCI.pDynamicState = &dynamicStateInfo;
		pipelineCI.pInputAssemblyState = &inputAssembly;
		pipelineCI.pRasterizationState = &rasterizer;
		pipelineCI.pDepthStencilState = &depthStencil;
		pipelineCI.pColorBlendState = &colorBlendingInfo;
		pipelineCI.pMultisampleState = &msaaInfo;
		pipelineCI.renderPass = getRenderPass(createInfo.renderPassInfo.renderPassHandle);
		pipelineCI.subpass = 0;
		pipelineCI.layout = getPipelineLayout(createInfo.pipelineLayoutHandle);
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCI.basePipelineIndex = 0;

		ASSERT(
			vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline) == VK_SUCCESS,
			"Assertion failed: CreateGraphicsPipeline failed!"
		);

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		return handle;
	}

	FramebufferHandle GPUDevice::createFramebuffer(const FramebufferCreation createInfo) {
		FramebufferHandle handle = requireFramebuffer();
		VkFramebuffer& framebuffer = getFramebuffer(handle);

		std::vector<VkImageView> attachments;
		attachments.resize(createInfo.attachments.size());
		for (u32 i = 0; i < createInfo.attachments.size(); i++) {
			TextureHandle texHandle = createInfo.attachments.at(i);
			Texture& tex = getTexture(texHandle);
			attachments[i] = tex.imageView;
		}

		VkFramebufferCreateInfo fbCI{};
		fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCI.width = createInfo.width;
		fbCI.height = createInfo.height;
		fbCI.layers = createInfo.layers;
		fbCI.renderPass = getRenderPass(createInfo.renderPassHandle);
		fbCI.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbCI.pAttachments = attachments.data();

		ASSERT(
			vkCreateFramebuffer(device, &fbCI, nullptr, &framebuffer) == VK_SUCCESS,
			"Assertion failed: CreateFramebuffer failed!"
		);

		return handle;
	}

	template<typename T>
	BufferHandle& GPUDevice::createBufferFromData(const std::vector<T>& data) {

		VkDeviceSize bufferSize = VkDeviceSize(sizeof(T) * data.size());

		//create vertex buffer (inaccessable by host, exclusive by GPU)
		BufferCreation mainCI{};
		mainCI.setSize(bufferSize)
			.setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
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
		void* pdata;
		vkMapMemory(device, stageBuffer.mem, 0, bufferSize, 0, &data);
		memcpy(pdata, data.data(), bufferSize);
		vkUnmapMemory(device, stageBuffer.mem);

		//transfer the vertices data from staging buffer to vertex buffer (by sumbitting commands)
		transferBufferInDevice(stageBuffer, mainBuffer, bufferSize);

		//release the stagging buffer and free the memory
		removeBuffer(stage);

		return main;
	}

	template<typename T>
	TextureHandle& GPUDevice::createTexture2DFromData(const std::vector<T>& data, u16 width, u16 height, u16 nMips, DataFormat format) {
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
		void* pdata;
		vkMapMemory(device, stageBuffer.mem, 0, bufferSize, 0, &data);
		memcpy(pdata, data.data(), bufferSize);
		vkUnmapMemory(device, stageBuffer.mem);

		// create the image
		TextureCreation texCI{};
		texCI.setType(TextureType::Texture2D)
			.setFormat(format)
			.setSize(width, height, 1)
			.setMipLevels(nMips);
		TextureHandle texHandle = createTexture(texCI);
		Texture& tex = getTexture(texHandle);

		// transfer data in device
		transferBufferToImage2DInDevice(stage, tex, width, height);

		// release the staging buffer
		removeBuffer(stage);

		// generate mip levels if requried
		if (nMips > 1) {
			// set layout transition for all mip levels
			// so that the i-th mip level will be transitioned to the layout COPY_DST when performing the copy from i-1 to i
			imageLayoutTransition(texHandle, GraphResourceAccessType::COPY_DST, 0, nMips);
			auxiCmdBuffer.begin();
			helper_generateMipMaps(texHandle, nMips);
			auxiCmdBuffer.end();
		}

		// since the texture will be read by shader during rendering
		// insert barrier to transition it to appropriate layout
		imageLayoutTransition(texHandle);
	}

	void GPUDevice::transferBufferInDevice(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize copySize) {
		auxiCmdBuffer.begin();
		auxiCmdBuffer.cmdCopyBuffer(srcBuffer, dstBuffer, copySize);
		auxiCmdBuffer.end();

		// submit to queue
		submitCmds(mainQueue, auxiCmdBuffer.getCmdBuffer());
	}

	// NOTE: this function operates on only the first mip level of both images
	void GPUDevice::transferImageInDevice(TextureHandle src, TextureHandle dst, VkExtent2D copyExtent) {
		auxiCmdBuffer.begin();

		// get the textures 
		Texture& srcImage = getTexture(src);
		Texture& dstImage = getTexture(dst);
		GraphResourceAccessType srcAccess = srcImage.access[0];
		GraphResourceAccessType dstAccess = dstImage.access[0];

		// layout transition before copy 
		imageLayoutTransition(src, GraphResourceAccessType::COPY_SRC, 0, 1);
		imageLayoutTransition(dst, GraphResourceAccessType::COPY_DST, 0, 1);

		// copy
		auxiCmdBuffer.cmdCopyImage(srcImage, dstImage, copyExtent);

		auxiCmdBuffer.end();

		// submit to queue
		submitCmds(mainQueue, auxiCmdBuffer.getCmdBuffer());

	}

	// NOTE: this func only transfer buffer data to the first mip level of texture
	void GPUDevice::transferBufferToImage2DInDevice(BufferHandle bHnd, TextureHandle texHnd, u32 width, u32 height) {
		auxiCmdBuffer.begin();
		Buffer& buffer = getBuffer(bHnd);
		Texture& image = getTexture(texHnd);

		// layout transition before transferring
		//auxiCmdBuffer.cmdInsertImageBarrier(image, GraphResourceAccessType::COPY_DST, 0, 1);
		imageLayoutTransition(texHnd, GraphResourceAccessType::COPY_DST, 0, 1);
		
		// transfer data
		auxiCmdBuffer.cmdCopyBufferToImage(buffer, image, width, height, 1);

		auxiCmdBuffer.end();

		// submit to queue
		submitCmds(mainQueue, auxiCmdBuffer.getCmdBuffer());

	}

	// transition the image layout
	// tex determines the the oldLayout
	// targetAccess determines the newLayout
	void GPUDevice::imageLayoutTransition(TextureHandle texHandle, GraphResourceAccessType targetAccess, u16 baseMip, u16 nMips) {
		auxiCmdBuffer.begin();

		Texture& tex = getTexture(texHandle);
		// insert barrier for each mip level separately 
		// also update the texture state for tracking
		for (u16 i = baseMip; i < baseMip + nMips; i++) {
			auxiCmdBuffer.cmdInsertImageBarrier(tex, targetAccess, baseMip);
			tex.access[i] = targetAccess;
		}

		auxiCmdBuffer.end();

		// submit to queue
		submitCmds(mainQueue, auxiCmdBuffer.getCmdBuffer());

	}

	// TODO: this should be a member function of class Queue, wrapping the VkQueue
	void GPUDevice::submitCmds(VkQueue queue, std::vector<VkCommandBuffer> cmdBuffesToSubmit, std::vector<VkSemaphore> waitSemas, std::vector<VkPipelineStageFlags> waitStages, std::vector<VkSemaphore> signalSemas, VkFence fence) {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemas.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemas.data();
		submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffesToSubmit.size());
		submitInfo.pCommandBuffers = cmdBuffesToSubmit.data();

		ASSERT(
			vkQueueSubmit(queue, 1, &submitInfo, fence) == VK_SUCCESS,
			"Assertion failed: QueueSubmit failed!"
		);
	}

	void GPUDevice::submitCmds(VkQueue queue, VkCommandBuffer cmdBuffer) {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		ASSERT(
			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
			"Assertion failed: QueueSubmit failed!"
		);
	}

	// helper: check whether the physical device support all required types of queues
	// return true if all required types of queues satisfied and that the device supports presentation
	// TODO: more types of queue support
	bool GPUDevice::helper_checkQueueSatisfication(VkPhysicalDevice phyDevice, u32 requiredQueues) {
		u32 propertyCnt;
		vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &propertyCnt, nullptr);
		std::vector<VkQueueFamilyProperties> properties(propertyCnt);
		vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &propertyCnt, properties.data());

		bool graphics = false;
		bool compute = false;
		bool transfer = false;
		VkBool32 supportSurface = false;
		for (u32 i = 0; i < propertyCnt; i++) {
			auto& prop = properties.at(i);
			if (prop.queueCount>0) {
				vkGetPhysicalDeviceSurfaceSupportKHR(phyDevice, i, windowSurface, &supportSurface);
			}
			if ((prop.queueCount > 0) && (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT))
				graphics = true;
			if ((prop.queueCount > 0) && (prop.queueFlags & VK_QUEUE_COMPUTE_BIT))
				compute = true;
			if ((prop.queueCount > 0) && (prop.queueFlags & VK_QUEUE_TRANSFER_BIT))
				transfer = true;
		}

		if((requiredQueues & VK_QUEUE_GRAPHICS_BIT) && (!graphics))
			return false;
		if ((requiredQueues & VK_QUEUE_COMPUTE_BIT) && (!compute))
			return false;
		if ((requiredQueues & VK_QUEUE_TRANSFER_BIT) && (!transfer))
			return false;
		if (!supportSurface)
			return false;

		return true;
	}

	bool GPUDevice::helper_checkExtensionSupport(VkPhysicalDevice phyDevice, const std::vector<const char*>& requiredExtensions) {
		uint32_t extensionsCnt;
		vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &extensionsCnt, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionsCnt);
		vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &extensionsCnt, availableExtensions.data());

		for (const char* extension : requiredExtensions) {
			bool found = false;
			for (const auto& availableExtension : availableExtensions) {
				if (strcmp(extension, availableExtension.extensionName)) {
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}
		return true;
	}

	bool GPUDevice::helper_checkInstanceLayerSupport(const std::vector<const char*>& requiredLayers) {
		uint32_t layerCnt;
		vkEnumerateInstanceLayerProperties(&layerCnt, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCnt);
		vkEnumerateInstanceLayerProperties(&layerCnt, availableLayers.data());

		for (const char* layer : requiredLayers) {
			bool found = false;
			for (const auto& availableLayer : availableLayers) {
				if (strcmp(layer, availableLayer.layerName)) {
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}
		return true;
	}

	SwapChainSupportDetails GPUDevice::helper_querySwapChainSupport(VkPhysicalDevice phyDevice) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, windowSurface, &details.capabilities);

		uint32_t formatsCnt;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, windowSurface, &formatsCnt,nullptr);
		details.formats.resize(formatsCnt);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, windowSurface, &formatsCnt, details.formats.data());

		uint32_t presentModesCnt;
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, windowSurface, &presentModesCnt, nullptr);
		details.presentModes.resize(presentModesCnt);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, windowSurface, &presentModesCnt,details.presentModes.data());

		return details;
	}

	VkSurfaceFormatKHR GPUDevice::helper_selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR GPUDevice::helper_selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) {
		for (const auto& mode : availableModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D GPUDevice::helper_selectSwapExtent(const VkSurfaceCapabilitiesKHR capabilities) {
		//if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		//	return capabilities.currentExtent;
		//}
		if (capabilities.currentExtent.width != 3) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	// helper: get indices of required queue families
	// by default to have only one queue for each required family
	// for now simply pick the first queue for each type that satisfies
	// TODO: may extend to support multiple queues per family
	// TODO: a better strategy to select queues
	// TODO: for transfer queue
	QueueFamilyInfos GPUDevice::helper_selectQueueFamilies(VkPhysicalDevice phyDevice, u32 requiredQueues) {
		QueueFamilyInfos familyInfos{};

		u32 propertyCnt;
		vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &propertyCnt, nullptr);
		std::vector<VkQueueFamilyProperties> properties(propertyCnt);
		vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &propertyCnt, properties.data());

		// select for present queue (this is always required by the application)
		for (u32 i = 0; i < propertyCnt; i++) {
			auto& prop = properties.at(i);
			if (prop.queueCount > 0) {
				VkBool32 supportSurface = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(phyDevice, i, windowSurface, &supportSurface);
				if (supportSurface) {
					familyInfos.presentQueue.familyIndex = i;
					familyInfos.presentQueue.queueIndex = 0;
					break;
				}
			}
		}

		// select for graphics queue
		if (requiredQueues & VK_QUEUE_GRAPHICS_BIT) {
			for (u32 i = 0; i < propertyCnt; i++) {
				auto& prop = properties.at(i);
				if ((prop.queueCount > 0) && (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
					familyInfos.mainQueue.familyIndex = i;
					familyInfos.mainQueue.queueIndex = 0;
					break;
				}
			}
		}

		// select for compute queue
		if (requiredQueues & VK_QUEUE_COMPUTE_BIT) {
			for (u32 i = 0; i < propertyCnt; i++) {
				auto& prop = properties.at(i);
				if ((prop.queueCount > 0) && (prop.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
					familyInfos.computeQueue.familyIndex = i;
					familyInfos.computeQueue.queueIndex = 0;
					break;
				}
			}
		} 

		// try to merge graphics and compute in a single family
		if (requiredQueues & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
			auto& graphicsProp = properties.at(familyInfos.mainQueue.familyIndex);
			if (graphicsProp.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				familyInfos.computeQueue.familyIndex = familyInfos.mainQueue.familyIndex;
				familyInfos.computeQueue.queueIndex = graphicsProp.queueCount > 1 ? 1 : 0;
			}
		}
		return familyInfos;
	}

	//TODO: priorities for queue
	std::vector<VkDeviceQueueCreateInfo>& GPUDevice::helper_getQueueCreateInfos(QueueFamilyInfos queueInfos, u32 requiredQueues) {
		std::vector<VkDeviceQueueCreateInfo> CIs;

		VkDeviceQueueCreateInfo CI{};
		CI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		CI.queueCount = 1;
		float prior = 1.0;
		CI.pQueuePriorities = &prior;

		if (requiredQueues & VK_QUEUE_GRAPHICS_BIT) {
			CI.queueFamilyIndex = queueInfos.mainQueue.familyIndex;
			CIs.push_back(CI);
		}

		if (requiredQueues & VK_QUEUE_COMPUTE_BIT) {
			CI.queueFamilyIndex = queueInfos.computeQueue.familyIndex;
			CIs.push_back(CI);
		}
		if (requiredQueues & VK_QUEUE_TRANSFER_BIT) {
			CI.queueFamilyIndex = queueInfos.transferQueue.familyIndex;
			CIs.push_back(CI);
		}

		return CIs;
	}

	// TODO: handle all types of shader extensions 
	// Study Panko for details (in ShaderManager::get_spirv)
	VkShaderModule GPUDevice::helper_createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		ASSERT(
			vkCreateShaderModule(device,&createInfo,nullptr,&shaderModule)==VK_SUCCESS,
			"Assertion failed: CreateShaderModule failed!"
		);

		return shaderModule;
	}

	uint32_t GPUDevice::helper_findSuitableMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties deviceMemoryproperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryproperties);
		for (size_t i = 0; i < deviceMemoryproperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && ((deviceMemoryproperties.memoryTypes[i].propertyFlags & properties) == properties)) {
				return i;
			}
		}
		throw std::runtime_error("failed to find a suitable memory type!");
	}

	void GPUDevice::helper_generateMipMaps(TextureHandle texHandle, u16 nMips) {
		Texture& tex = getTexture(texHandle);
		u16 mipWidth = tex.width;
		u16 mipHeight = tex.height;

		for (u16 i = 1; i < nMips; i++) {
			// transit the previous miplevel layout for copy
			//auxiCmdBuffer.cmdInsertImageBarrier(tex, GraphResourceAccessType::COPY_SRC, i - 1, 1);
			imageLayoutTransition(texHandle, GraphResourceAccessType::COPY_SRC, i - 1, 1);
			
			// copy data from mip level i-1 to i
			VkOffset3D srcRegion = { mipWidth,mipHeight,1 };
			VkOffset3D dstRegion = { mipWidth>1? mipWidth/2:1, mipHeight > 1 ? mipHeight / 2 : 1,1 };
			auxiCmdBuffer.cmdBlitImage(tex, tex, i - 1, i, srcRegion, dstRegion);
		}
	}

	std::vector<const char*> GPUDevice::helper_getRequiredInstanceExtensions(bool enableValidation) {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidation) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
 }