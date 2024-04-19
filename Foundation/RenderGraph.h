#pragma once
#include<vulkan/vulkan_core.h>
#include<string>
#include<vector>
#include"datatypes.h"
#include"enums.h"

namespace zzcVulkanRenderEngine {
	typedef u32 GraphNodeHandle;

	struct ResourceInfo {
		union {
			struct {
				sizet size;
			}buffer;

			struct {
				u32 width;
				u32 height;
				u32 depth;
				VkFormat format;
				VkImageUsageFlags usage;
				VkAttachmentLoadOp loadOp;
				VkAttachmentStoreOp storeOp;
			}texture;
		};
	};

	struct GraphResource {
		bool isExternal;
		GraphResourceType type;   
		ResourceInfo info;
		std::string key;     //used to uniquely identify a resource
		//Texture* texture;
	};

	struct GraphNode {
		GraphNodeType type;
		std::vector<GraphResource> input;
		std::vector<GraphResource> output;
		VkRenderPass renderPass;
		VkFramebuffer framebuffer;
	};

	class RenderGraph {
	public:
		RenderGraph();
		~RenderGraph();
		void addNode(GraphNode node);
		void compile();
		void execute();
	private:
		std::vector<GraphNode> nodes;
		std::vector<std::vector<GraphNodeHandle>> graph;
		std::vector<GraphNodeHandle> topologyOrder;
		void buildGraph();
		void topologySort();
		GraphResource& getResource(std::string key);
		void insert_barriers(VkCommandBuffer cmdBuffer, GraphNode& node);

		//helper functions
		//void insert_barrier(VkCommandBuffer cmdBuffer, Texture& texture, VkAccessFlags newAccess);
	};
}
