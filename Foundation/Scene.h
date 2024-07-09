#pragma once
#include"datatypes.h"
#include"Resource.h"
#include"glm/glm.hpp"
#include<vector>
#include"GPUDevice.h"

namespace zzcVulkanRenderEngine {
    struct PBRMaterial {
        DescriptorSetsHandle descriptorSets;

        // Indices used for bindless textures.
        TextureHandle       albedo;
        TextureHandle       roughness;
        TextureHandle       normal;
        TextureHandle       occlusion;
    };

    struct Mesh {
        PBRMaterial material;
        BufferHandle position_buffer;
        BufferHandle index_buffer;
        BufferHandle tangent_buffer;
        BufferHandle normal_buffer;
        BufferHandle uv_buffer;

        u32 position_offset;
        u32 tangent_offset;
        u32 normal_offset;
        u32 uv_offset;
        
        u32 primitive_cnt;
    };


    // TODO: IndirectDraw + bindless to reduce number of draw calls by grouping multiple onjects in a single draw call
    // multiple textures associated with different objects are all stored in a large texture array 
    // in shader code, use the build-in variable gl_DrawID and gl_InstanceIndex
    // gl_DrawID used to index the object
    // gl_InstanceIndex used to index an instance of an object 
    // (for example, window tiles can be considered different instances of the tile object with same textures but different positions)
    // (another typical example of instancing is to render massive number of plants)
    // (the instance-specific parameters are passed as an array bound to a descriptor)
    class Scene {
    public:
        virtual void add_mesh() = 0;
        virtual void prepare() = 0;
    private:
        GPUDevice* device;
        std::vector<Mesh> meshes;

        // only store the handle of resources on device
        std::vector<BufferHandle> buffers;
        std::vector<TextureHandle> textures;

    };
}


