#include"Resource.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {

	//For TextureCreation
	TextureCreation& TextureCreation::setSize(u16 _width, u16 _height, u16 _depth) {
		width = _width;
		height = _height;
		depth = _depth;
		return *this;
	}

	TextureCreation& TextureCreation::setFlags(u16 _nMipLevels, u16 _flags) {
		nMipLevels = _nMipLevels;
		flags = _flags;
		return *this;
	}

	TextureCreation& TextureCreation::setFormat(VkFormat _format) {
		format = _format;
		return *this;
	}

	TextureCreation& TextureCreation::setType(TextureType _type) {
		type = _type;
		return *this;
	}

	TextureCreation& TextureCreation::setAliasTexture(TextureHandle _aliasTex) {
		aliasTexture = _aliasTex;
		return *this;
	}

	//For buffer creation
	BufferCreation& BufferCreation::setSize(u32 _size) {
		size = _size;
	}

	BufferCreation& BufferCreation::setUsage(u32 _usage) {
		usage = _usage;
	}

	BufferCreation& BufferCreation::setProp(u32 _prop) {
		prop = _prop;
	}

	BufferCreation& BufferCreation::setShareMode(ResourceSharingMode _shareMode) {
		shareMode = _shareMode;
	}

	// For Texture
	void Texture::setAccessType(GraphResourceAccessType _access) {
		access = _access;
	}

	// For DescriptorSetsCreation
	DescriptorSetsCreation& DescriptorSetsCreation::setNodeType(GraphNodeType _nodeType) {
		nodeType = _nodeType;
	}

	DescriptorSetsCreation& DescriptorSetsCreation::addInput(BindingDesc binding) {
		inputs.push_back(binding);
	}

	DescriptorSetsCreation& DescriptorSetsCreation::addOutput(BindingDesc binding) {
		outputs.push_back(binding);
	}

	// For ResourcePool
	template <typename T, typename ResourceHandle>
	ResourcePool<T,ResourceHandle>::ResourcePool(u32 poolSize) {
		ASSERT(poolSize <= maxPoolSize, "required pool size exceeds maximum limit!");

		data.reserve(poolSize);
		for (u32 i = 0; i < poolSize; i++) {
			data.push_back(T());
		}

		for (u32 i = 0; i < data.size(); i++) {
			freeList.push(i);
		}
	}

	template <typename T, typename ResourceHandle>
	ResourcePool<T, ResourceHandle>::~ResourcePool() {

	}

	template <typename T, typename ResourceHandle>
	ResourceHandle ResourcePool<T, ResourceHandle>::require_resource() {
		ASSERT(!freeList.empty(), "no available resource!");

		ResourceHandle handle = freeList.front();
		freeList.pop();
		return handle;
	}

	template <typename T, typename ResourceHandle>
	T& ResourcePool<T, ResourceHandle>::get_resource(ResourceHandle handle) {
		ASSERT(handle>=0 && handle<data.size(), "invalid resource handle!");

		return data.at(handle);
	}

	template <typename T, typename ResourceHandle>
	void ResourcePool<T, ResourceHandle>::release_resource(ResourceHandle handle) {
		ASSERT(handle >= 0 && handle < data.size(), "invalid resource handle!");

		freeList.push(handle);
	}
}