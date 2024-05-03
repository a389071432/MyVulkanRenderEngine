#pragma once
#include"vulkan/vulkan_core.h"
#include"datatypes.h"
#include"Resource.h"
#include"enums.h"

namespace zzcVulkanRenderEngine {
	class CommandBuffer {
	public:
		CommandBuffer();
		~CommandBuffer();

		// Wrapped commands for easy usage, free users from filling in createInfo ...
		VkCommandBuffer getCmdBuffer();
		void cmdBeginRenderPass(VkRenderPass renderpass, VkFramebuffer framebuffer);
		void cmdEndRenderPass();
		void cmdInsertImageBarrier(Texture& texture, GraphResourceAccessType accessType, u32 baseMipLevel, u32 nMipLevels);
	private:
		VkCommandBuffer cmdBuffer;
	};
}
