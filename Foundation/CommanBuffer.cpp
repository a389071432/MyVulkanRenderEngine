#include"CommandBuffer.h"
#include"utils/utils.h"

namespace zzcVulkanRenderEngine {
	CommandBuffer::CommandBuffer() {

	}

	CommandBuffer::~CommandBuffer() {

	}

	inline VkCommandBuffer CommandBuffer::getCmdBuffer() {
		return cmdBuffer;
	}

	void CommandBuffer::cmdBeginRenderPass(VkRenderPass renderpass, VkFramebuffer framebuffer) {

	}

	void CommandBuffer::cmdEndRenderPass() {

	}

	// Insert an image barrier
	// AccessMask, layout and pipelineStage are all determined by the accessType
	void CommandBuffer::cmdInsertImageBarrier(Texture& texture, GraphResourceAccessType newAccessType, u32 baseMipLevel, u32 nMipLevels) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = texture.image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = util_getAccessFlags(texture.access);
		barrier.oldLayout = util_getImageLayout(texture.access);
		barrier.dstAccessMask = util_getAccessFlags(newAccessType);
		barrier.newLayout = util_getImageLayout(newAccessType);
		barrier.subresourceRange.aspectMask = (newAccessType == GraphResourceAccessType::WRITE_DEPTH) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = nMipLevels;

		VkPipelineStageFlags srcStage = util_getPipelineStageFlags(texture.access);
		VkPipelineStageFlags dstStage = util_getPipelineStageFlags(newAccessType);

		vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}
}