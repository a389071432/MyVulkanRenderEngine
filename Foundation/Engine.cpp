#include"Engine.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {
	Engine::Engine() {

	}

	Engine::~Engine() {

	}

	void Engine::init() {
		// Check for completeness
		ASSERT(device != nullptr, "Device is NULL!");
		ASSERT(renderGraph != nullptr, "RenderGraph is NULL!");
		ASSERT(scene != nullptr, "Scene is NULL!");

		// init sync objects
		fencesFrame.resize(frameInFlight);
		semaphoresImageAvailable.resize(frameInFlight);
		semaphoresRenderDone.resize(frameInFlight);

		for (u32 i = 0; i < frameInFlight; i++) {
			VkFenceCreateInfo fenceCI{};
			fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			ASSERT(
				vkCreateFence(device->getDevice(), &fenceCI, nullptr, &fencesFrame[i]) == VK_SUCCESS,
				"Assertion failed: CreateFence failed!"
			);

			VkSemaphoreCreateInfo semaCI{};
			semaCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			ASSERT(
				vkCreateSemaphore(device->getDevice(), &semaCI, nullptr, &semaphoresImageAvailable[i]) == VK_SUCCESS,
				"Assertion failed: Create ImageAvailable Semaphore failed!"
			);

			ASSERT(
				vkCreateSemaphore(device->getDevice(), &semaCI, nullptr, &semaphoresRenderDone[i]) == VK_SUCCESS,
				"Assertion failed: Create RenderDone Semaphore failed!"
			);
		}

		// create the window
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(scr_width, scr_width, "Vulkan", nullptr, nullptr);

		// compile the render graph
		renderGraph->compile();
	}

	void Engine::run() {
		init();
		mainLoop();
	}

	// TODO: remove _device
	// all operations move to GPUDevice
	void Engine::mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			// Temp varaibles
			VkDevice _device = device->getDevice();
			VkSwapchainKHR _swapChain = device->getSwapChain();
			CommandBuffer& _cmdBuffer = device->getCommandBuffer(currentFrame);

			// Synchronization of the same frame
			vkWaitForFences(_device, 1, &fencesFrame[currentFrame], VK_TRUE, UINT64_MAX);
			vkResetFences(_device, 1, &fencesFrame[currentFrame]);

			// Acquire an image from swapchain
			uint32_t imageIndex;
			ASSERT(
				vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, semaphoresImageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex) == VK_SUCCESS,
				"Assertion failed: Acquire image from swapchain failed!"
			);

			// Execture the renderGraph (involves recording commands)
			renderGraph->execute(&_cmdBuffer, device, scene);


			// TODO: a dedicated renderpass (fullQuad) to write the resulting image from renderGraph to the final swapchain image for presentation
			presentFinalImage(imageIndex);

			// Submit to queue
			device->submitCmds(
				device->getMainQueue(), 
				{ _cmdBuffer.getCmdBuffer() }, 
				{ semaphoresImageAvailable[currentFrame]},
				{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
				{ semaphoresRenderDone[currentFrame] },
				fencesFrame[currentFrame]
				);

			// Present
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &_swapChain;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &semaphoresRenderDone[currentFrame];
			presentInfo.pImageIndices = &imageIndex;

			ASSERT(
				vkQueuePresentKHR(device->getPresentQueue(), &presentInfo) == VK_SUCCESS,
				"Assertion failed: QueuePresent failed!"
			);
			
			currentFrame = (currentFrame + 1) % frameInFlight;
		}
	}

	void Engine::presentFinalImage(u32 imageIndex) {
		TextureHandle finalTex = renderGraph->getTextureByKey("final");
		TextureHandle presentTex = device->getSwapChainImageByIndex(imageIndex);
		device->transferImageInDevice(finalTex, presentTex,device->getSwapChainExtent());

	}

	void Engine::setDevice(GPUDevice* _device) {
		device = _device;
	}

	void Engine::setRenderGraph(RenderGraph* graph) {
		renderGraph = graph;
	}

	void Engine::setScene(Scene* _scene) {
		scene = _scene;
	}

	void Engine::setPresentFormat(DataFormat _format) {
		
	}

	void Engine::setDisplayResolution(u32 width, u32 height) {
		scr_width = width;
		scr_height = height;
	}
}