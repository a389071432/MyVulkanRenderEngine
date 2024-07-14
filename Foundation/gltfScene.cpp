#include"gltfScene.h"
#include"Third parties/tinygltf/tiny_gltf.h"
#include"assert.h"
#include"utils/utils.h"

namespace zzcVulkanRenderEngine {
    gltfScene::gltfScene() {

    }

    gltfScene::~gltfScene() {

    }

    void gltfScene::add_model(const std::string& filename) {
        // load from gltf
        ::tinygltf::Model model;
        ::tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
        ASSERT(ret, "Failed to load glTF!");

        // Create all images
        std::vector<TextureHandle> textureHandles(model.images.size());
        for (size_t i = 0; i < model.images.size(); ++i) {
            const auto& image = model.images[i];
            TextureHandle handle = device->createTexture2DFromData(
                image.image,
                static_cast<u16>(image.width),
                static_cast<u16>(image.height),
                util_getDataFormat(image)
            );
        }

        bool hasNorm = false;
        bool hasTan = false;
        bool hasUV = false;
        for (const auto& mesh : model.meshes) {
            Mesh newMesh;
            newMesh.primitive_cnt = mesh.primitives.size();
            std::vector<float> vertexData;
            std::vector<uint32_t> indexData;

            u32 vertexCount = 0;
            for (const auto& primitive : mesh.primitives) {
                const auto& attribs = primitive.attributes;

                // Process indices
                if (primitive.indices >= 0) {
                    const ::tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                    uint32_t indexOffset = vertexCount;

                    for (size_t i = 0; i < accessor.count; ++i) {
                        uint32_t index = 0;
                        switch (accessor.componentType) {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            index = static_cast<uint32_t>(GetAttributeValue<uint16_t>(model, accessor, i, 0));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            index = GetAttributeValue<uint32_t>(model, accessor, i, 0);
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            index = static_cast<uint32_t>(GetAttributeValue<uint8_t>(model, accessor, i, 0));
                            break;
                        default:
                            ASSERT(false, "Unsupported index component type");
                        }
                        indexData.push_back(index + indexOffset);
                    }
                    vertexCount += accessor.count;
                }

                // Gather per-vertex data
                // Position
                ASSERT(
                    attribs.find("POSITION") != attribs.end(),
                    "Assertion failed: missing attribute POSTION from gltf!"
                );
                const ::tinygltf::Accessor& posAccessor = model.accessors[attribs.at("POSITION")];
                for (size_t i = 0; i < posAccessor.count; i++) {
                    for (int j = 0; j < 3; ++j) {
                        vertexData.push_back(GetAttributeValue<float>(model, posAccessor, i, j));
                    }
                }

                // Normal
                if (attribs.find("NORMAL") != attribs.end()) {
                    hasNorm = true;
                    const ::tinygltf::Accessor& normAccessor = model.accessors[attribs.at("NORMAL")];
                    for (size_t i = 0; i < normAccessor.count; i++) {
                        for (int j = 0; j < 3; ++j) {
                            vertexData.push_back(GetAttributeValue<float>(model, normAccessor, i, j));
                        }
                    }
                }

                // Tangent
                if (attribs.find("TANGENT") != attribs.end()) {
                    hasTan = true;
                    const ::tinygltf::Accessor& tanAccessor = model.accessors[attribs.at("NORMAL")];
                    for (size_t i = 0; i < tanAccessor.count; i++) {
                        for (int j = 0; j < 3; ++j) {
                            vertexData.push_back(GetAttributeValue<float>(model, tanAccessor, i, j));
                        }
                    }
                }

                // UV
                if (attribs.find("TEXCOORD_0") != attribs.end()) {
                    hasUV = true;
                    const ::tinygltf::Accessor& uvAccessor = model.accessors[attribs.at("TEXCOORD_0")];
                    for (size_t i = 0; i < uvAccessor.count; i++) {
                        for (int j = 0; j < 2; j++) {
                            vertexData.push_back(GetAttributeValue<float>(model, uvAccessor, i, j));
                        }
                    }
                }
            }

            newMesh.vertex_buffer = device->createBufferFromData(vertexData);
            newMesh.index_buffer = device->createBufferFromData(indexData);
            newMesh.index_count = indexData.size();

            // associate the pbr material
            // TODO: each primitive may refer to different materials
            // (Now we assume that all primitives from the same mesh all share the same material)
            u32 mtIndex = mesh.primitives[0].material;
            // allocate descriptors for the material
            DescriptorSetLayoutsCreation layoutCI{};
            u32 baseColorIndex = model.materials[mtIndex].pbrMetallicRoughness.baseColorTexture.index;
            u32 metalRoughIndex = model.materials[mtIndex].pbrMetallicRoughness.metallicRoughnessTexture.index;
            u32 normalIndex = model.materials[mtIndex].normalTexture.index;
            u32 occlusionIndex = model.materials[mtIndex].occlusionTexture.index;
            u32 emissiveIndex = model.materials[mtIndex].emissiveTexture.index;

            if (baseColorIndex >= 0) {
                newMesh.material.albedo = textureHandles[model.textures[baseColorIndex].source];
                layoutCI.addBinding({
                        BindingType::IMAGE_SAMPLER,
                        ShaderStage::FRAG,
                        0,
                        0
                    });
            }
            if (metalRoughIndex >= 0) {
                newMesh.material.metal_roughness = textureHandles[model.textures[metalRoughIndex].source];
                layoutCI.addBinding({
                        BindingType::IMAGE_SAMPLER,
                        ShaderStage::FRAG,
                        0,
                        0
                    });
            }
            if (normalIndex >= 0) {
                newMesh.material.normal = textureHandles[model.textures[normalIndex].source];
                layoutCI.addBinding({
                        BindingType::IMAGE_SAMPLER,
                        ShaderStage::FRAG,
                        0,
                        0
                    });
            }
            if (occlusionIndex >= 0) {
                newMesh.material.occlusion = textureHandles[model.textures[occlusionIndex].source];
                layoutCI.addBinding({
                        BindingType::IMAGE_SAMPLER,
                        ShaderStage::FRAG,
                        0,
                        0
                    });
            }
            if (emissiveIndex >= 0) {
                newMesh.material.emissive = textureHandles[model.textures[emissiveIndex].source];
                layoutCI.addBinding({
                        BindingType::IMAGE_SAMPLER,
                        ShaderStage::FRAG,
                        0,
                        0
                    });
            }

            // descriptor set for pbr materials
            DescriptorSetLayoutsHandle setLayout = device->createDescriptorSetLayouts(layoutCI);
            DescriptorSetsAlloc setAlloc{};
            setAlloc.setLayoutsHandle(setLayout);
            newMesh.material.descriptorSets = device->createDescriptorSets(setAlloc);
            newMesh.material.setLayout = setLayout;

            //write sets
            std::vector<DescriptorSetWrite> writes;
            if (baseColorIndex >= 0) {
                writes.push_back(
                    DescriptorSetWrite()
                    .setType(BindingType::IMAGE_SAMPLER)
                    .setDstSet(0)
                    .setDstBinding(0)
                    .setBufferHandle(INVALID_BUFFER_HANDLE)
                    .setTexHandle(newMesh.material.albedo)
                );
            }

            if (metalRoughIndex >= 0) {
                writes.push_back(
                    DescriptorSetWrite()
                    .setType(BindingType::IMAGE_SAMPLER)
                    .setDstSet(0)
                    .setDstBinding(1)
                    .setBufferHandle(INVALID_BUFFER_HANDLE)
                    .setTexHandle(newMesh.material.metal_roughness)
                );
            }

            if (normalIndex >= 0) {
                writes.push_back(
                    DescriptorSetWrite()
                    .setType(BindingType::IMAGE_SAMPLER)
                    .setDstSet(0)
                    .setDstBinding(2)
                    .setBufferHandle(INVALID_BUFFER_HANDLE)
                    .setTexHandle(newMesh.material.normal)
                );
            }

            if (occlusionIndex >= 0) {
                writes.push_back(
                    DescriptorSetWrite()
                    .setType(BindingType::IMAGE_SAMPLER)
                    .setDstSet(0)
                    .setDstBinding(3)
                    .setBufferHandle(INVALID_BUFFER_HANDLE)
                    .setTexHandle(newMesh.material.occlusion)
                );
            }
            device->writeDescriptorSets(writes, newMesh.material.descriptorSets);
            meshes.push_back(newMesh);


        }
        // set vertex input format
        vertexInfo.addVertexAttribute({ 0, 0, 0,DataFormat::FLOAT3 });
        u32 vertexSize = 3 * sizeof(float);
        u32 offset = 3 * sizeof(float);
        u32 location = 1;
        if (hasNorm) {
            vertexInfo.addVertexAttribute({ 0,location,offset,DataFormat::FLOAT3 });
            vertexSize += 3 * sizeof(float);
            location++;
            offset += 3 * sizeof(float);
        }
        if (hasTan) {
            vertexInfo.addVertexAttribute({ 0,location,offset,DataFormat::FLOAT3 });
            vertexSize += 3 * sizeof(float);
            location++;
            offset += 3 * sizeof(float);
        }
        if (hasUV) {
            vertexInfo.addVertexAttribute({ 0,location,offset,DataFormat::FLOAT2 });
            vertexSize += 2 * sizeof(float);
            location++;
            offset += 2 * sizeof(float);
        }
        vertexInfo.setBindingDesc({ 0, vertexSize, VertexInputRate::VERTEX });
        models.push_back(meshes);

        // TODO: a unified way to manage all models, enabling efficient addition and removal of models

    }

    template<typename T>
    T gltfScene::GetAttributeValue(const tinygltf::Model& model, const tinygltf::Accessor& accessor, size_t index, int component) {
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        const unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
        size_t stride = accessor.ByteStride(bufferView);
        const T* typedData = reinterpret_cast<const T*>(dataStart + index * stride);
        return typedData[component];
    }
}