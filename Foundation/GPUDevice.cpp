#include"GPUDevice.h"
#include"assert.h"
#include"utils/utils.h"

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

	Texture& GPUDevice::getTexture(TextureHandle handle) {
		return texturePool.get_resource(handle);
	}

	Buffer& GPUDevice::getBuffer(BufferHandle handle) {
		return bufferPool.get_resource(handle);
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
			vkCreateImageView(device, &viewCI, nullptr, &texture.imageView),
			"Assertion failed: create image view faield!"
		);

		return handle;
	}
}