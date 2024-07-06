#pragma once


namespace zzcVulkanRenderEngine {

	enum class TextureType {
		Texture1D,
		Texture2D,
		Texture3D
	};

	enum class ResourceSharingMode {
		EXCLUSIVE,
		CONCURRENT
	};


	enum class BindingType {
		UNIFORM_BUFFER,
		IMAGE_SAMPLER,
		STORAGE_BUFFER
	};

	enum class GraphResourceType {
		TEXTURE,
		DEPTH_MAP,
		BUFFER
	};

	enum class GraphNodeType {
		GRAPHICS,
		COMPUTE
	};

	enum class GraphResourceAccessType {
		READ_TEXTURE,
		WRITE_ATTACHMENT,
		WRITE_DEPTH,
		COPY_SRC,
		COPY_DST,,
		PRESENT,
		UNDEFINED
	};

	// TODO: more types
	enum class ShaderStage {
		VERTEX,
		FRAG,
		RANDOM
	};

	enum class VertexInputRate {
		VERTEX,
		INSTANCE
	};

	// TODO: more formats support
	enum class DataFormat {
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,

		INT,
		INT2,
		INT3,
		INT4,

		UINT,
		UINT2,
		UINT3,
		UINT4
	};



	enum class CullMode {
		FRONT,
		BACK
	};

	enum class FrontFace {
		CLOCKWISE,
		CONTER_CLOCKWISE
	};

	enum class MSAASamples {
		SAMPLE_1,
		SAMPLE_2,
		SAMPLE_4,
		SAMPLE_8,
		SAMPLE_16,
		SAMPLE_32,
		SAMPLE_64,
		SAMPLE_MAX
	};
}
