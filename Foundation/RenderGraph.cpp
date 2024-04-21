#include<queue>
#include<map>
#include <algorithm>
#include"RenderGraph.h"
#include"assert.h"
#include"utils/utils.h"

namespace zzcVulkanRenderEngine {
	// For GraphNode
	GraphNode& GraphNode::setType(GraphNodeType _type) {
		type = _type;
		return *this;
	}

	GraphNode& GraphNode::setInputs(std::vector<GraphResource> _inputs) {
		inputs = _inputs;
		return *this;
	}

	GraphNode& GraphNode::setOutputs(std::vector<GraphResource> _outputs) {
		outputs = _outputs;
		return *this;
	}

	// For RenderGraph
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
			for (GraphResource& r : node.outputs) {
				producer.insert({ r.key,(GraphNodeHandle)i });
			}
		}

		//add edges
		graph.resize(nodes.size());
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode& node = nodes.at(i);
			for (GraphResource& r : node.inputs) {
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
			for (GraphResource& r : node.inputs) {
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
		std::queue<ResourceHandle>freelist;
		for (u32 i = 0; i < topologyOrder.size(); i++) {
			u32 index = topologyOrder.at(i);
			GraphNode& node = nodes.at(index);

			// TODO: create resource on device and assign the obtained handle to GraphResource
			for (GraphResource& r : node.outputs) {
				if (!freelist.empty()) {
					ResourceHandle freeTex = freelist.front();
					freelist.pop();
					device->createTexture(r.texture,freeTex);
				}
				else {
					device->createTexture(r.texture);
				}
			}
			
			//update ref_count of resources and the freelist
			for (GraphResource& r : node.inputs) {
				ref_count[r.key]--;
				if (ref_count[r.key] == 0)
					freelist.push(getResource(r.key).texture);
			}

			//TODO: create render pass and framebuffer for the node

		}


	}

	void RenderGraph::execute(CommandBuffer& cmdBuffer) {
		for (u32 i = 0; i < topologyOrder.size(); i++) {
			u32 index = topologyOrder.at(i);
			GraphNode& node = nodes.at(index);

			// Insert barriers for both input and output resources
			insert_barriers(cmdBuffer, node);

			// Begin the render pass
			cmdBuffer.cmdBeginRenderPass(node.renderPass, node.framebuffer);

			// Execute the node using user-defined method
			node.execute();

			//End the render pass
			cmdBuffer.cmdEndRenderPass();
		}
	}

	void RenderGraph::insert_barriers(CommandBuffer& cmdBuffer, GraphNode& node) {
		if (node.type == GraphNodeType::GRAPHICS) {
			// add barrier for input resources
			for (GraphResource& r : node.inputs) {
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(device->getTexture(r.texture), GraphResourceAccessType::READ_TEXTURE, 0, 1);
				}
				else if (r.type == GraphResourceType::BUFFER) {
					
				}
			}

			// add barrier for output resources
			for (GraphResource& r : node.outputs) {
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(device->getTexture(r.texture), GraphResourceAccessType::WRITE_ATTACHMENT, 0, 1);
				}
				else if (r.type == GraphResourceType::DEPTH_MAP) {
					cmdBuffer.cmdInsertImageBarrier(device->getTexture(r.texture), GraphResourceAccessType::WRITE_DEPTH, 0, 1);
				}
				else if (r.type == GraphResourceType::BUFFER) {

				}
			}
		}
		else if (node.type == GraphNodeType::COMPUTE) {
			//TODO: add barriers for a compute node

		}
	}
}


