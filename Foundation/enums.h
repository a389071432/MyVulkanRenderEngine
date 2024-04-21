#pragma once


namespace zzcVulkanRenderEngine {

	enum class TextureType {
		Texture1D,
		Texture2D,
		Texture3D
	};

	enum class GraphResourceType {
		TEXTURE,
		DEPTH_MAP,
		BUFFER,
		RENDER_TARGET,

	};

	enum class GraphNodeType {
		GRAPHICS,
		COMPUTE
	};

	enum class GraphResourceAccessType {
		READ_TEXTURE,
		WRITE_ATTACHMENT,
		WRITE_DEPTH
	};
}
