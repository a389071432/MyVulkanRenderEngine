#include"gltfScene.h"
#include"Third parties/tinygltf/tiny_gltf.h"
#include"assert.h"

namespace zzcVulkanRenderEngine {
    gltfScene::gltfScene() {

    }

    gltfScene::~gltfScene() {

    }

    void gltfScene::add_mesh(const std::string& filename) {
        // load from gltf
        ::tinygltf::Model model;
        ::tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
        ASSERT(ret, "Failed to load glTF!");

        // Create all buffers
        std::vector<BufferHandle> bufferHandles(model.buffers.size());
        for (size_t i = 0; i < model.buffers.size(); ++i) {

            bufferHandles[i] = device->createBufferFromData(model.buffers[i].data);
        }

        // Create all images
        std::vector<TextureHandle> textureHandles(model.images.size());
        for (size_t i = 0; i < model.images.size(); ++i) {
            const auto& image = model.images[i];
            TextureHandle handle = device->createTexture2DFromData(
                image.image,
                static_cast<u32>(image.width),
                static_cast<u32>(image.height),
                image.component
            );
        }

        for (const auto& mesh : model.meshes) {
            Mesh newMesh;
            newMesh.primitive_cnt = mesh.primitives.size();

            // Create a new buffer for this mesh's data
            std::vector<float> meshData;
            u32 vertexCount = 0;

            for (const auto& primitive : mesh.primitives) {
                const auto& attribs = primitive.attributes;

                // Get accessor for position (assuming it exists)
                const ::tinygltf::Accessor& posAccessor = model.accessors[attribs.at("POSITION")];
                vertexCount += posAccessor.count;

                // Interleave attributes
                for (size_t i = 0; i < posAccessor.count; ++i) {
                    // Position
                    for (int j = 0; j < 3; ++j) {
                        meshData.push_back(GetAttributeValue<float>(model, posAccessor, i, j));
                    }

                    // Normal
                    if (attribs.find("NORMAL") != attribs.end()) {
                        const ::tinygltf::Accessor& accessor = model.accessors[attribs.at("NORMAL")];
                        for (int j = 0; j < 3; ++j) {
                            meshData.push_back(GetAttributeValue<float>(model, accessor, i, j));
                        }
                    }

                    // Tangent
                    if (attribs.find("TANGENT") != attribs.end()) {
                        const ::tinygltf::Accessor& accessor = model.accessors[attribs.at("TANGENT")];
                        for (int j = 0; j < 4; ++j) {
                            meshData.push_back(GetAttributeValue<float>(model, accessor, i, j));
                        }
                    }

                    // UV
                    if (attribs.find("TEXCOORD_0") != attribs.end()) {
                        const ::tinygltf::Accessor& accessor = model.accessors[attribs.at("TEXCOORD_0")];
                        for (int j = 0; j < 2; ++j) {
                            meshData.push_back(GetAttributeValue<float>(model, accessor, i, j));
                        }
                    }
                }
            }

            // Create a single buffer for all vertex data
            //newMesh.vertex_buffer = createBufferFromData(meshData.data());

            // ... (material loading remains the same)

            meshes.push_back(newMesh);
        }
    }
}