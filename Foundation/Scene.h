#pragma once
#include"datatypes.h"
#include"Resource.h"
#include"glm/glm.hpp"
#include<vector>
#include"GPUDevice.h"
#include<vulkan/vulkan.h>

namespace zzcVulkanRenderEngine {
    struct PBRMaterial {
        // Indices used for bindless textures.
        TextureHandle       albedo = INVALID_TEXTURE_HANDLE;
        TextureHandle       metal_roughness = INVALID_TEXTURE_HANDLE;
        TextureHandle       normal = INVALID_TEXTURE_HANDLE;
        TextureHandle       occlusion = INVALID_TEXTURE_HANDLE;
        TextureHandle       emissive = INVALID_TEXTURE_HANDLE;

        DescriptorSetLayoutsHandle setLayouts;
        DescriptorSetsHandle descriptorSets = INVALID_DESCRIPTORSETS_HANDLE;
    };

    struct Mesh {
        PBRMaterial material;
        BufferHandle vertex_buffer;
        //BufferHandle position_buffer;
        //BufferHandle tangent_buffer;
        //BufferHandle normal_buffer;
        //BufferHandle uv_buffer;

        BufferHandle index_buffer;

        u32 index_count;

        //u32 position_offset;
        //u32 tangent_offset;
        //u32 normal_offset;
        //u32 uv_offset;
        
        u32 primitive_cnt;

        void destroy();
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
        virtual void add_model(const std::string& filename) = 0;
        void remove_model(u32 index);
        int getModelCount();
        std::vector<Mesh>& getModel(int index);
        GraphicsPipelineCreation::VertexInput vertexInfo;
        /*virtual void prepare() = 0;*/
    protected:
        GPUDevice* device;
        std::vector<Mesh> meshes;
        std::vector<std::vector<Mesh>> models; // a model is defined as a set of meshes

        //// only store the handle of resources on device
        //std::vector<BufferHandle> buffers;
        //std::vector<TextureHandle> textures;

    };
}


