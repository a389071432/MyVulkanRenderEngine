#pragma once
#include"Foundation/RenderGraph.h"
#include"Foundation/GPUDevice.h"
#include<array>


namespace zzcVulkanRenderEngine {
	struct ExampleNode : public GraphicsNode<ExampleNode> {
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
		void init(GPUDevice* device) override {
			vertexBuffer = device->createBufferFromData(vertices);
			indexBuffer = device->createBufferFromData(indices);
		}

		void execute(CommandBuffer* cmdBuffer, GPUDevice* device, Scene* scene) override {
			// Update dynamic viewport state
			cmdBuffer->cmdSetViewport((float)device->getSwapChainExtent().height, (float)device->getSwapChainExtent().width);

			// Update dynamic scissor state
			cmdBuffer->cmdSetScissor(device->getSwapChainExtent().height, (float)device->getSwapChainExtent().width, 0, 0);

			// Bind the vertex and textures for each model and mesh
			for (u32 i = 0; i < scene->getModelCount(); i++) {
				std::vector<Mesh>& model = scene->getModel(i);
				for (Mesh& mesh : model) {
					std::array<>
				}
			}
			cmdBuffer->cmdBindDescriptorSets(PipelineBindPoint::GRAPHICS, device->getPipelineLayout(pipelineHandle), device->getDescriptorSets(GraphNodeBase::descriptorSets));

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

