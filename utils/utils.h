#pragma once
#include"vulkan/vulkan_core.h"
#include"Foundation/enums.h"

namespace zzcVulkanRenderEngine {
	inline VkAccessFlags util_getAccessFlags(GraphResourceAccessType accessType) {
		switch (accessType)
		{
		case GraphResourceAccessType::READ_TEXTURE:
			return VK_ACCESS_SHADER_READ_BIT;
		case GraphResourceAccessType::WRITE_ATTACHMENT:
			return VK_ACCESS_SHADER_WRITE_BIT;
		case GraphResourceAccessType::WRITE_DEPTH:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		default:
			break;
		}
		return 0;
	}

	inline VkImageLayout util_getImageLayout(GraphResourceAccessType accessType) {
		switch (accessType)
		{
		case GraphResourceAccessType::READ_TEXTURE:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case GraphResourceAccessType::WRITE_ATTACHMENT:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case GraphResourceAccessType::WRITE_DEPTH:
			return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		default:
			break;
		}
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	inline VkPipelineStageFlags util_getPipelineStageFlags(GraphResourceAccessType accessType) {
		switch (accessType)
		{
		case GraphResourceAccessType::READ_TEXTURE:
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		case GraphResourceAccessType::WRITE_ATTACHMENT:
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case GraphResourceAccessType::WRITE_DEPTH:
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		default:
			break;
		}
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
}


