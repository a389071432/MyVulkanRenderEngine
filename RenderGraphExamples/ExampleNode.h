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

			// Bind the pipeline
			cmdBuffer->cmdBindGraphicsPipeline(device->getGraphicsPipeline(pipelineHandle));

			// Bind the vertex and textures for each model and mesh
			for (u32 i = 0; i < scene->getModelCount(); i++) {
				std::vector<Mesh>& model = scene->getModel(i);
				std::vector<VkDescriptorSet> setsToBind;
				setsToBind.resize(2);
				setsToBind[0] = device->getDescriptorSets(GraphNodeBase::descriptorSets)[0];
				for (Mesh& mesh : model) {
					cmdBuffer->cmdBindVertex(device->getBuffer(mesh.vertex_buffer));
					cmdBuffer->cmdBindIndexBuffer(device->getBuffer(mesh.index_buffer));
					if (mesh.material.descriptorSets != INVALID_DESCRIPTORSETS_HANDLE) {
						setsToBind[1] = device->getDescriptorSets(mesh.material.descriptorSets)[0];
						cmdBuffer->cmdBindDescriptorSets(PipelineBindPoint::GRAPHICS, device->getPipelineLayout(pipelineHandle), setsToBind);
					}
					cmdBuffer->cmdDrawIndexed(mesh.index_count, 1);
				}
			}
		}
	};
}

