#include"GPUDevice.h"
#include"assert.h"
#include"utils/utils.h"
#include<array>

namespace zzcVulkanRenderEngine {
	GPUDevice::GPUDevice(GPUDeviceCreation createInfo) :texturePool(defaultPoolSize), bufferPool(defaultPoolSize) {

		// TODO: fill in createInfos

		cmdBuffers.resize(maxFrameInFlight);

		// TODO: Create vulkan instance (study details before doing this)

		// TODO: Create physical device (study details before doing this)

		// TODO: Create logical device (study details before doing this)

		// TODO: Create descriptorPool

		// TODO: Create commandBuffers

		// Create VMA allocator
		VmaAllocatorCreateInfo allocatorCI = {};
		allocatorCI.physicalDevice = physicalDevice;
		allocatorCI.device = device;
		allocatorCI.instance = vkInstance;
		ASSERT(
			vmaCreateAllocator(&allocatorCI, &vmaAllocator) == VK_SUCCESS,
			"Assertion failed: VMA Allocator creation failed!"
		);
	}

	GPUDevice::~GPUDevice() {

	}

	TextureHandle GPUDevice::requireTexture() {
		return texturePool.require_resource();
	}

	BufferHandle GPUDevice::requireBuffer() {
		return bufferPool.require_resource();
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

	std::vector<VkDescriptorSet>& GPUDevice::getDescriptorSets(DescriptorSetsHandle handle) {
		return descriptorSetsPool.get_resource(handle);
	}

	TextureHandle GPUDevice::createTexture(const TextureCreation createInfo) {
		// Require a resource first
		TextureHandle handle = requireTexture();
		Texture& texture = getTexture(handle);

		// Fill in the structure 
		texture.format = createInfo.format;
		texture.sampler = VK_NULL_HANDLE;
		texture.access = GraphResourceAccessType::UNDEFINED;    //initialized to be undefined

		// Create image
		VkImageCreateInfo imageCI;
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.arrayLayers = 1;
		imageCI.format = createInfo.format;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.mipLevels = createInfo.nMipLevels;
		imageCI.extent.width = createInfo.width;
		imageCI.extent.height = createInfo.height;
		imageCI.extent.depth = createInfo.depth;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.imageType = util_getImageType(createInfo.type);

		// set image usage
		imageCI.usage = 0;
		imageCI.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // Default usage since we only create textures for output of render passes
		imageCI.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;             // Default to be readable
		if (createInfo.resourceType == GraphResourceType::DEPTH_MAP) {
			imageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}

		// Allocate memory on device
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (createInfo.aliasTexture == INVALID_TEXTURE_HANDLE) {
			ASSERT(
				vmaCreateImage(vmaAllocator, &imageCI, &allocInfo, &texture.image, &texture.vmaAlloc, nullptr) == VK_SUCCESS,
				"Assertion failed: VMA create image failed!"
			);
		}
		else {
			Texture& aliasTex = getTexture(createInfo.aliasTexture);
			ASSERT(
				vmaCreateAliasingImage(vmaAllocator, aliasTex.vmaAlloc,&imageCI,&texture.image) == VK_SUCCESS,
				"Assertion failed: VMA create image failed!"
			);
		}
		
		// Create imageView
		VkImageViewCreateInfo viewCI{};
		viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCI.image = texture.image;
		viewCI.format = createInfo.format;
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

		VkPipelineLayoutCreateInfo layoutCI{};
		layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCI.pushConstantRangeCount = 0;                     // by default no push constants
		layoutCI.pPushConstantRanges = VK_NULL_HANDLE;
		std::vector<VkDescriptorSetLayout>& descLayouts = getDescriptorSetLayouts(createInfo.descLayoutsHandle);
		layoutCI.setLayoutCount = static_cast<uint32_t>(descLayouts.size());
		layoutCI.pSetLayouts = descLayouts.data();
		
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
}