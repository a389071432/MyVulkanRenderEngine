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
		void begin();
		void end();
		void cmdBeginRenderPass(VkRenderPassBeginInfo beginInfo);
		void cmdEndRenderPass();
		void cmdCopyBuffer(VkBuffer& src, VkBuffer& dst, u32 copySize);
		void cmdCopyImage(Texture& src, Texture& dst ,VkExtent2D copyExtent);
		void cmdInsertImageBarrier(Texture& texture, GraphResourceAccessType accessType, u32 baseMipLevel, u32 nMipLevels);
	private:
		VkCommandBuffer cmdBuffer;
	};
}
