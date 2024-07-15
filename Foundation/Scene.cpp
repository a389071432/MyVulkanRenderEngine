#include"Scene.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {
	int Scene::getModelCount() {
		return models.size();
	}

	std::vector<Mesh>& Scene::getModel(int index) {
		ASSERT(
			index >= 0 && index < models.size(),
			"Assertion failed: model index out of range"
		);
		return models[index];
	}

	void Scene::remove_model(u32 index) {

	}
}

