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

	void RenderGraph::checkValidity() {
		// check for non-unique keys of resources

		// for each node, check number of outputs matches the number of ColorBlendingStates

		// for each node, check number of inputs/outputs matches DescriptorSetLayoutsCreation.bindings
	}


	//add edges between nodes according to inputs and outputs
	void RenderGraph::buildGraph() {
		//check for validity of the graph 


		
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
		
		// STEP 0: CREATE RESOURCES ON DEVICE
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

		// allocate memory on device for resources, using technique of memory aliasing to efficiently reuse memory 
		std::queue<TextureHandle> texFreelist;
		for (u32 i = 0; i < topologyOrder.size(); i++) {
			u32 index = topologyOrder.at(i);
			GraphNode& node = nodes.at(index);

			// Create resource on device and assign the obtained handle to GraphResource
			for (GraphResource& r : node.outputs) {
				ResourceInfo info = r.info;
				TextureCreation texCI;
				texCI.setType(info.texture.textureType)
					.setFormat(info.texture.format)
					.setSize(info.texture.width, info.texture.height, info.texture.depth);

				if (!texFreelist.empty()) {      // meomry aliasing if applicable
					TextureHandle aliasTex = texFreelist.front();
					texFreelist.pop();
					texCI.setAliasTexture(aliasTex);
				}
				r.info.texture.texHandle = device->createTexture(texCI);
				key2TexMap.insert({ r.key,r.info.texture.texHandle });
			}

			// Register handles for input resources
			for (GraphResource& r : node.inputs) {
				r.info.texture.texHandle = getTextureByKey(r.key);
			}

			// TODO: repeat the same thing for buffer allocation
			
			// Update ref_count of resources and the freelist
			for (GraphResource& r : node.inputs) {
				ref_count[r.key]--;
				if (ref_count[r.key] == 0)
					texFreelist.push(getTextureByKey(r.key));
			}
		}

	   // TODO: STEP 2 (create descriptorSetLayouts for the node)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode& node = nodes.at(i);
			DescriptorSetLayoutsCreation layoutsCI{};
			layoutsCI.setNodeType(node.type);
			if (node.type == GraphNodeType::GRAPHICS) {
				for (GraphResource& r : node.inputs) {
					layoutsCI.addBinding({
						util_getBindingType(r.type,true),
						r.accessStage,
						r.groupId,
						r.binding
						});
				}
				node.descriptorSetLayouts = device->createDescriptorSetLayouts(layoutsCI);
			}
			else {
				// TODO: repeat for Compute node
			}
		}

	   // TODO: STEP 2 (allocate descriptorSets for the node)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode& node = nodes.at(i);
			DescriptorSetsAlloc setsAllocInfo{};
			setsAllocInfo.layoutsHandle = node.descriptorSetLayouts;
			node.descriptorSets = device->createDescriptorSets(setsAllocInfo);
		}

	   // TODO: STEP 2 (write descriptorSets for the node)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode& node = nodes.at(i);
			std::vector<DescriptorSetWrite> writes;
			if (node.type == GraphNodeType::GRAPHICS) {
				for (GraphResource& r : node.inputs) {
					DescriptorSetWrite write{};
					write.setType(util_getBindingType(r.type, true))
						.setDstSet(r.groupId)
						.setDstBinding(r.binding)
						.setBufferHandle(r.type == GraphResourceType::BUFFER ? r.info.buffer.bufferHandle : INVALID_BUFFER_HANDLE)
						.setTexHandle(r.type == GraphResourceType::TEXTURE ? r.info.texture.texHandle : INVALID_TEXTURE_HANDLE);
					writes.push_back(write);
				}
				device->writeDescriptorSets(writes, node.descriptorSets);
			}
			else {
				// TODO: repeat for Compute node
			}
		}

       // TODO: STEP 3 (create render pass for the node)


       // TODO: STEP 4 (create framebuffer for the node)

	   // TODO: STEP 2 (create pipelineLayout for the node)

	   // TODO: STEP 2 (create pipeline for the node)

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
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::READ_TEXTURE, 0, 1);
					texture.setAccessType(GraphResourceAccessType::READ_TEXTURE);    // necessary to track the state of texture to determine initialLayout
				}
				else if (r.type == GraphResourceType::BUFFER) {
					
				}
			}

			// add barrier for output resources
			for (GraphResource& r : node.outputs) {
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::WRITE_ATTACHMENT, 0, 1);
					texture.setAccessType(GraphResourceAccessType::WRITE_ATTACHMENT);
				}
				else if (r.type == GraphResourceType::DEPTH_MAP) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::WRITE_DEPTH, 0, 1);
					texture.setAccessType(GraphResourceAccessType::WRITE_DEPTH);
				}
				else if (r.type == GraphResourceType::BUFFER) {

				}
			}
		}
		else if (node.type == GraphNodeType::COMPUTE) {
			// TODO: add barriers for a compute node

		}
	}

	TextureHandle& RenderGraph::getTextureByKey(std::string key) {
		auto it = key2TexMap.find(key);
		ASSERT(it != key2TexMap.end(), "Assertion failed: invalid key for texture");
		return it->second;
	}
}


