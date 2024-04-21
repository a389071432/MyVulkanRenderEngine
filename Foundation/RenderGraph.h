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

	struct ResourceDesc {
		union {
			struct {
				sizet size;
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
			}texture;
		};
	};

	struct GraphResource {
		bool isExternal;
		GraphResourceType type;   
		ResourceDesc info;
		std::string key;     //used to uniquely identify a resource
		ResourceHandle texture;
	};

	struct GraphNode {
		GraphNodeType type;
		std::vector<GraphResource> inputs;
		std::vector<GraphResource> outputs;
		VkRenderPass renderPass;
		VkFramebuffer framebuffer;

		GraphNode& setType(GraphNodeType type);
		GraphNode& setInputs(std::vector<GraphResource> inputs);
		GraphNode& setOutputs(std::vector<GraphResource> outputs);
		virtual void execute();
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
		void buildGraph();
		void topologySort();
		GraphResource& getResource(std::string key);
		void insert_barriers(CommandBuffer& cmdBuffer, GraphNode& node);

		//helper functions
		//void insert_barrier(VkCommandBuffer cmdBuffer, Texture& texture, VkAccessFlags newAccess);
	};
}
