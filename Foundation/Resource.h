#pragma once
#include<vector>
#include<queue>
#include<string>
//#include"vma/vk_mem_alloc.h"
#include"enums.h"
#include"datatypes.h"
#include"vulkan/vulkan.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {
	typedef u32 TextureHandle;
	typedef u32 BufferHandle;
	typedef u32 DescriptorSetLayoutsHandle;      // refers to a list of layouts
	typedef u32 DescriptorSetsHandle;            // refers to a list of sets
	typedef u32 RenderPassHandle;
	typedef u32 FramebufferHandle;
	typedef u32 PipelineLayoutHandle;
	typedef u32 GraphicsPipelineHandle;
	typedef u32 ComputePipelineHandle;
	typedef u32 RayTracingPipelineHandle;


	const TextureHandle INVALID_TEXTURE_HANDLE = -1;
	const BufferHandle INVALID_BUFFER_HANDLE = -1;
	const DescriptorSetsHandle INVALID_DESCRIPTORSETS_HANDLE = -1;
	const DescriptorSetLayoutsHandle INVALID_DESCRIPTORSET_LAYOUTS_HANDLE = -1;
	const PipelineLayoutHandle INVALID_PIPELINELAYOUT_HANDLE = -1;
	const RenderPassHandle INVALID_RENDERPASS_HANDLE = -1;
	const FramebufferHandle INVALID_FRAMEBUFFER_HANDLE = -1;

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
		
		float minLod = 0.0;
		float maxLod = 0.0;
		float lodBias = 0.0;

		SamplerCreation& setFilterMode(VkFilter min, VkFilter mag) { minFilter = min; magFilter = mag; return *this; }
		SamplerCreation& setMipMode(VkSamplerMipmapMode mip, float _minLod, float _maxLod, float _lodBias) { 
			mipMode = mip; 
			minLod = _minLod;
			maxLod = _maxLod;
			lodBias = _lodBias;
			return *this;
		}
		SamplerCreation& setAddressMode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w) {
			address_mode_u = u;
			address_mode_v = v;
			address_mode_w = w;
			return *this;
		}
		SamplerCreation& setAnisotropy(bool enable, float _anisotropy) { enableAnisotropy = enable; anisotropy = _anisotropy; return *this; }
		SamplerCreation& setCompareOp(bool enable, VkCompareOp _compareOp) { enableCompare = enable; compareOp = _compareOp; return *this; }
	};

	// TODO: add params for sampler settings
	struct Texture {
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory mem;
		VkFormat format;
		VkSampler sampler;
		std::vector<GraphResourceAccessType> access;   // defined separately for each mip level of the texture (NOT SURE, NEED CONFIRMATION)
		u16 width, height, depth;
		u16 nMips;

		//VmaAllocation vmaAlloc;    // record allocation info on device, useful for aliasing memory allocation

		void setAccessType(GraphResourceAccessType access, u16 baseMip, u16 nMips);
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
		bool isFinalOutput = false;

		TextureCreation& setSize(u16 width, u16 height, u16 depth);
		TextureCreation& setMipLevels(u16 nMipLevels);
		TextureCreation& setFlags(u16 flags);
		TextureCreation& setFormat(DataFormat format);
		TextureCreation& setType(TextureType type);
		TextureCreation& setAliasTexture(TextureHandle aliasTex);
		TextureCreation& setIsFinal(bool isFinal);
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

		DescriptorSetLayoutsCreation& setNodeType(GraphNodeType type) { nodeType = type; return *this; }
		DescriptorSetLayoutsCreation& addBinding(BindingDesc binding) { bindings.push_back(binding); return *this; }
	};

	struct DescriptorSetsAlloc {
		DescriptorSetLayoutsHandle layoutsHandle;

		DescriptorSetsAlloc& setLayoutsHandle(DescriptorSetLayoutsHandle layouts) { layoutsHandle = layouts; return *this; }
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

		DescriptorSetWrite& setType(BindingType _type) { type = _type; return *this; }
		DescriptorSetWrite& setDstSet(u16 setId) { dstSetId = setId; return *this; }
		DescriptorSetWrite& setDstBinding(u16 binding) { dstBinding = binding; return *this; }
		DescriptorSetWrite& setTexHandle(TextureHandle texHandle) { resource.texHandle = texHandle; return *this; }
		DescriptorSetWrite& setBufferHandle(BufferHandle bufferHandle) { resource.bufferhandle = bufferHandle; return *this; }
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
		/*DescriptorSetLayoutsHandle descLayoutsHandle;*/
		std::vector<DescriptorSetLayoutsHandle> descLayoutsHandles;

		PipelineLayoutCreation& addDescLayouts(DescriptorSetLayoutsHandle handle) { descLayoutsHandles.push_back(handle); return *this; }
	};

	struct GraphicsPipelineCreation {
		struct ShaderInfo{
			std::string vertShaderPath;
			std::string fragShaderPath;
		} shaders;

		struct VertexInput {
			std::vector<VertexBindingDesc> bindingDesc;
			std::vector<VertexAttribute> attributes;
			
			VertexInput& addBindingDesc(VertexBindingDesc _bindingDesc) { bindingDesc.push_back(_bindingDesc); return *this; };
			VertexInput& addVertexAttribute(VertexAttribute attribute) { attributes.push_back(attribute); return *this; };
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

		GraphicsPipelineCreation& setShaderInfo(ShaderInfo shaderInfo) { shaders = shaderInfo; return *this; }
		GraphicsPipelineCreation& setVertexInput(VertexInput _vertexInput) { vertexInput = _vertexInput; return *this; }
		GraphicsPipelineCreation& setRasterizerInfo(RasterizerInfo _rasterInfo) { rasterInfo = _rasterInfo; return *this; }
		GraphicsPipelineCreation& setMSAAInfo(MSAAInfo _msaaInfo) { msaa = _msaaInfo; return *this; }
		GraphicsPipelineCreation& setDepthStencilInfo(DepthStencilInfo depthStencilInfo) { depthStencil = depthStencilInfo; return *this; }
		GraphicsPipelineCreation& setRenderPass(RenderPassHandle handle) { renderPassInfo.renderPassHandle = handle; return *this; }
		GraphicsPipelineCreation& setPipelineLayout(PipelineLayoutHandle handle) { pipelineLayoutHandle = handle; return *this; }
	};

	struct RayTracingShaderDesc {
		int groupId;
		std::string filePath;
		RayTracingShaderType shaderType;
	};

	struct RayTracingPipelineCreation {
		std::vector<RayTracingShaderDesc> shaders;
		PipelineLayoutHandle pipelineLayoutHandle = INVALID_PIPELINELAYOUT_HANDLE;
		int recur_depth = 1;

		RayTracingPipelineCreation& addShader(RayTracingShaderDesc shader) { shaders.push_back(shader); return *this; }
		RayTracingPipelineCreation& setPipelineLayout(PipelineLayoutHandle layoutHandle) { pipelineLayoutHandle = layoutHandle; return *this; }
		RayTracingPipelineCreation& setResursionDepth(int depth) { recur_depth = depth; return *this; }
	};

	struct RayTracingShaderBindingTableCreation {
		RayTracingPipelineHandle pipeline;
		int missCnt;
		int hitCnt;
	};

	struct RenderAttachmentInfo {
		DataFormat format;
		GraphResourceType resourceType;
	};

	struct RenderPassCreation {
		std::vector<RenderAttachmentInfo> attachmentInfos;

		RenderPassCreation& addAttachInfo(RenderAttachmentInfo info) { attachmentInfos.push_back(info); return *this; }
	};

	struct FramebufferCreation {
		std::vector<TextureHandle> attachments;
		RenderPassHandle renderPassHandle;
		u32 width;
		u32 height;
		u32 layers = 1;

		FramebufferCreation& addAttachment(TextureHandle tex) { attachments.push_back(tex); return *this; }
		FramebufferCreation& setRenderPass(RenderPassHandle renderpass) { renderPassHandle = renderpass; return *this; }
		FramebufferCreation& setSize(u32 _width, u32 _height) { width = _width; height = _height; return *this; }
		FramebufferCreation& setLayers(u32 nLayers) { layers = nLayers; return *this; }
	};

	// TODO: An optimized version for 2D data
	template<class T, class ResourceHandle>
	class ResourcePool {
	public:
		ResourcePool(u32 poolSize) {
			ASSERT(poolSize <= maxPoolSize, "required pool size exceeds maximum limit!");

			data.reserve(poolSize);
			for (u32 i = 0; i < poolSize; i++) {
				T t;
				data.push_back(t);
			}

			for (u32 i = 0; i < data.size(); i++) {
				freeList.push(i);
			}
		}

		~ResourcePool() {

		}

		ResourceHandle require_resource() {
			ASSERT(!freeList.empty(), "no available resource!");

			ResourceHandle handle = freeList.front();
			freeList.pop();
			return handle;
		}

		T& get_resource(ResourceHandle handle) {
			ASSERT(handle >= 0 && handle < data.size(), "invalid resource handle!");
			return data.at(handle);
		}

		void release_resource(ResourceHandle handle) {
			ASSERT(handle >= 0 && handle < data.size(), "invalid resource handle!");

			freeList.push(handle);
		}

	private:
		u32 poolSize;
		const u32 maxPoolSize = 512;
		std::vector<T> data;
		std::queue<ResourceHandle> freeList;
	};

}
