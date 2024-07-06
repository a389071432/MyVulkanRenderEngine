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

		}
	};
}

