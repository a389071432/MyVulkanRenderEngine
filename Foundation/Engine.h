#pragma once

#include"vulkan/vulkan_core.h"
#include"datatypes.h"
#include"GPUDevice.h"
#include"RenderGraph.h"
#include"Scene.h"

namespace zzcVulkanRenderEngine {
	class Engine {
	public:
		Engine();
		~Engine();

		void init();
		void run();
		void setDevice(GPUDevice* device);
		void setRenderGraph(RenderGraph* graph);
		void setScene(Scene* scene);
		void setPresentFormat(DataFormat format);
		void setDisplayResolution(u32 width, u32 height);
	private:
		u32 scr_width = 800;
		u32 scr_height = 600;
		GLFWwindow* window;
		GPUDevice* device;
		RenderGraph* renderGraph;
		Scene* scene;

		u32 currentFrame = 0;

		// configs 
		// TODO: set as staring configs parameters
		const u32 frameInFlight = 2;

		// Sync objects
		std::vector<VkFence> fencesFrame;
		std::vector<VkSemaphore> semaphoresImageAvailable;
		std::vector<VkSemaphore> semaphoresRenderDone;

		void mainLoop();

		// used for final presentation of the resulting image
		void presentFinalImage(u32 imageIndex);

	};
}
