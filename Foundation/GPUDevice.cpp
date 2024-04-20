#include"GPUDevice.h"


namespace zzcVulkanRenderEngine {
	GPUDevice::GPUDevice() :textures(defaultPoolSize), buffers(defaultPoolSize) {
		cmdBuffers.resize(maxFrameInFlight);
	}

	GPUDevice::~GPUDevice() {

	}

	ResourceHandle GPUDevice::requireTexture() {
		return textures.require_resource();
	}

	ResourceHandle GPUDevice::requireBuffer() {
		return buffers.require_resource();
	}

	Texture& GPUDevice::getTexture(ResourceHandle handle) {
		return textures.get_resource(handle);
	}

	Buffer& GPUDevice::getBuffer(ResourceHandle handle) {
		return buffers.get_resource(handle);
	}
}