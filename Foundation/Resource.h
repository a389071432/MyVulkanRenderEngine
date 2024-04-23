#pragma once
#include <stdint.h>
#include<vector>
#include<queue>
#include"vulkan/vulkan_core.h"
#include"vma/vk_mem_alloc.h"
#include"enums.h"
#include"datatypes.h"

namespace zzcVulkanRenderEngine {
	typedef u32 TextureHandle;
	typedef u32 BufferHandle;
	typedef u32 DescriptorSetLayoutsHandle;      // points to a list of layouts
	typedef u32 DescriptorSetsHandle;            // points to a list of sets
	typedef u32 RenderPassHandle;
	typedef u32 FramebufferHandle;


	const TextureHandle INVALID_TEXTURE_HANDLE = -1;
	const BufferHandle INVALID_TEXTURE_HANDLE = -1;
	const DescriptorSetsHandle INVALID_DESCRIPTORSETS_HANDLE = -1;

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
		TextureHandle aliasTexture = INVALID_TEXTURE_HANDLE;

		TextureCreation& setSize(u16 width, u16 height, u16 depth);
		TextureCreation& setFlags(u16 nMipLevels, u16 flags);
		TextureCreation& setFormat(VkFormat format);
		TextureCreation& setType(TextureType type);
		TextureCreation& setAliasTexture(TextureHandle aliasTex);
	};

	// TODO: fill in this
	struct BufferCreation {

	};

	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory mem;
		VkDeviceSize size;
		Buffer() {};
	};

	struct BindingDesc {
		BindingType type;
		ShaderStage accessStage;
		u16 groupId;
		u32 binding;
	};

	struct DescriptorSetLayoutsCreation {
		std::vector<BindingDesc> bindings;
		GraphNodeType nodeType;

		DescriptorSetLayoutsCreation& setNodeType(GraphNodeType nodeType);
		DescriptorSetLayoutsCreation& addBinding(BindingDesc binding);
	};

	struct DescriptorSetsAlloc {
		DescriptorSetLayoutsHandle layoutsHandle;
	};

	struct DescriptorSetWrite {
		// resource to bind
		BindingType type;
		union {
			TextureHandle texHandle;
			BufferHandle bufferhandle;
		}resource;

		// position to bind
		u16 dstSetId;
		u16 dstBinding;
	};

	template<typename T, typename ResourceHandle>
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
