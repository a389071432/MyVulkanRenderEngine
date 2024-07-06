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

		struct {
			VkDeviceMemory memory; 
			VkBuffer buffer;      
		} vertices;

		struct {
			VkDeviceMemory memory;
			VkBuffer buffer;
			uint32_t count;
		} indices;

		// overwrite the interfaces
		virtual void init(GPUDevice* device) {
			// define the triangle
			std::vector<Vertex> vertexBuffer{
				{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
				{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
			};
			std::vector<uint32_t> indexBuffer{ 0, 1, 2 };

			BufferHandle vHnd = device->createBufferFromData(vertexBuffer);
			BufferHandle iHnd = device->createBufferFromData(indexBuffer);
		}

		virtual void execute() {

		}
	};
}

