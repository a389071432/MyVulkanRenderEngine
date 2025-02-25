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

		// create the window
		ASSERT(
			glfwInit(),
			"Assertion failed: glfw initialization failed!"
		);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(scr_width, scr_height, "Vulkan", nullptr, nullptr);
		ASSERT(
			window,
			"Assertion failed: failed to create the window!"
		);

		// share the window with device
		device->setWindow(window);
		device->init();

		// init the scene
		scene->setDevice(device);

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

		// compile the render graph
		renderGraph->setDevice(device);
		renderGraph->compile();

		ASSERT(
			maxThreadPerFrame > renderGraph->getGraphNodeCount(),
			"Assertion failed: number of command buffers per frame must be greater than number of render nodes"
		);
	}

	void Engine::run() {
		//init();
		mainLoop();
	}

	// TODO: remove _device
	// all operations move to GPUDevice
	void Engine::mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			// Temp varaibles
			VkDevice _device = device->getDevice();
			VkSwapchainKHR& _swapChain = device->getSwapChain();
			std::vector<CommandBuffer>& frameCmdBuffers = device->getFrameCommandBuffers(currentFrame);
			u32 nodeCount = renderGraph->getGraphNodeCount();

			// Synchronization of the same frame
			vkWaitForFences(_device, 1, &fencesFrame[currentFrame], VK_TRUE, UINT64_MAX);
			vkResetFences(_device, 1, &fencesFrame[currentFrame]);

			// Acquire an image from swapchain
			uint32_t imageIndex;
			ASSERT(
				vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, semaphoresImageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex) == VK_SUCCESS,
				"Assertion failed: Acquire image from swapchain failed!"
			);

			// the (nodeCount)-th command buffer is used for the built-in present pass
			for (u32 i = 0; i < nodeCount+1; i++) {
				frameCmdBuffers[i].reset();
				frameCmdBuffers[i].begin();
			}
			// Execture the renderGraph (involves recording commands)
			renderGraph->execute(frameCmdBuffers, device, scene);

			// Copy the final output of renderGraph to the swapchain for presentation
			presentFinalImage(imageIndex);
			
			for (u32 i = 0; i < nodeCount+1; i++) {
				frameCmdBuffers[i].end();
			}

			// Submit to queue
			device->submitCmds(
				device->getMainQueue(), 
				frameCmdBuffers,
				nodeCount+1,
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
		CommandBuffer& cmdBuffer = device->getCommandBuffer(currentFrame,renderGraph->getGraphNodeCount());
		TextureHandle finalTex = renderGraph->getTextureByKey("final");
		TextureHandle presentTex = device->getSwapChainImageByIndex(imageIndex);
		device->transferImageInDevice(cmdBuffer,finalTex, presentTex,device->getSwapChainExtent());
		// layout transition for presentation
		device->imageLayoutTransition(cmdBuffer, presentTex, GraphResourceAccessType::PRESENT, 0, 1);
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