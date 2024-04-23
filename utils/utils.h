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
			ASSERT(false, "Assertion failed: invalid GraphResourceAccessType!");
			break;
		}
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
			ASSERT(false, "Assertion failed: invalid GraphResourceAccessType!");
			break;
		}
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
			ASSERT(false, "Assertion failed: invalid GraphResourceAccessType!");
			break;
		}
	}

	inline VkImageType util_getImageType(TextureType textureType) {
		switch (textureType)
		{
		case TextureType::Texture1D:
			return VK_IMAGE_TYPE_1D;
		case TextureType::Texture2D:
			return VK_IMAGE_TYPE_2D;
		case TextureType::Texture3D:
			return VK_IMAGE_TYPE_3D;
		default:
			ASSERT(false, "Assertion failed: invalid TextureType!");
			break;
		}
	}

	inline VkImageViewType util_getImageViewType(TextureType textureType) {
		switch (textureType)
		{
		case TextureType::Texture1D:
			return VK_IMAGE_VIEW_TYPE_1D;
		case TextureType::Texture2D:
			return VK_IMAGE_VIEW_TYPE_2D;
		case TextureType::Texture3D:
			return VK_IMAGE_VIEW_TYPE_3D;
		default:
			ASSERT(false, "Assertion failed: invalid TextureType!");
			break;
		}
	}

	inline VkShaderStageFlags util_getShaderStageFlags(ShaderStage stage) {
		switch (stage)
		{
		case ShaderStage::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case ShaderStage::FRAG:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case ShaderStage::RANDOM:
			return VK_SHADER_STAGE_ALL_GRAPHICS;
			break;
		default:
			ASSERT(false, "Assertion failed: invalid ShaderStage!");
			break;
		}
	}

	inline VkDescriptorType util_getDescriptorType(BindingType bindingType) {
		// TODO: more types
		switch (bindingType)
		{
		case BindingType::IMAGE_SAMPLER:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case BindingType::UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		default:
			ASSERT(false, "Assertion failed: invalid BindingType!");
			break;
		}
	}

	// TODO: modify this method with isRead considered
	inline BindingType util_getBindingType(GraphResourceType rType,bool isRead) {
		switch (rType)
		{
		case GraphResourceType::TEXTURE:
			return BindingType::IMAGE_SAMPLER;
		case GraphResourceType::DEPTH_MAP:
			return BindingType::IMAGE_SAMPLER;
		case GraphResourceType::BUFFER:
			return BindingType::UNIFORM_BUFFER;
		default:
			ASSERT(false, "Assertion failed: invalid GraphResourceType!");
			break;
		}
	}
}


