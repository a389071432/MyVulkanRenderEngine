#pragma once
#include"Scene.h"

namespace zzcVulkanRenderEngine {
	class SimpleScene : public Scene {
	public:
		SimpleScene() {}
		~SimpleScene() {}
		void add_model(const std::string& filename) override {
			std::vector<float> vertexData{
			1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
			};

			std::vector<uint32_t> indexData{ 0, 1, 2 };

			Mesh mesh;
			mesh.vertex_buffer = device->createBufferFromData(vertexData);
			mesh.index_buffer = device->createBufferFromData(indexData);
			mesh.index_count = indexData.size();

			mesh.material.descriptorSets = INVALID_DESCRIPTORSETS_HANDLE;

			// specify the vertex input info
			vertexInfo.setBindingDesc({ 0,6 * sizeof(float),VertexInputRate::VERTEX })
				.addVertexAttribute({ 0,0,0,DataFormat::FLOAT3 })
				.addVertexAttribute({ 0,1,3 * sizeof(float),DataFormat::FLOAT3 });
		}
	};
}