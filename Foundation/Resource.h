#pragma once
#include <stdint.h>
#include<vector>
#include<queue>
#include"vulkan/vulkan_core.h"
#include"vma/vk_mem_alloc.h"
#include"enums.h"
#include"datatypes.h"

namespace zzcVulkanRenderEngine {
	typedef uint32_t ResourceHandle;

	struct Texture {
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory mem;
		VkFormat format;
		VkSampler sampler;
		GraphResourceAccessType access;

		VmaAllocation vmaAlloc;    // record allocation info on device, useful for aliasing memory allocation

		Texture() {};
		void setAccessType(GraphResourceAccessType access);
	};

	const ResourceHandle INVALID_TEXTURE_HANDLE = -1;
	struct TextureCreation {
		u16 width;
		u16 height;
		u16 depth;
		u16 baseMipLevel = 0;
		u16 nMipLevels = 1;
		u16 flags;
		GraphResourceType resourceType;
		VkFormat format = VK_FORMAT_UNDEFINED;
		TextureType  type = TextureType::Texture2D;
		ResourceHandle aliasTexture = INVALID_TEXTURE_HANDLE;

		TextureCreation& set_size(u16 width, u16 height, u16 depth);
		TextureCreation& set_flags(u16 nMipLevels, u16 flags);
		TextureCreation& set_format(VkFormat format);
		TextureCreation& set_type(TextureType type);
		TextureCreation& set_aliasTexture(ResourceHandle aliasTex);
	};

	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory mem;
		VkDeviceSize size;
		Buffer() {};
	};

	template<typename T>
	class ResourcePool {
	public:
		ResourcePool(u32 poolSize);
		~ResourcePool();
		ResourceHandle require_resource();
		T& get_resource(ResourceHandle handle);
		void release_resource(ResourceHandle handle);
	private:
		u32 poolSize;
		const maxPoolSize;
		std::vector<T> data;
		std::queue<T> freeList;
	};
}
