#pragma once
#include<vulkan/vulkan_core.h>
#include<string>
#include<vector>
#include"datatypes.h"
#include"enums.h"
#include"Resource.h"
#include"GPUDevice.h"
#include<unordered_map>
#include"Scene.h"

namespace zzcVulkanRenderEngine {
	typedef u32 GraphNodeHandle;

	struct ResourceInfo {
		union {
			struct {
				sizet size;
				BufferHandle bufferHandle;
			}buffer;

			struct {
				u32 width;
				u32 height;
				u32 depth;

				TextureType  textureType;
				// TODO: wrap following Vulkan enums for convenient usage by application programmers
				DataFormat format;
				//VkImageUsageFlags usage;
				VkAttachmentLoadOp loadOp;
				VkAttachmentStoreOp storeOp;
				TextureHandle texHandle;
			}texture;
		};
	};

	//u32 INVALID_BINDING = -1;
	struct GraphResource {
		bool isExternal=false;
		DescriptorSetLayoutsHandle externalDescLayouts;   // mandatory if isExternal=True, used to created the pipelinelayout for node

		GraphResourceType type;   
		ResourceInfo info;
		std::string key;           // used to uniquely identify a resource, note that 'final' represent the final output to display
		u16 groupId;               // specify which group the resource belongs to, typically determined by frequency of updating 
		//u16 binding = INVALID_BIND;               // specify which binding point to bound
		u16 binding = -1;
		ShaderStage accessStage = ShaderStage::DONT_CARE;   // specify which shader stage(s) will access this resource
	};

	struct GraphicsPipelineInfo {
		struct ShaderInfo {
			std::string vertShaderPath;
			std::string fragShaderPath;
		} shaders;

		struct VertexInput {
			std::vector<VertexBindingDesc> bindingDesc;
			std::vector<VertexAttribute> attributes;

			//VertexInput& setBindingDesc(VertexBindingDesc bindingDesc);
			//VertexInput& addVertexAttribute(VertexAttribute attribute);
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

		GraphicsPipelineInfo& setShaderInfo(ShaderInfo shaderInfo) { shaders = shaderInfo; return *this; }
		GraphicsPipelineInfo& setVertexInput(VertexInput _vertexInput) { vertexInput = _vertexInput; return *this;}
		GraphicsPipelineInfo& setRasterizerInfo(RasterizerInfo _rasterInfo) { rasterInfo = _rasterInfo; return *this; }
		GraphicsPipelineInfo& setMSAAInfo(MSAAInfo _msaaInfo) { msaa = _msaaInfo; return *this; }
		GraphicsPipelineInfo& setDepthStencilInfo(DepthStencilInfo depthStencilInfo) { depthStencil = depthStencilInfo; return *this; }
		GraphicsPipelineInfo& setPipelineLayout(PipelineLayoutHandle handle) { pipelineLayoutHandle = handle; return *this; }
	};
	
	struct ComputePipelineInfo {

	};

	// TODO: add raytracing pipeline
	struct GraphNode;
	struct NodeRender {
	public:
		// virtual methods to be overwritten by derived nodes
        // init() may include logic for creating buffer/image objects and registering handles for external input resources
        // (note that external resources like PBR textures, camera can be accessed by name and registered by handle, this is automatically done in compile())
         // execute() may include logic for binding pipeline/vertex/indices/descriptorsets
		virtual void init(GPUDevice* device) = 0;
		virtual void execute(CommandBuffer* cmdBuffer, GPUDevice* device, Scene* scene, GraphNode* node) = 0;
	};


	//Base class
	struct GraphNode {
	public:
		GraphNode() {

		}
		~GraphNode() {}
		// specified by the user
		GraphNodeType type = GraphNodeType::GRAPHICS;
		std::vector<GraphResource> inputs;
		std::vector<GraphResource> outputs;
		NodeRender* render = nullptr;

		// automatically generated
        // TODO: following resources should be considered as Resource managed by GPUDevice
		RenderPassHandle renderPass = INVALID_RENDERPASS_HANDLE;
		FramebufferHandle framebuffer = INVALID_FRAMEBUFFER_HANDLE;
		DescriptorSetLayoutsHandle descriptorSetLayouts = INVALID_DESCRIPTORSET_LAYOUTS_HANDLE;
		DescriptorSetsHandle descriptorSets = INVALID_DESCRIPTORSETS_HANDLE;
		PipelineLayoutHandle pipelineLayout = INVALID_PIPELINELAYOUT_HANDLE;

		union {
			struct {
				GraphicsPipelineInfo* pipelineInfo;
				GraphicsPipelineHandle pipelineHandle;
			}graphicPipeline;

			struct {
				ComputePipelineInfo* pipelineInfo;
				ComputePipelineHandle pipelineHandle;
			}computePipeline;
		}pipeline;

		// building helpers
		GraphNode& setType(GraphNodeType _type) {
			type = _type;
			return *this;
		}

		GraphNode& setInputs(std::vector<GraphResource> _inputs) {
			inputs = _inputs;
			return *this;
		}

		GraphNode& setOutputs(std::vector<GraphResource> _outputs) {
			outputs = _outputs;
			return *this;
		}

		GraphNode& setGraphicsPipelineInfo(GraphicsPipelineInfo* pipelineInfo) {
			pipeline.graphicPipeline.pipelineInfo = pipelineInfo;
			return *this;
		}

		GraphNode& setComputePipelineInfo(ComputePipelineInfo* pipelineInfo) {
			pipeline.computePipeline.pipelineInfo = pipelineInfo;
			return *this;
		}

		GraphNode& register_render(NodeRender* _render) {
			render = _render;
			return *this;
		}
		
		bool isComplete() {
			return render != nullptr;
		}
	};

	class RenderGraph {
	public:
		RenderGraph();
		~RenderGraph();
		void setDevice(GPUDevice* device);
		void addNode(GraphNode* node);
		void compile();
		void execute(CommandBuffer* cmdBuffer, GPUDevice* device, Scene* scene);
		TextureHandle& getTextureByKey(std::string key);
		BufferHandle& getBufferByKey(std::string key);
	private:
		GPUDevice* device;
		std::vector<GraphNode*> nodes;
		std::vector<std::vector<GraphNodeHandle>> graph;
		std::vector<GraphNodeHandle> topologyOrder;

		// TODO: moved to GPUDevice? not sure yet
		std::unordered_map<std::string, TextureHandle> key2TexMap;
		std::unordered_map<std::string, BufferHandle> key2BufferMap;
		
		void checkValidity();
		void buildGraph();
		void topologySort();
		void insert_barriers(CommandBuffer& cmdBuffer, GraphNode& node);

		//helper functions
		//void insert_barrier(VkCommandBuffer cmdBuffer, Texture& texture, VkAccessFlags newAccess);
	};
}
