#pragma once
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
		void cmdCopyBufferToImage(Buffer& buffer, Texture& tex, u32 width, u32 height, u32 depth);
		void cmdInsertImageBarrier(Texture& texture, GraphResourceAccessType newAccessType, u16 baseMipLevel);
		void cmdBlitImage(Texture& srcTex, Texture& dstTex, u16 srcMip, u16 dstMip, VkOffset3D srcRegion, VkOffset3D dstRegion);
	private:
		VkCommandBuffer cmdBuffer;
	};
}
