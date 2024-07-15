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

	u32 INVALID_BINDING = -1;
	struct GraphResource {
		bool isExternal=false;
		DescriptorSetLayoutsHandle externalDescLayouts;   // mandatory if isExternal=True, used to created the pipelinelayout for node

		GraphResourceType type;   
		ResourceInfo info;
		std::string key;           // used to uniquely identify a resource, note that 'final' represent the final output to display
		u16 groupId;               // specify which group the resource belongs to, typically determined by frequency of updating 
		u16 binding = INVALID_BINDING;               // specify which binding point to bound
		//ShaderStage accessStage = ShaderStage::DONT_CARE;   // specify which shader stage(s) will access this resource
		ShaderStage accessStage = ShaderStage::DONT_CARE;   // specify which shader stage(s) will access this resource
	};

	struct GraphicsPipelineInfo {
		struct ShaderInfo {
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

		GraphicsPipelineInfo& setShaderInfo(ShaderInfo shaderInfo);
		GraphicsPipelineInfo& setVertexInput(VertexInput vertexInput);
		GraphicsPipelineInfo& setRasterizerInfo(RasterizerInfo rasterInfo);
		GraphicsPipelineInfo& setMSAAInfo(MSAAInfo msaaInfo);
		GraphicsPipelineInfo& setDepthStencilInfo(DepthStencilInfo depthStencilInfo);
		GraphicsPipelineInfo& setPipelineLayout(PipelineLayoutHandle pipelineLayoutHandle);
	};
	
	struct ComputePipelineInfo {

	};

	// TODO: add raytracing pipeline


	//Base class
	struct GraphNodeBase {
	public:
		GraphNodeType type;
		std::vector<GraphResource> inputs;
		std::vector<GraphResource> outputs;

		// automatically generated
        // TODO: following resources should be considered as Resource managed by GPUDevice
		RenderPassHandle renderPass;
		FramebufferHandle framebuffer;
		DescriptorSetLayoutsHandle descriptorSetLayouts;
		DescriptorSetsHandle descriptorSets;
		PipelineLayoutHandle pipelineLayout;

		// virtual methods to be overwritten by derived nodes
        // init() may include logic for creating buffer/image objects and registering handles for external input resources
        // (note that external resources like PBR textures, camera can be accessed by name and registered by handle, this is automatically done in compile())
        // execute() may include logic for binding pipeline/vertex/indices/descriptorsets
		virtual void init(GPUDevice* device)=0;
		virtual void execute(CommandBuffer* cmdBuffer, GPUDevice* device, Scene* scene)=0;
		
		// for type determination
		virtual void asGraphics();
	};

	// Use the  Curiously Recurring Template Pattern (CRTP)
	template<typename Derived>
	struct GraphNode: virtual public GraphNodeBase {
	public:
		//// specified by the user
		//GraphNodeType type;
		//std::vector<GraphResource> inputs;
		//std::vector<GraphResource> outputs;

		// building helpers
		Derived& setType(GraphNodeType type);
		Derived& setInputs(std::vector<GraphResource> inputs);
		Derived& setOutputs(std::vector<GraphResource> outputs);

		//// virtual methods to be overwritten by derived nodes
		//// init() may include logic for creating buffer/image objects and registering handles for external input resources
		//// (note that external resources like PBR textures, camera can be accessed by name and registered by handle, this is automatically done in compile())
		//// execute() may include logic for binding pipeline/vertex/indices/descriptorsets
		//virtual void init(GPUDevice* device);
		//virtual void execute(CommandBuffer* cmdBuffer, GPUDevice* device);


		//// automatically generated
		//// TODO: following resources should be considered as Resource managed by GPUDevice
		//RenderPassHandle renderPass;
		//FramebufferHandle framebuffer;
		//DescriptorSetLayoutsHandle descriptorSetLayouts;
		//DescriptorSetsHandle descriptorSets;
		//PipelineLayoutHandle pipelineLayout;
	};

	struct GraphicsNodeBase : virtual public GraphNodeBase {
	public:
		GraphicsPipelineInfo pipelineInfo;
		GraphicsPipelineHandle pipelineHandle;
	};

	template<typename Derived>
	struct GraphicsNode : public GraphNode<Derived>, public GraphicsNodeBase {
	public:
		//GraphicsPipelineInfo pipelineInfo;
		//GraphicsPipelineHandle pipelineHandle;

		Derived& setPipelineInfo(GraphicsPipelineInfo info);
	};

	struct ComputeNode : public GraphNode<ComputeNode> {
	public:
		ComputePipelineInfo pipelineInfo;
		ComputePipelineHandle pipelineHandle;
	};

	class RenderGraph {
	public:
		RenderGraph();
		~RenderGraph();
		void addNode(GraphNodeBase* node);
		void compile();
		void execute(CommandBuffer& cmdBuffer);
		TextureHandle& getTextureByKey(std::string key);
		BufferHandle& getBufferByKey(std::string key);
	private:
		GPUDevice* device;
		std::vector<GraphNodeBase*> nodes;
		std::vector<std::vector<GraphNodeHandle>> graph;
		std::vector<GraphNodeHandle> topologyOrder;

		// TODO: moved to GPUDevice? not sure yet
		std::unordered_map<std::string, TextureHandle> key2TexMap;
		std::unordered_map<std::string, BufferHandle> key2BufferMap;
		
		void checkValidity();
		void buildGraph();
		void topologySort();
		void insert_barriers(CommandBuffer& cmdBuffer, GraphNodeBase& node);

		//helper functions
		//void insert_barrier(VkCommandBuffer cmdBuffer, Texture& texture, VkAccessFlags newAccess);
	};
}
