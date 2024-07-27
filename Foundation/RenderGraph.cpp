#include<queue>
#include<map>
#include <algorithm>
#include"RenderGraph.h"
#include"assert.h"
#include"utils/utils.h"

namespace zzcVulkanRenderEngine {
	//// For the GraphicsPipelineInfo
	//GraphicsPipelineInfo& setShaderInfo(GraphicsPipelineInfo::ShaderInfo _shaderInfo) {

	//}

	//GraphicsPipelineInfo& setVertexInput(GraphicsPipelineInfo::VertexInput _vertexInput) {

	//}

	//GraphicsPipelineInfo& setRasterizerInfo(GraphicsPipelineInfo::RasterizerInfo _rasterInfo) {

	//}

	//GraphicsPipelineInfo& setMSAAInfo(GraphicsPipelineInfo::MSAAInfo _msaaInfo) {

	//}

	//GraphicsPipelineInfo& setDepthStencilInfo(GraphicsPipelineInfo::DepthStencilInfo depthStencilInfo) {

	//}

	//GraphicsPipelineInfo& setPipelineLayout(PipelineLayoutHandle pipelineLayoutHandle) {

	//}

	// For RenderGraph
	RenderGraph::RenderGraph() {

	}

	RenderGraph::~RenderGraph() {

	}

	void RenderGraph::setDevice(GPUDevice* _device) {
		device = _device;
	}

	void RenderGraph::addNode(GraphNode* node){
		nodes.push_back(node);
	}

