#pragma once
#include<vulkan/vulkan_core.h>
#include<string>
#include<vector>
#include"datatypes.h"
#include"enums.h"
#include"Resource.h"
#include"GPUDevice.h"

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
				VkFormat format;
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
		GraphResourceType type;   
		ResourceInfo info;
		std::string key;           // used to uniquely identify a resource
		u32 groupId;               // specify which group the resource belongs to, typically determined by frequency of updating 
		u32 binding=INVALID_BINDING;               // specify which binding point to bound
		ShaderStage accessStage;   // specify which shader stage(s) will access this resource
	};

	struct GraphNode {
		// specified by the user
		GraphNodeType type;
		std::vector<GraphResource> inputs;
		std::vector<GraphResource> outputs;

		// building helpers
		GraphNode& setType(GraphNodeType type);
		GraphNode& setInputs(std::vector<GraphResource> inputs);
		GraphNode& setOutputs(std::vector<GraphResource> outputs);
		virtual void execute();

		// automatically generated
		// TODO: following resources should be considered as Resource managed by GPUDevice
		RenderPassHandle renderPass;
		FramebufferHandle framebuffer;
		DescriptorSetLayoutsHandle descriptorSetLayout;
		DescriptorSetsHandle descriptorSets;
	};

	class RenderGraph {
	public:
		RenderGraph();
		~RenderGraph();
		void addNode(GraphNode node);
		void compile();
		void execute(CommandBuffer& cmdBuffer);
	private:
		GPUDevice* device;
		std::vector<GraphNode> nodes;
		std::vector<std::vector<GraphNodeHandle>> graph;
		std::vector<GraphNodeHandle> topologyOrder;
		std::map<std::string, TextureHandle> key2TexMap;
		std::map<std::string, BufferHandle> key2BufferMap;
		
		void buildGraph();
		void topologySort();
		TextureHandle& getTextureByKey(std::string key);
		BufferHandle& getBufferByKey(std::string key);
		void insert_barriers(CommandBuffer& cmdBuffer, GraphNode& node);

		//helper functions
		//void insert_barrier(VkCommandBuffer cmdBuffer, Texture& texture, VkAccessFlags newAccess);
	};
}
