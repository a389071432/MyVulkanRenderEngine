#pragma once
#include"Foundation/RenderGraph.h"
#include"Foundation/GPUDevice.h"


namespace zzcVulkanRenderEngine {
	struct ExampleNode : public GraphicsNode {
	public:
		// define extra variables
		struct Vertex {
			float position[3];
			float color[3];
		};

		// define the triangle
		std::vector<Vertex> vertices{
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};
		std::vector<uint32_t> indices{ 0, 1, 2 };

		BufferHandle vertexBuffer;
		BufferHandle indexBuffer;

		// overwrite the interfaces
		virtual void init(GPUDevice* device) {
			vertexBuffer = device->createBufferFromData(vertices);
			indexBuffer = device->createBufferFromData(indices);
		}

		virtual void execute(CommandBuffer* cmdBuffer, GPUDevice* device) {
			// Update dynamic viewport state
			VkViewport viewport{};
			viewport.height = (float)device->getSwapChainExtent().height;
			viewport.width = (float)device->getSwapChainExtent().width;
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
			// Update dynamic scissor state
			VkRect2D scissor{};
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(commandBuffers[currentBuffer], 0, 1, &scissor);
			// Bind descriptor set for the currrent frame's uniform buffer, so the shader uses the data from that buffer for this draw
			vkCmdBindDescriptorSets(commandBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &uniformBuffers[currentBuffer].descriptorSet, 0, nullptr);
			// Bind the rendering pipeline
			// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
			vkCmdBindPipeline(commandBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			// Bind triangle vertex buffer (contains position and colors)
			VkDeviceSize offsets[1]{ 0 };
			vkCmdBindVertexBuffers(commandBuffers[currentBuffer], 0, 1, &vertices.buffer, offsets);
			// Bind triangle index buffer
			vkCmdBindIndexBuffer(commandBuffers[currentBuffer], indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			// Draw indexed triangle
			vkCmdDrawIndexed(commandBuffers[currentBuffer], indices.count, 1, 0, 0, 1);
		}
	};
}