	void RenderGraph::checkValidity() {
		// check for non-unique keys of otuput resources

		// for each node, check number of outputs matches the number of ColorBlendingStates

		// for each node, check number of inputs/outputs matches DescriptorSetLayoutsCreation.bindings

		// for each node, check its output attachments have the same size (required to create framebuffer)

		// check that the resolution of the final output matches the window

		// for each node, check whether the required external input resources exists
		// if exist, register the handle for that resource
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			for (GraphResource& r : node->inputs) {
				if (r.isExternal) {
					if (r.type == GraphResourceType::BUFFER) {
						auto it = key2BufferMap.find(r.key);
						ASSERT(it != key2BufferMap.end(), "Invalid external input resource (buffer)", r.key);
						r.info.buffer.bufferHandle = it->second;
					}
					else if (r.type == GraphResourceType::TEXTURE || r.type == GraphResourceType::DEPTH_MAP) {
						auto it = key2TexMap.find(r.key);
						ASSERT(it != key2TexMap.end(), "Invalid external input resource (texture)", r.key);
						r.info.texture.texHandle = it->second;
					}
				}
			}
		}
		
	}


	//add edges between nodes according to inputs and outputs
	void RenderGraph::buildGraph() {
		//check for validity of the graph 
		checkValidity();

		
		//a map used to record the producer of resources
		std::map<std::string, GraphNodeHandle> producer;
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			for (GraphResource& r : node->outputs) {
				producer.insert({ r.key,(GraphNodeHandle)i });
			}
		}

		//add edges
		graph.resize(nodes.size());
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			for (GraphResource& r : node->inputs) {
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
			GraphNode* node = nodes.at(index);
			for (GraphResource& r : node->inputs) {
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
			GraphNode* node = nodes.at(index);

			// Create resource on device and assign the obtained handle to GraphResource
			for (GraphResource& r : node->outputs) {
				ResourceInfo info = r.info;
				TextureCreation texCI;
				texCI.setType(info.texture.textureType)
					.setFormat(info.texture.format)
					.setSize(info.texture.width, info.texture.height, info.texture.depth)
					.setIsFinal(r.key == "final" ? true : false);

				if (!texFreelist.empty()) {      // meomry aliasing if applicable
					TextureHandle aliasTex = texFreelist.front();
					texFreelist.pop();
					texCI.setAliasTexture(aliasTex);
				}
				r.info.texture.texHandle = device->createTexture(texCI);
				key2TexMap.insert({ r.key,r.info.texture.texHandle });
			}

			// Register handles for input resources
			for (GraphResource& r : node->inputs) {
				r.info.texture.texHandle = getTextureByKey(r.key);
			}

			// TODO: repeat the same thing for buffer allocation
			
			// Update ref_count of resources and the freelist
			for (GraphResource& r : node->inputs) {
				ref_count[r.key]--;
				if (ref_count[r.key] == 0)
					texFreelist.push(getTextureByKey(r.key));
			}
		}

	   // TODO: STEP 2 (create descriptorSetLayouts for the node)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			if (!(node->inputs.size() > 0))
				continue;
			DescriptorSetLayoutsCreation layoutsCI{};
			layoutsCI.setNodeType(node->type);
			if (node->type == GraphNodeType::GRAPHICS) {
				for (GraphResource& r : node->inputs) {
					if (r.isExternal == false) {
						layoutsCI.addBinding({
	                          util_getBindingType(r.type,true),
	                          r.accessStage,
	                          r.groupId,
	                          r.binding
							});
					}
				}
				node->descriptorSetLayouts = device->createDescriptorSetLayouts(layoutsCI);
			}
			else {
				// TODO: repeat for Compute node
			}
		}

	   // TODO: STEP 2 (allocate descriptorSets for the node)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			if (!(node->inputs.size() > 0))
				continue;
			DescriptorSetsAlloc setsAllocInfo{};
			setsAllocInfo.layoutsHandle = node->descriptorSetLayouts;
			node->descriptorSets = device->createDescriptorSets(setsAllocInfo);
		}

	   // TODO: STEP 2 (write descriptorSets for the node)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			std::vector<DescriptorSetWrite> writes;
			if (node->type == GraphNodeType::GRAPHICS) {
				for (GraphResource& r : node->inputs) {
					DescriptorSetWrite write{};
					write.setType(util_getBindingType(r.type, true))
						.setDstSet(r.groupId)
						.setDstBinding(r.binding)
						.setBufferHandle(r.type == GraphResourceType::BUFFER ? r.info.buffer.bufferHandle : INVALID_BUFFER_HANDLE)
						.setTexHandle(r.type == GraphResourceType::TEXTURE ? r.info.texture.texHandle : INVALID_TEXTURE_HANDLE);
					writes.push_back(write);
				}
				if (node->inputs.size() > 0) {
					device->writeDescriptorSets(writes, node->descriptorSets);
				}
			}
			else {
				// TODO: repeat for Compute node
			}
		}

       // TODO: STEP 3 (create render pass for graphic nodes)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			RenderPassCreation creation{};
			if (node->type == GraphNodeType::GRAPHICS) {
				for (GraphResource& r : node->outputs) {
					if (r.type == GraphResourceType::TEXTURE || r.type == GraphResourceType::DEPTH_MAP) {
						creation.addAttachInfo({ r.info.texture.format,r.type });
					}
				}
				node->typeData->graphics.renderPass = device->createRenderPass(creation);
			}
		}

       // TODO: STEP 4 (create framebuffer for graphics nodes)
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			FramebufferCreation creation{};
			if (node->type == GraphNodeType::GRAPHICS) {
				u32 width = -1;
				u32 height = -1;
				for (GraphResource& r : node->outputs) {
					if (r.type == GraphResourceType::TEXTURE || r.type == GraphResourceType::DEPTH_MAP) {
						creation.addAttachment({ r.info.texture.texHandle });
						width = r.info.texture.width;
						height = r.info.texture.height;
					}
				}
				creation.setLayers(1);
				creation.setRenderPass(node->typeData->graphics.renderPass);
				creation.setSize(width,height);
				node->typeData->graphics.framebuffer = device->createFramebuffer(creation);
			}
		}

	    // TODO: STEP 2 (create pipelineLayout for the node)
		// NOTE: need to concatenate multiple descriptor sets from internal and external resources
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			PipelineLayoutCreation layoutCI{};
			// descLayout for internal resources
			if (node->descriptorSetLayouts != INVALID_DESCRIPTORSET_LAYOUTS_HANDLE) {
				layoutCI.addDescLayouts(node->descriptorSetLayouts);
			}
			// descLayout for external resources
			for (GraphResource& r : node->inputs) {
				if (r.isExternal) {
					ASSERT(
						r.externalDescLayouts != INVALID_DESCRIPTORSET_LAYOUTS_HANDLE,
						"Assertion failed: descriptorSet layout is not defined for external resource!"
					);
					layoutCI.addDescLayouts(r.externalDescLayouts);
				}
			}
			node->pipelineLayout = device->createPipelineLayout(layoutCI);
		}

	    // create pipeline for the node
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			if (node->type == GraphNodeType::GRAPHICS) {
				GraphicsPipelineCreation gPipelineCI{};
				GraphicsPipelineCreation ci{};
				GraphicsPipelineInfo* pipeInfo = node->typeData->graphics.pipelineInfo;
				ci.setShaderInfo({ pipeInfo->shaders.vertShaderPath, pipeInfo->shaders.fragShaderPath })
					.setVertexInput({ pipeInfo->vertexInput.bindingDesc, pipeInfo->vertexInput.attributes })
					.setRasterizerInfo({ pipeInfo->rasterInfo.cullMode, pipeInfo->rasterInfo.frontFace })
					.setMSAAInfo({ pipeInfo->msaa.nSamplesPerPixel })
					.setDepthStencilInfo({ pipeInfo->depthStencil.enableDepth })
					.setRenderPass(node->typeData->graphics.renderPass)
					.setPipelineLayout(node->pipelineLayout);
				node->typeData->graphics.pipelineHandle = device->createGraphicsPipeline(ci);
			}
			else if (node->type == GraphNodeType::RAYTRACING) {
				RayTracingPipelineCreation rtPipelineCI{};
				RayTracingPipelineInfo* pipeInfo = node->typeData->raytracing.pipelineInfo;
				rtPipelineCI.setShaders(pipeInfo->shaders)
					.setResursionDepth(pipeInfo->recur_depth)
					.setPipelineLayout(node->pipelineLayout);
				node->typeData->raytracing.pipelineHandle = device->createRayTracingPipeline(rtPipelineCI);
			}
		}

		// create shader binding table for raytracing node
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			if (node->type == GraphNodeType::RAYTRACING) {
				RayTracingShaderBindingTableCreation sbtCI{};
				sbtCI.setPipeline(node->typeData->raytracing.pipelineHandle)
					.setShaderCounts(node->typeData->raytracing.pipelineInfo->missCnt, node->typeData->raytracing.pipelineInfo->hitCnt);
				node->typeData->raytracing.sbt = device->createRayTracingShaderBindingTable(
					sbtCI,
					&node->typeData->raytracing.rgenShaderRegion,
					&node->typeData->raytracing.missShaderRegion,
					&node->typeData->raytracing.hitShaderRegion
				);
			}
		}

		// init all nodes
		for (u32 i = 0; i < nodes.size(); i++) {
			GraphNode* node = nodes.at(i);
			node->render->init(device);
		}

	}

	void RenderGraph::execute(CommandBuffer* cmdBuffer, GPUDevice* device, Scene* scene) {
		for (u32 i = 0; i < topologyOrder.size(); i++) {
			u32 index = topologyOrder.at(i);
			GraphNode* node = nodes.at(index);

			// Insert barriers for both input and output resources
			insert_barriers(*cmdBuffer, *node);

			// Begin the render pass
			std::vector<VkClearValue> clearValues;
			clearValues.resize(node->outputs.size());
			for (int i = 0; i < node->outputs.size();i++) {
				GraphResource& r = node->outputs[i];
				if (r.type == GraphResourceType::TEXTURE) {
					clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				}
				else if (r.type == GraphResourceType::DEPTH_MAP) {
					clearValues[3].depthStencil = { 1.0f, 0 };
				}
			}

			VkRenderPassBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			beginInfo.renderPass = device->getRenderPass(node->typeData->graphics.renderPass);
			beginInfo.framebuffer = device->getFramebuffer(node->typeData->graphics.framebuffer);
			beginInfo.renderArea.extent.width = node->outputs[0].info.texture.width;
			beginInfo.renderArea.extent.height = node->outputs[0].info.texture.height;
			beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			beginInfo.pClearValues = clearValues.data();
			cmdBuffer->cmdBeginRenderPass(beginInfo);

			// Execute the node using user-defined method
			node->render->execute(cmdBuffer,device,scene,node);

			//End the render pass
			cmdBuffer->cmdEndRenderPass();
		}
	}

	void RenderGraph::insert_barriers(CommandBuffer& cmdBuffer, GraphNode& node) {
		if (node.type == GraphNodeType::GRAPHICS) {
			// add barrier for input resources
			for (GraphResource& r : node.inputs) {
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::READ_TEXTURE, 0);
					texture.setAccessType(GraphResourceAccessType::READ_TEXTURE,0,1);    // necessary to track the state of texture to determine initialLayout
				}
				else if (r.type == GraphResourceType::BUFFER) {
					
				}
			}

			// add barrier for output resources
			for (GraphResource& r : node.outputs) {
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::WRITE_ATTACHMENT, 0);
					texture.setAccessType(GraphResourceAccessType::WRITE_ATTACHMENT,0,1);
				}
				else if (r.type == GraphResourceType::DEPTH_MAP) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::WRITE_DEPTH, 0);
					texture.setAccessType(GraphResourceAccessType::WRITE_DEPTH, 0,1);
				}
				else if (r.type == GraphResourceType::BUFFER) {

				}
			}
		}
		else if (node.type == GraphNodeType::COMPUTE) {
			// add barrier for input resources
			for (GraphResource& r : node.inputs) {
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::COMPUTE_READ_STORAGE_IMAGE, 0);
					texture.setAccessType(GraphResourceAccessType::COMPUTE_READ_STORAGE_IMAGE, 0, 1);    // necessary to track the state of texture to determine initialLayout
				}
				else if (r.type == GraphResourceType::BUFFER) {

				}
			}

			// add barrier for output resources
			for (GraphResource& r : node.outputs) {
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::COMPUTE_READ_WRITE_STORAGE_IMAGE, 0);
					texture.setAccessType(GraphResourceAccessType::COMPUTE_READ_WRITE_STORAGE_IMAGE, 0, 1);
				}
				else if (r.type == GraphResourceType::BUFFER) {

				}
			}

		}else if (node.type == GraphNodeType::RAYTRACING) {
			// add barrier for input resources
			for (GraphResource& r : node.inputs) {
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::RAYTRACING_READ_STORAGE_IMAGE, 0);
					texture.setAccessType(GraphResourceAccessType::RAYTRACING_READ_STORAGE_IMAGE, 0, 1);    // necessary to track the state of texture to determine initialLayout
				}
				else if (r.type == GraphResourceType::BUFFER) {

				}
			}

			// add barrier for output resources
			for (GraphResource& r : node.outputs) {
				Texture& texture = device->getTexture(r.info.texture.texHandle);
				if (r.type == GraphResourceType::TEXTURE) {
					cmdBuffer.cmdInsertImageBarrier(texture, GraphResourceAccessType::RAYTRACING_READ_WRITE_STORAGE_IMAGE, 0);
					texture.setAccessType(GraphResourceAccessType::RAYTRACING_READ_WRITE_STORAGE_IMAGE, 0, 1);
				}
				else if (r.type == GraphResourceType::BUFFER) {

				}
			}

		}
	}

	TextureHandle& RenderGraph::getTextureByKey(std::string key) {
		auto it = key2TexMap.find(key);
		ASSERT(it != key2TexMap.end(), "Assertion failed: invalid key for texture");
		return it->second;
	}
}


