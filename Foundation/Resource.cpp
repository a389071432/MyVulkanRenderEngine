#include"Resource.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {

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