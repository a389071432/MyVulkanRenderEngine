#pragma once
#include <stdint.h>
#include<vector>
#include<queue>
#include<string>
#include"vma/vk_mem_alloc.h"
#include"enums.h"
#include"datatypes.h"

namespace zzcVulkanRenderEngine {
	typedef u32 TextureHandle;
	typedef u32 BufferHandle;
	typedef u32 DescriptorSetLayoutsHandle;      // points to a list of layouts
	typedef u32 DescriptorSetsHandle;            // points to a list of sets
	typedef u32 RenderPassHandle;
	typedef u32 FramebufferHandle;
	typedef u32 PipelineLayoutHandle;
	typedef u32 GraphicsPipelineHandle;
	typedef u32 ComputePipelineHandle;


	const TextureHandle INVALID_TEXTURE_HANDLE = -1;
	const BufferHandle INVALID_BUFFER_HANDLE = -1;
	const DescriptorSetsHandle INVALID_DESCRIPTORSETS_HANDLE = -1;

	// TODO: wrap the enumerations?
	struct SamplerCreation {
		VkFilter                        minFilter = VK_FILTER_NEAREST;
		VkFilter                        magFilter = VK_FILTER_NEAREST;
		VkSamplerMipmapMode             mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

		VkSamplerAddressMode            address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode            address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode            address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		bool enableAnisotropy = false;
		float anisotropy = 0;
		bool enableCompare = false;
		// TODO: wrap this to enum
		VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
		

		SamplerCreation& setFilterMode(VkFilter min, VkFilter mag);
		SamplerCreation& setMipMode(VkSamplerMipmapMode mip);
		SamplerCreation& setAddressMode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
		SamplerCreation& setAnisotropy(bool enable, float anisotropy);
		SamplerCreation& setCompareOp(bool enable, VkCompareOp compareOp);
	};

	struct Texture {
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory mem;
		VkFormat format;
		VkSampler sampler;
		std::vector<GraphResourceAccessType> access;   // defined separately for each mip level of the texture (NOT SURE, NEED CONFIRMATION)
		u16 width, height, depth;
		u16 nMips;

		VmaAllocation vmaAlloc;    // record allocation info on device, useful for aliasing memory allocation

		Texture() {};
		void setAccessType(GraphResourceAccessType access);
	};

	struct TextureCreation {
		u16 width;
		u16 height;
		u16 depth;
		u16 baseMipLevel = 0;
		u16 nMipLevels = 1;
		u16 flags = 0 ;    // by default to 0
		GraphResourceType resourceType;
		DataFormat format = DataFormat::FLOAT3;
		TextureType  type = TextureType::Texture2D;
		TextureHandle aliasTexture = INVALID_TEXTURE_HANDLE;

		TextureCreation& setSize(u16 width, u16 height, u16 depth);
		TextureCreation& setMipLevels(u16 nMipLevels);
		TextureCreation& setFlags(u16 flags);
		TextureCreation& setFormat(DataFormat format);
		TextureCreation& setType(TextureType type);
		TextureCreation& setAliasTexture(TextureHandle aliasTex);
	};

	// TODO: fill in this
	struct BufferCreation {
		u32 size;
		u16 usage;
		u16 prop;  // property: device local or ...
		ResourceSharingMode shareMode = ResourceSharingMode::EXCLUSIVE;
		
