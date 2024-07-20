#pragma once
#include"Scene.h"
#include"Third parties/tinygltf/tiny_gltf.h"

namespace zzcVulkanRenderEngine {
	class gltfScene :public Scene {
	public:
		gltfScene();
		~gltfScene();
		void add_mesh(const std::string& filename) override;

	private:
		template<typename T>
		T GetAttributeValue(const ::tinygltf::Model& model, const ::tinygltf::Accessor& accessor, size_t index, int component);
	};
}

