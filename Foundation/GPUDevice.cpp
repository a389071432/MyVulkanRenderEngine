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

	TextureHandle GPUDevice::createTexture(TextureCreation createInfo) {
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

	DescriptorSetLayoutsHandle GPUDevice::createDescriptorSetLayouts(DescriptorSetLayoutsCreation createInfo) {
		// Require a resource first
		DescriptorSetLayoutsHandle handle = requireDescriptorSetLayouts();
		std::vector<VkDescriptorSetLayout>& layouts = getDescriptorSetLayouts(handle);
		
		// Count for the required number of descriptor sets
		sizet maxSet = 0;
		for (u32 i = 0; i < createInfo.bindings.size(); i++) {
			BindingDesc& binding = createInfo.bindings.at(i);
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

	void GPUDevice::writeDescriptorSets(std::vector<DescriptorSetWrite>& writes, DescriptorSetsHandle setsHandle) {
		std::vector<VkDescriptorSet>& sets = getDescriptorSets(setsHandle);

		std::vector<VkWriteDescriptorSet>updates;
		updates.resize(writes.size());
		for (u32 i = 0; i < writes.size(); i++) {
			DescriptorSetWrite& write = writes.at(i);
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
}