#include"Resource.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {

	//For TextureCreation
	TextureCreation& TextureCreation::set_size(u16 _width, u16 _height, u16 _depth) {
		width = _width;
		height = _height;
		depth = _depth;
		return *this;
	}

	TextureCreation& TextureCreation::set_flags(u16 _nMipLevels, u16 _flags) {
		nMipLevels = _nMipLevels;
		flags = _flags;
		return *this;
	}

	TextureCreation& TextureCreation::set_format(VkFormat _format) {
		format = _format;
		return *this;
	}

	TextureCreation& TextureCreation::set_type(TextureType _type) {
		type = _type;
		return *this;
	}

	//For Texture
	void Texture::setAccessType(GraphResourceAccessType _access) {
		access = _access;
	}

	// For ResourcePool
	template <typename T>
	ResourcePool<T>::ResourcePool(u32 poolSize) {
		ASSERT(poolSize <= maxPoolSize, "required pool size exceeds maximum limit!");

		data.reserve(poolSize);
		for (u32 i = 0; i < poolSize; i++) {
			data.push_back(T());
		}

		for (u32 i = 0; i < data.size(); i++) {
			freeList.push(i);
		}
	}

	template <typename T>
	ResourcePool<T>::~ResourcePool() {

	}

	template <typename T>
	ResourceHandle ResourcePool<T>::require_resource() {
		ASSERT(!freeList.empty(), "no available resource!");

		ResourceHandle handle = freeList.front();
		freeList.pop();
		return handle;
	}

	template <typename T>
	T& ResourcePool<T>::get_resource(ResourceHandle handle) {
		ASSERT(handle>=0 && handle<data.size(), "invalid resource handle!");

		return data.at(handle);
	}

	template <typename T>
	void ResourcePool<T>::release_resource(ResourceHandle handle) {
		ASSERT(handle >= 0 && handle < data.size(), "invalid resource handle!");

		freeList.push(handle);
	}
}