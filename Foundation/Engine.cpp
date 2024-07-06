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
		

		// init sync objects
		fencesFrame.resize(frameInFlight);
		semaphoresImageAvailable.resize(frameInFlight);
		semaphoresRenderDone.resize(frameInFlight);

		for (u32 i = 0; i < frameInFlight; i++) {
			VkFenceCreateInfo fenceCI{};
			fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			vkCreateFence(device->getDevice(), &fenceCI, nullptr, &fencesFrame[i]);
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

		// init for the final presenting pass
		RenderPassCreation passCI{};
		passCI.addAttachInfo({ presentFormat,GraphResourceType::TEXTURE });
		finalPass = device->createRenderPass(passCI);


	}

	void Engine::run() {
		init();
		mainLoop();
	}

	void Engine::mainLoop() {
		while () {
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
			renderGraph->execute(_cmdBuffer);


			// TODO: a dedicated renderpass (fullQuad) to write the resulting image from renderGraph to the final swapchain image for presentation
			presentFinalImage(imageIndex);

			// Submit to queue
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &semaphoresImageAvailable[currentFrame];
			VkPipelineStageFlags waitStages[]={ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &semaphoresRenderDone[currentFrame];
			submitInfo.commandBufferCount = 1;
			VkCommandBuffer cmdBuffer = _cmdBuffer.getCmdBuffer();
			submitInfo.pCommandBuffers = &(cmdBuffer);

			ASSERT(
				vkQueueSubmit(device->getMainQueue(), 1, &submitInfo, fencesFrame[currentFrame]) == VK_SUCCESS,
				"Assertion failed: QueueSubmit failed!"
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
}