		BufferCreation& setSize(u32 size);
		BufferCreation& setUsage(u32 usage);
		BufferCreation& setProp(u32 prop);
		BufferCreation& setShareMode(ResourceSharingMode shareMode);
	};

	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory mem;
		VkDeviceSize size;
		Buffer() {};
	};

	struct BindingDesc {
		BindingType type;
		ShaderStage accessStage;
		u16 groupId;
		u16 binding;
	};

	struct DescriptorSetLayoutsCreation {
		std::vector<BindingDesc> bindings;
		GraphNodeType nodeType;

		DescriptorSetLayoutsCreation& setNodeType(GraphNodeType nodeType);
		DescriptorSetLayoutsCreation& addBinding(BindingDesc binding);
	};

	struct DescriptorSetsAlloc {
		DescriptorSetLayoutsHandle layoutsHandle;

		DescriptorSetsAlloc& setLayoutsHandle(DescriptorSetLayoutsHandle layouts);
	};

	struct DescriptorSetWrite {
		// resource to bind
		BindingType type;
		union {
			TextureHandle texHandle;
			BufferHandle bufferhandle;
		}resource;

		// position to bind
		u16 dstSetId;
		u16 dstBinding;

		DescriptorSetWrite& setType(BindingType type);
		DescriptorSetWrite& setDstSet(u16 setId);
		DescriptorSetWrite& setDstBinding(u16 binding);
		DescriptorSetWrite& setTexHandle(TextureHandle texHandle);
		DescriptorSetWrite& setBufferHandle(BufferHandle bufferHandle);
	};

	struct VertexBindingDesc {
		u32 binding;
		u32 stride;
		VertexInputRate inputRate = VertexInputRate::VERTEX;        // by default
	};


	struct VertexAttribute {
		u32 binding = 0;
		u32 location = 0;
		u32 offset = 0;
		DataFormat format;
	};

	struct PipelineLayoutCreation {
		DescriptorSetLayoutsHandle descLayoutsHandle;

		DescriptorSetLayoutsHandle& setDescLayouts(DescriptorSetLayoutsHandle descLayoutsHandle);
	};

	struct GraphicsPipelineCreation {
		struct ShaderInfo{
			std::string vertShaderPath;
			std::string fragShaderPath;
		} shaders;

		struct VertexInput {
			VertexBindingDesc bindingDesc;
			std::vector<VertexAttribute> attributes;
			
			VertexInput& setBindingDesc(VertexBindingDesc bindingDesc);
			VertexInput& addVertexAttribute(VertexAttribute attribute);
		}vertexInput;

		struct RasterizerInfo {
			CullMode cullMode;
			FrontFace frontFace;
		} rasterInfo;

		struct MSAAInfo {
			u32 nSamplesPerPixel;
		}msaa;

		struct DepthStencilInfo {
			bool enableDepth;
		}depthStencil;

		PipelineLayoutHandle pipelineLayoutHandle;

		struct RenderPassInfo {
			RenderPassHandle renderPassHandle;
		}renderPassInfo;

		GraphicsPipelineCreation& setShaderInfo(ShaderInfo shaderInfo);
		GraphicsPipelineCreation& setVertexInput(VertexInput vertexInput);
		GraphicsPipelineCreation& setRasterizerInfo(RasterizerInfo rasterInfo);
		GraphicsPipelineCreation& setMSAAInfo(MSAAInfo msaaInfo);
		GraphicsPipelineCreation& setDepthStencilInfo(DepthStencilInfo depthStencilInfo);
		GraphicsPipelineCreation& setPipelineLayout(PipelineLayoutHandle pipelineLayoutHandle);
		GraphicsPipelineCreation& setRenderPassInfo(RenderPassInfo renderPassInfo);
	};

	struct RenderAttachmentInfo {
		DataFormat format;
		GraphResourceType resourceType;
	};

	struct RenderPassCreation {
		std::vector<RenderAttachmentInfo> attachmentInfos;

		RenderPassCreation& addAttachInfo(RenderAttachmentInfo info);
	};

	struct FramebufferCreation {
		std::vector<TextureHandle> attachments;
		RenderPassHandle renderPassHandle;
		u32 width;
		u32 height;
		u32 layers = 1;

		FramebufferCreation& addAttachment(TextureHandle tex);
		FramebufferCreation& setRenderPass(RenderPassHandle renderpass);
		FramebufferCreation& setSize(u32 width,u32 height);
		FramebufferCreation& setLayers(u32 nLayers);
	};

	// TODO: An optimized version for 2D data
	template<typename T, typename ResourceHandle>
	class ResourcePool {
	public:
		ResourcePool(u32 poolSize);
		~ResourcePool();
		ResourceHandle require_resource();
		T& get_resource(ResourceHandle handle);
		void release_resource(ResourceHandle handle);
	private:
		u32 poolSize;
		const u32 maxPoolSize;
		std::vector<T> data;
		std::queue<ResourceHandle> freeList;
	};
}
