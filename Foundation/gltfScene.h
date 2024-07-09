#pragma once
#include"Scene.h"

namespace zzcVulkanRenderEngine {
	class gltfScene :public Scene {
	public:
		gltfScene();
		~gltfScene();
		void add_mesh() override;
		void prepare() override;
	};
}

