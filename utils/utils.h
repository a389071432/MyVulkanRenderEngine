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
			ASSERT(false, "Assertion failed: invalid Enum GraphResourceAccessType!");
			break;
		}
	}

	inline VkSharingMode util_getSharingMode(ResourceSharingMode shareMode) {
		switch (shareMode)
		{
		case ResourceSharingMode::EXCLUSIVE:
			return VK_SHARING_MODE_EXCLUSIVE;
		case ResourceSharingMode::CONCURRENT:
			return VK_SHARING_MODE_CONCURRENT;
		default:
			ASSERT(false, "Assertion failed: invalid Enum ResourceSharingMode!");
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
		case GraphResourceAccessType::COPY_SRC:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case GraphResourceAccessType::COPY_DST:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case GraphResourceAccessType::PRESENT:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		default:
			ASSERT(false, "Assertion failed: invalid Enum GraphResourceAccessType!");
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
			ASSERT(false, "Assertion failed: invalid Enum GraphResourceAccessType!");
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
			ASSERT(false, "Assertion failed: invalid Enum TextureType!");
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
			ASSERT(false, "Assertion failed: invalid Enum TextureType!");
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
		case ShaderStage::ALL:
			return VK_SHADER_STAGE_ALL_GRAPHICS;
			break;
		case ShaderStage::RANDOM:
			return VK_SHADER_STAGE_ALL_GRAPHICS;
			break;
		case ShaderStage::DONT_CARE:
			return 0;
			break;
		default:
			ASSERT(false, "Assertion failed: invalid Enum ShaderStage!");
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
			ASSERT(false, "Assertion failed: invalid Enum BindingType!");
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
			ASSERT(false, "Assertion failed: invalid Enum GraphResourceType!");
			break;
		}
	}

	inline VkVertexInputRate util_getVertexInputRate(VertexInputRate inputRate) {
		switch (inputRate)
		{
		case zzcVulkanRenderEngine::VertexInputRate::VERTEX:
			return VK_VERTEX_INPUT_RATE_VERTEX;
			break;
		case zzcVulkanRenderEngine::VertexInputRate::INSTANCE:
			return VK_VERTEX_INPUT_RATE_INSTANCE;
			break;
		default:
			ASSERT(false, "Assertion failed: invalid Enum VertexInputRate!");
			break;
		}
	}

	inline VkFormat util_getFormat(DataFormat format) {
		switch (format)
		{
		case zzcVulkanRenderEngine::DataFormat::FLOAT:
			return VK_FORMAT_R32_SFLOAT;
			break;
		case zzcVulkanRenderEngine::DataFormat::FLOAT2:
			return VK_FORMAT_R32G32_SFLOAT;
			break;
		case zzcVulkanRenderEngine::DataFormat::FLOAT3:
			return VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case zzcVulkanRenderEngine::DataFormat::FLOAT4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case zzcVulkanRenderEngine::DataFormat::INT:
			return VK_FORMAT_R32_SINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::INT2:
			return VK_FORMAT_R32G32_SINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::INT3:
			return VK_FORMAT_R32G32B32_SINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::INT4:
			return VK_FORMAT_R32G32B32A32_SINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::UINT:
			return VK_FORMAT_R32_UINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::UINT2:
			return VK_FORMAT_R32G32_UINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::UINT3:
			return VK_FORMAT_R32G32B32_UINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::UINT4:
			return VK_FORMAT_R32G32B32A32_UINT;
			break;
		case zzcVulkanRenderEngine::DataFormat::UNORM:
			return VK_FORMAT_R8_UNORM;
			break;
		case zzcVulkanRenderEngine::DataFormat::UNORM2:
			return VK_FORMAT_R8G8_UNORM;
			break;
		case zzcVulkanRenderEngine::DataFormat::UNORM3:
			return VK_FORMAT_R8G8B8_UNORM;
			break;
		case zzcVulkanRenderEngine::DataFormat::UNORM4:
			return VK_FORMAT_R8G8B8A8_UNORM;
			break;
		default:
			ASSERT(false, "Assertion failed: invalid Enum DataFormat!");
			break;
		}
	}

	inline VkCullModeFlags util_getCullMode(CullMode cullMode) {
		switch (cullMode)
		{
		case zzcVulkanRenderEngine::CullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
			break;
		case zzcVulkanRenderEngine::CullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
			break;
		default:
			ASSERT(false, "Assertion failed: invalid Enum CullMode!");
			break;
		}
	}

	inline VkFrontFace util_getFrontFace(FrontFace front) {
		switch (front)
		{
		case zzcVulkanRenderEngine::FrontFace::CLOCKWISE:
			return VK_FRONT_FACE_CLOCKWISE;
			break;
		case zzcVulkanRenderEngine::FrontFace::CONTER_CLOCKWISE:
			return VK_FRONT_FACE_CLOCKWISE;
			break;
		default:
			ASSERT(false, "Assertion failed: invalid Enum CullFrontFace!");
			break;
		}
	}
}


