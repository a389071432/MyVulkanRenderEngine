#include"CommandBuffer.h"
#include"utils/utils.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {
	CommandBuffer::CommandBuffer() {

	}

	CommandBuffer::~CommandBuffer() {

	}

	inline VkCommandBuffer CommandBuffer::getCmdBuffer() {
		return cmdBuffer;
	}

	void CommandBuffer::begin() {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		ASSERT(
			vkBeginCommandBuffer(cmdBuffer, &beginInfo),
			"Failed to begin the command buffer!"
		);
	}

	void CommandBuffer::end() {
		ASSERT(
			vkEndCommandBuffer(cmdBuffer),
			"Failed to end the command buffer!"
		);
	}

	// TODO: wrap the beginInfo?
	void CommandBuffer::cmdBeginRenderPass(VkRenderPassBeginInfo beginInfo) {
		vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::cmdEndRenderPass() {
		vkCmdEndRenderPass(cmdBuffer);
	}

	void CommandBuffer::cmdCopyBuffer(VkBuffer& src, VkBuffer& dst, u32 copySize) {
		//record the data transfer command
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = copySize;
		vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &copyRegion);
	}

	// TODO: need explicit layout transition here? since it could be done by vkCmdCopyImage
	void CommandBuffer::cmdCopyImage(Texture& src, Texture& dst, VkExtent2D copyExtent) {
		//// layout transition of the src image
		//cmdInsertImageBarrier(src, GraphResourceAccessType::COPY_SRC, 0, 1);

		//// layout transition of the dst image
		//cmdInsertImageBarrier(dst, GraphResourceAccessType::COPY_DST, 0, 1);

		//// Copy from src to dst
		//VkImageCopy copyRegion = {};
		//copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//copyRegion.srcSubresource.mipLevel = 0;
		//copyRegion.srcSubresource.baseArrayLayer = 0;
		//copyRegion.srcSubresource.layerCount = 1;
		//copyRegion.srcOffset = { 0, 0, 0 };
		//copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//copyRegion.dstSubresource.mipLevel = 0;
		//copyRegion.dstSubresource.baseArrayLayer = 0;
		//copyRegion.dstSubresource.layerCount = 1;
		//copyRegion.dstOffset = { 0, 0, 0 };
		//copyRegion.extent.width = copyExtent.width;
		//copyRegion.extent.height = copyExtent.height;
		//copyRegion.extent.depth = 1;

		//vkCmdCopyImage(
		//	cmdBuffer,
		//	src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		//	dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	1, &copyRegion
		//);

		VkImageCopy copyRegion = {};
		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.baseArrayLayer = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent.width = copyExtent.width;
		copyRegion.extent.height = copyExtent.height;
		copyRegion.extent.depth = 1;

		vkCmdCopyImage(
			cmdBuffer,
			src.image, util_getImageLayout(src.access),
			dst.image, util_getImageLayout(dst.access),
			1, &copyRegion
		);
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