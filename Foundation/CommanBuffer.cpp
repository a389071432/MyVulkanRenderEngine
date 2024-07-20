#include"CommandBuffer.h"
#include"utils/utils.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {
	CommandBuffer::CommandBuffer() {

	}

	CommandBuffer::~CommandBuffer() {

	}

	VkCommandBuffer& CommandBuffer::getCmdBuffer() {
		return cmdBuffer;
	}

	void CommandBuffer::begin() {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional
		ASSERT(
			vkBeginCommandBuffer(cmdBuffer, &beginInfo) == VK_SUCCESS,
			"Failed to begin the command buffer!"
		);
	}

	void CommandBuffer::end() {
		ASSERT(
			vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS,
			"Failed to end the command buffer!"
		);
	}

	void CommandBuffer::reset() {
		ASSERT(
			vkResetCommandBuffer(cmdBuffer, 0) == VK_SUCCESS,
			"Failed to reset the command buffer!"
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
			src.image, util_getImageLayout(src.access[0]),
			dst.image, util_getImageLayout(dst.access[0]),
			1, &copyRegion
		);
	}

	void CommandBuffer::cmdCopyBufferToImage(Buffer& buffer, Texture& tex, u32 width, u32 height, u32 depth) {
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			depth
		};

		vkCmdCopyBufferToImage(
			cmdBuffer,
			buffer.buffer,
			tex.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
	}

	// Insert an image barrier
	// AccessMask, layout and pipelineStage are all determined by the accessType
	// Note that this func insert barrier for a single mip level 
	void CommandBuffer::cmdInsertImageBarrier(Texture& texture, GraphResourceAccessType newAccessType, u16 baseMipLevel) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = texture.image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = util_getAccessFlags(texture.access[baseMipLevel]);
		barrier.oldLayout = util_getImageLayout(texture.access[baseMipLevel]);
		barrier.dstAccessMask = util_getAccessFlags(newAccessType);
		barrier.newLayout = util_getImageLayout(newAccessType);
		barrier.subresourceRange.aspectMask = (newAccessType == GraphResourceAccessType::WRITE_DEPTH) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = 1;

		VkPipelineStageFlags srcStage = util_getPipelineStageFlags(texture.access[baseMipLevel]);
		VkPipelineStageFlags dstStage = util_getPipelineStageFlags(newAccessType);

		vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void CommandBuffer::cmdBlitImage(Texture& srcTex, Texture& dstTex, u16 srcMip, u16 dstMip, VkOffset3D srcRegion, VkOffset3D dstRegion ){
		VkImageBlit blit{};
		blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
		blit.srcOffsets[1] = srcRegion;
		blit.srcSubresource.mipLevel = srcMip;
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
		blit.dstOffsets[1] = dstRegion;
		blit.dstSubresource.mipLevel = dstMip;
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// NOTE: here we always assume that the image have been transitioned into the default layout
		vkCmdBlitImage(cmdBuffer, srcTex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
	}

	void CommandBuffer::cmdSetViewport(float width, float height) {
		VkViewport viewport{};
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	}

	void CommandBuffer::cmdSetScissor(u32 width, u32 height, int offsetX, int offsetY) {
		VkRect2D scissor{};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = offsetX;
		scissor.offset.y = offsetY;
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
	}

	void CommandBuffer::cmdBindDescriptorSets(PipelineBindPoint bindPoint, VkPipelineLayout pipeLayout, std::vector<VkDescriptorSet> sets) {
		vkCmdBindDescriptorSets(
			cmdBuffer,
			util_getPipelineBindPoint(bindPoint),
			pipeLayout,
			0,
			static_cast<uint32_t>(sets.size()),
			sets.data(),
			0,
			nullptr
		);
	}

	void CommandBuffer::cmdBindVertex(Buffer& vertexBuffer) {
		VkDeviceSize offsets[1]{ 0 };
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer.buffer, offsets);
	}

	void CommandBuffer::cmdBindIndexBuffer(Buffer& indexBuffer) {
		vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::cmdBindGraphicsPipeline(VkPipeline pipeline){
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void CommandBuffer::cmdDrawIndexed(u32 index_cnt, u32 instance_cnt) {
		vkCmdDrawIndexed(cmdBuffer, index_cnt, instance_cnt, 0, 0, 1);
	}
}