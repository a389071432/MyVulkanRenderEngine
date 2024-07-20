#pragma once
#include"vulkan/vulkan_core.h"
#include"Foundation/enums.h"
#include"Foundation/assert.h"
#include"Third parties/tinygltf/tiny_gltf.h"

namespace zzcVulkanRenderEngine {
	inline VkAccessFlags util_getAccessFlags(GraphResourceAccessType accessType) {
		switch (accessType)
		{
		case GraphResourceAccessType::READ_TEXTURE:
			return VK_ACCESS_SHADER_READ_BIT;
		case GraphResourceAccessType::WRITE_ATTACHMENT:
			return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case GraphResourceAccessType::WRITE_DEPTH:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case GraphResourceAccessType::COPY_SRC:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case GraphResourceAccessType::COPY_DST:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		case GraphResourceAccessType::WRITE_STORAGE:
			return VK_ACCESS_SHADER_WRITE_BIT;
		case GraphResourceAccessType::PRESENT:
			return VK_ACCESS_MEMORY_READ_BIT;
		case GraphResourceAccessType::UNDEFINED:
			return VK_ACCESS_NONE;
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
		case GraphResourceAccessType::UNDEFINED:
				return VK_IMAGE_LAYOUT_UNDEFINED;
		default:
			ASSERT(false, "Assertion failed: invalid Enum GraphResourceAccessType!");
			break;
		}
	}

	// 
	inline VkPipelineStageFlags util_getPipelineStageFlags(GraphResourceAccessType accessType) {
		switch (accessType)
		{
		case GraphResourceAccessType::READ_TEXTURE:
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		case GraphResourceAccessType::WRITE_ATTACHMENT:
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case GraphResourceAccessType::WRITE_DEPTH:
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		case GraphResourceAccessType::COPY_SRC:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case GraphResourceAccessType::COPY_DST:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case GraphResourceAccessType::PRESENT:
			return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		case GraphResourceAccessType::UNDEFINED:
			return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		default:
			ASSERT(false, "Assertion failed: invalid Enum GraphResourceAccessType!");
			break;
		}
	}

	//inline VkPipelineStageFlags util_getPipelineStageFlags(ShaderStage accessStage) {
	//	switch (accessType)
	//	{
	//	case GraphResourceAccessType::READ_TEXTURE:
	//		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	//	case GraphResourceAccessType::WRITE_ATTACHMENT:
	//		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//	case GraphResourceAccessType::WRITE_DEPTH:
	//		return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//	case GraphResourceAccessType::COPY_SRC:
	//		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	//	case GraphResourceAccessType::COPY_DST:
	//		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	//	default:
	//		ASSERT(false, "Assertion failed: invalid Enum GraphResourceAccessType!");
	//		break;
	//	}
	//}

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

	inline DataFormat util_getDataFormat(::tinygltf::Image image) {
		DataFormat format = DataFormat::UNDEFINED;

		// Helper function to choose between UNORM and FLOAT
		auto chooseFormat = [](int components, bool isFloat) {
			switch (components) {
			case 1: return isFloat ? DataFormat::FLOAT : DataFormat::UNORM;
			case 2: return isFloat ? DataFormat::FLOAT2 : DataFormat::UNORM2;
			case 3: return isFloat ? DataFormat::FLOAT3 : DataFormat::UNORM3;
			case 4: return isFloat ? DataFormat::FLOAT4 : DataFormat::UNORM4;
			default: return DataFormat::UNDEFINED;
			}
		};

		bool isFloat = false;

		// Check bits per channel
		if (image.bits == 32) {
			isFloat = true; 
		}
		else if (image.bits != 8 && image.bits != 16) {
			std::cerr << "Unexpected bit depth: " << image.bits << std::endl;
			return DataFormat::UNDEFINED;
		}

		// These are typically UNORM
		if (image.mimeType == "image/png" || image.mimeType == "image/jpeg") {
			isFloat = false;  
		}

		// Determine format based on components and float/int
		format = chooseFormat(image.component, isFloat);
	}

	inline VkPipelineBindPoint util_getPipelineBindPoint(PipelineBindPoint bindPoint) {
		switch (bindPoint)
		{
		case zzcVulkanRenderEngine::PipelineBindPoint::GRAPHICS:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
			break;
		case zzcVulkanRenderEngine::PipelineBindPoint::COMPUTE:
			return VK_PIPELINE_BIND_POINT_COMPUTE;
			break;
		case zzcVulkanRenderEngine::PipelineBindPoint::RAY_TRACING:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
			break;
		default:
			break;
		}
	}
}


