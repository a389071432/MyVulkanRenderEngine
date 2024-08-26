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

	TextureCreation& TextureCreation::setFlags(u16 _flags) {
		flags = _flags;
		return *this;
	}

	TextureCreation& TextureCreation::setMipLevels(u16 _nMipLevels) {
		nMipLevels = _nMipLevels;
		return *this;
	}

	TextureCreation& TextureCreation::setFormat(DataFormat _format) {
		format = _format;
		return *this;
	}

	TextureCreation& TextureCreation::setType(TextureType _type) {
		type = _type;
		return *this;
	}

	TextureCreation& TextureCreation::setUsage(VkImageUsageFlags _usage) {
		usage = _usage;
		return *this;
	}

	TextureCreation& TextureCreation::setAliasTexture(TextureHandle _aliasTex) {
		aliasTexture = _aliasTex;
		return *this;
	}

	TextureCreation& TextureCreation::setIsFinal(bool isFinal) {
		isFinalOutput = isFinal;
		return *this;
	}

	//For buffer creation
	BufferCreation& BufferCreation::setSize(u32 _size) {
		size = _size;
		return *this;
	}

	BufferCreation& BufferCreation::setUsage(VkBufferUsageFlags _usage) {
		usage = _usage;
		return *this;
	}

	BufferCreation& BufferCreation::setProp(u32 _prop) {
		prop = _prop;
		return *this;
	}

	BufferCreation& BufferCreation::setShareMode(ResourceSharingMode _shareMode) {
		shareMode = _shareMode;
		return *this;
	}

	// For Texture
	void Texture::setAccessType(GraphResourceAccessType _access, u16 baseMip, u16 nMips) {
		for (u16 i = baseMip; i < baseMip + nMips; i++) {
			access[i] = _access;
		}
	}

	void Texture::setNowQueueFamily(u32 queueFamilyIndex) {
		nowQueueFamily = queueFamilyIndex;
	}

	//// For ResourcePool
	//template<class T, class ResourceHandle>
	//ResourcePool<T,ResourceHandle>::ResourcePool(u32 poolSize) {
	//	ASSERT(poolSize <= maxPoolSize, "required pool size exceeds maximum limit!");

	//	data.reserve(poolSize);
	//	for (u32 i = 0; i < poolSize; i++) {
	//		T t;
	//		data.push_back(t);
	//	}

	//	for (u32 i = 0; i < data.size(); i++) {
	//		freeList.push(i);
	//	}
	//}

	//template<class T, class ResourceHandle>
	//ResourcePool<T, ResourceHandle>::~ResourcePool() {

	//}

	//template<class T, class ResourceHandle>
	//ResourceHandle ResourcePool<T, ResourceHandle>::require_resource() {
	//	ASSERT(!freeList.empty(), "no available resource!");

	//	ResourceHandle handle = freeList.front();
	//	freeList.pop();
	//	return handle;
	//}

	//template<class T, class ResourceHandle>
	//T& ResourcePool<T, ResourceHandle>::get_resource(ResourceHandle handle) {
	//	ASSERT(handle>=0 && handle<data.size(), "invalid resource handle!");

	//	return data.at(handle);
	//}

	//template<class T, class ResourceHandle>
	//void ResourcePool<T, ResourceHandle>::release_resource(ResourceHandle handle) {
	//	ASSERT(handle >= 0 && handle < data.size(), "invalid resource handle!");

	//	freeList.push(handle);
	//}

}