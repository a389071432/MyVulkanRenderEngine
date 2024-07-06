#pragma once

#include"vulkan/vulkan_core.h"
#include"datatypes.h"
#include"GPUDevice.h"
#include"RenderGraph.h"

namespace zzcVulkanRenderEngine {
	class Engine {
	public:
		Engine();
		~Engine();

		void run();
		void setDevice(GPUDevice* device);
		void setRenderGraph(RenderGraph* graph);
		void setPresentFormat(DataFormat format);
	private:
		GPUDevice* device;
		RenderGraph* renderGraph;

		u32 currentFrame = 0;

		// configs 
		// TODO: set as staring configs parameters
		const u32 frameInFlight = 2;

		// Sync objects
		std::vector<VkFence> fencesFrame;
		std::vector<VkSemaphore> semaphoresImageAvailable;
		std::vector<VkSemaphore> semaphoresRenderDone;

		void init();
		void mainLoop();

		// used for final presentation of the resulting image
		void presentFinalImage(u32 imageIndex);

	};
}
