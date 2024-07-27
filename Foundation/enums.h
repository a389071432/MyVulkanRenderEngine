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
		COMPUTE,
		RAYTRACING
	};

	enum class GraphResourceAccessType {
		READ_TEXTURE,
		WRITE_ATTACHMENT,
		WRITE_DEPTH,
		COPY_SRC,
		COPY_DST,
		PRESENT,
		COMPUTE_READ_STORAGE_IMAGE,
		COMPUTE_READ_WRITE_STORAGE_IMAGE, 
		RAYTRACING_READ_STORAGE_IMAGE,
		RAYTRACING_READ_WRITE_STORAGE_IMAGE,
		UNDEFINED
	};

	// TODO: more types
	enum class ShaderStage {
		VERTEX,
		FRAG,
		ALL,
		RANDOM,
		RAYGEN,
		MISS,
		HIT,
		DONT_CARE
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
		UINT4,

		UNORM,
		UNORM2,
		UNORM3,
		UNORM4,

		UNDEFINED
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

	enum class PipelineBindPoint {
		GRAPHICS,
		COMPUTE,
		RAY_TRACING
	};

	enum class BufferUsage {
		VERTEX,
		INDEX,
		UNIFORM,
		STORAGE,
		SBT      // Shader Binding Table
	};

	enum class RayTracingShaderType {
		RAY_GEN,
		HIT,
		MISS
	};
}
