#pragma once


namespace zzcVulkanRenderEngine {

	enum class TextureType {
		Texture1D,
		Texture2D,
		Texture3D
	};

	enum class GraphResourceType {
		TEXTURE_TO_SAMPLE,
		BUFFER,
		RENDER_TARGET,
		DEPTH_MAP
	};

	enum class GraphNodeType {
		GRAPHICS,
		COMPUTE
	};
}
