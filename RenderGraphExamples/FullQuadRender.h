#pragma once
#include"Foundation/RenderGraph.h"
#include"Foundation/GPUDevice.h"
#include<array>

namespace zzcVulkanRenderEngine {
	struct FullQuadRender : public NodeRender {
	public:

		// overwrite the interfaces
		void init(GPUDevice* device) override {
		}

		void execute(CommandBuffer* cmdBuffer, GPUDevice* device, Scene* scene, GraphNode* node) override {
			// Update dynamic viewport state
			cmdBuffer->cmdSetViewport((float)device->getSwapChainExtent().width, (float)device->getSwapChainExtent().height);

			// Update dynamic scissor state
			cmdBuffer->cmdSetScissor(device->getSwapChainExtent().width, (float)device->getSwapChainExtent().height, 0, 0);

			// Bind the pipeline
			cmdBuffer->cmdBindGraphicsPipeline(device->getGraphicsPipeline(node->typeData->graphics.pipelineHandle));

			// Bind the descriptor set for texture sampling
			cmdBuffer->cmdBindDescriptorSets(
				PipelineBindPoint::GRAPHICS,
				device->getPipelineLayout(node->typeData->graphics.pipelineHandle),
				device->getDescriptorSets(node->descriptorSets)
			);

			// Draw full quad
			cmdBuffer->cmdDrawFullQuad();
		}
	};
}
