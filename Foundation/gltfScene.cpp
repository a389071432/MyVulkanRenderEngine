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
                static_cast<u32>(image.width),
                static_cast<u32>(image.height),
                util_getDataFormat(image)
            );
        }

        for (const auto& mesh : model.meshes) {
            Mesh newMesh;
            newMesh.primitive_cnt = mesh.primitives.size();
            std::vector<float> posData;
            std::vector<float> normData;
            std::vector<float> tanData;
            std::vector<float> uvData;
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
                const ::tinygltf::Accessor& posAccessor = model.accessors[attribs.at("POSITION")];
                for (size_t i = 0; i < posAccessor.count; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        posData.push_back(GetAttributeValue<float>(model, posAccessor, i, j));
                    }
                }

                // Normal
                for (size_t i = 0; i < posAccessor.count; ++i) {

                    if (attribs.find("NORMAL") != attribs.end()) {
                        const ::tinygltf::Accessor& accessor = model.accessors[attribs.at("NORMAL")];
                        for (int j = 0; j < 3; ++j) {
                            normData.push_back(GetAttributeValue<float>(model, accessor, i, j));
                        }
                    }
                }

                // Tangent
                for (size_t i = 0; i < posAccessor.count; ++i) {

                    if (attribs.find("TANGENT") != attribs.end()) {
                        const ::tinygltf::Accessor& accessor = model.accessors[attribs.at("TANGENT")];
                        for (int j = 0; j < 4; ++j) {
                            tanData.push_back(GetAttributeValue<float>(model, accessor, i, j));
                        }
                    }
                }

                // UV
                for (size_t i = 0; i < posAccessor.count; ++i) {
                    if (attribs.find("TEXCOORD_0") != attribs.end()) {
                        const ::tinygltf::Accessor& accessor = model.accessors[attribs.at("TEXCOORD_0")];
                        for (int j = 0; j < 2; ++j) {
                            uvData.push_back(GetAttributeValue<float>(model, accessor, i, j));
                        }
                    }
                }
            }

            newMesh.position_buffer = device->createBufferFromData(posData);
            newMesh.normal_buffer = device->createBufferFromData(normData);
            newMesh.tangent_buffer = device->createBufferFromData(tanData);
            newMesh.uv_buffer = device->createBufferFromData(uvData);
            newMesh.index_buffer = device->createBufferFromData(indexData);

            // associate the pbr material
            // TODO: each primitive may refer to different materials
            // (Now we assume that all primitives from the same mesh all share the same material)
            u32 mtIndex = mesh.primitives[0].material;
            newMesh.material.albedo = textureHandles[model.textures[model.materials[mtIndex].pbrMetallicRoughness.baseColorTexture.index].source];
            newMesh.material.metal_roughness= textureHandles[model.textures[model.materials[mtIndex].pbrMetallicRoughness.metallicRoughnessTexture.index].source];
            newMesh.material.normal = textureHandles[model.textures[model.materials[mtIndex].normalTexture.index].source];
            newMesh.material.occlusion = textureHandles[model.textures[model.materials[mtIndex].occlusionTexture.index].source];
            newMesh.material.emissive = textureHandles[model.textures[model.materials[mtIndex].emissiveTexture.index].source];

            // create sampler for each texture
            

            meshes.push_back(newMesh);


        }
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