#include"RenderGraph.h"
#include"assert.h"
#include<queue>
#include<map>
#include <algorithm>

namespace zzcVulkanRenderEngine {
	RenderGraph::RenderGraph() {

	}

	RenderGraph::~RenderGraph() {

	}

	void RenderGraph::addNode(GraphNode node){
		nodes.push_back(node);
	}

	//add edges between nodes according to inputs and outputs
	void RenderGraph::buildGraph() {
		//check for non-unique keys of resources


		
		//a map used to record the producer of resources
		std::map<std::string, GraphNodeHandle> producer;
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode& node = nodes.at(i);
			for (GraphResource& r : node.output) {
				producer.insert({ r.key,(GraphNodeHandle)i });
			}
		}

		//add edges
		graph.resize(nodes.size());
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode& node = nodes.at(i);
			for (GraphResource& r : node.input) {
				if (r.isExternal)
					continue;
				auto it = producer.find(r.key);
				ASSERT(it != producer.end(), "invalid input resource for node");
				GraphNodeHandle pNode = it->second;
				std::vector<GraphNodeHandle>& edgeList = graph.at(pNode);
				if (std::find(edgeList.begin(), edgeList.end(), GraphNodeHandle(i)) != edgeList.end())
					continue;
				edgeList.push_back(GraphNodeHandle(i));
			}
		}
	}

	void RenderGraph::topologySort() {
		//compute degree for each node
		std::vector<u32>degree(nodes.size());
		for (u32 i = 0; i < graph.size(); i++) {
			std::vector<GraphNodeHandle>& edgeList = graph.at(i);
			for (u32 j = 0; j < edgeList.size(); j++) {
				degree[edgeList.at(j)]++;
			}
		}

		//topological sort by BFS
		std::queue<GraphNodeHandle> q;
		for (u32 i = 0; i < nodes.size(); i++) {
			if (degree.at(i) == 0) {
				q.push(i);
			}
		}
		while (!q.empty()) {
			GraphNodeHandle& nodeIndex = q.front();
			q.pop();
			topologyOrder.push_back(nodeIndex);
			std::vector<GraphNodeHandle>& edgeList = graph.at(nodeIndex);
			for (u32 j = 0; j < edgeList.size(); j++) {
				u32 succNode = edgeList.at(j);
				degree[succNode]--;
				if (degree[succNode] == 0) {
					q.push(edgeList.at(j));
				}	
			}
		}
	}

	void RenderGraph::compile() {
		buildGraph();
		topologySort();

		//count number of reference for each resource
		std::map<std::string, u32>ref_count;
		for (u32 i = 0; i < topologyOrder.size(); i++) {
			u32 index = topologyOrder.at(i);
			GraphNode& node = nodes.at(index);
			for (GraphResource& r : node.input) {
				auto it = ref_count.find(r.key);
				if (it == ref_count.end()) {
					ref_count.insert({ r.key,0 });
				}
				else {
					it->second++;
				}
			}
		}

		//allocate memory on device for resources, using technique of memory aliasing to efficiently reuse memory 
		std::queue<GraphResource&>freelist;
		for (u32 i = 0; i < topologyOrder.size(); i++) {
			u32 index = topologyOrder.at(i);
			GraphNode& node = nodes.at(index);

			// TODO: create resource on device and assign the obtained handle to GraphResource
			for (GraphResource& r : node.output) {
				if (!freelist.empty()) {

				}
				else {

				}
			}
			
			//update ref_count of resources and the freelist
			for (GraphResource& r : node.input) {
				ref_count[r.key]--;
				if (ref_count[r.key] == 0)
					freelist.push(getResource(r.key));
			}

			//TODO: create render pass and framebuffer for the node

		}


	}

	void RenderGraph::execute() {
		for (u32 i = 0; i < topologyOrder.size(); i++) {
			u32 index = topologyOrder.at(i);
			GraphNode& node = nodes.at(index);

			//insert barrier for both input and output resources
			//insert_barriers(, node);
		}
	}

	void RenderGraph::insert_barriers(VkCommandBuffer cmdBuffer, GraphNode& node) {
		if (node.type == GRAPHICS) {
			// add barrier for input resources
			for (GraphResource& r : node.input) {
				if (r.type == TEXTURE_TO_SAMPLE) {

				}
				else if (r.type == BUFFER) {

				}
			}

			// add barrier for output resources
			for (GraphResource& r : node.input) {

			}
		}
		else if (node.type == COMPUTE) {
			//TODO: add barriers for a compute node

		}
	}
}


