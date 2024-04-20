#pragma once
#include"vulkan/vulkan_core.h"

namespace zzcVulkanRenderEngine {
	class CommandBuffer {
	public:
		CommandBuffer();
		~CommandBuffer();

		// Wrapped commands for easy usage, free users from filling in createInfo ...
		void cmdBeginRenderPass(VkRenderPass renderpass, VkFramebuffer framebuffer);
		void cmdEndRenderPass();
	private:
		VkCommandBuffer cmdBuffer;
	};
}
