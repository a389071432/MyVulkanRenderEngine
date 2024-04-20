#pragma once
#include <stdint.h>
#include<vector>
#include<queue>
#include"datatypes.h"
#include"vulkan/vulkan_core.h"
#include"enums.h"

namespace zzcVulkanRenderEngine {
	typedef uint32_t ResourceHandle;

	struct Texture {
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory mem;
		VkFormat format;
		VkSampler sampler;
		Texture() {};
	};

	struct TextureCreation {
		u16 width;
		u16 height;
		u16 depth;
		u16 nMipLevels;
		u16 flags;
		VkFormat format = VK_FORMAT_UNDEFINED;
		TextureType  type = TextureType::Texture2D;

		TextureCreation& set_size(u16 width, u16 height, u16 depth);
		TextureCreation& set_flags(u16 nMipLevels, u16 flags);
		TextureCreation& set_format(VkFormat format);
		TextureCreation& set_type(TextureType type);
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
