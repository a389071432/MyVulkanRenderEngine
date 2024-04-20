#pragma once
#include <stdint.h>
#include<vector>
#include<queue>
#include"datatypes.h"
#include"vulkan/vulkan_core.h"

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
