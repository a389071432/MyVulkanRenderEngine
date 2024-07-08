#include"Foundation/GPUDevice.h"
#include"Foundation/RenderGraph.h"
#include"Foundation/Engine.h"
#include"RenderGraphExamples/ExampleNode.h"

using namespace zzcVulkanRenderEngine;

int main() {
	GPUDeviceCreation gpuCI{};
	Engine engine;
	RenderGraph graph;

	//define the render graph
	ExampleNode node;
	node.setType(GraphNodeType::GRAPHICS)
		.setOutputs({
			{
				false,                                 // isExternal
				GraphResourceType::TEXTURE,            // resource type
				ResourceInfo{                          // Explicitly initialize ResourceInfo
					.texture = {                       // Initialize the texture member of the union
						800,                           // width
						600,                           // height
						1,                             // depth
						TextureType::Texture2D,
						DataFormat::FLOAT4,
						VK_ATTACHMENT_LOAD_OP_CLEAR,   // Add loadOp
						VK_ATTACHMENT_STORE_OP_STORE,  // Add storeOp
						TextureHandle{}                // Add texHandle (initialize as needed)
					}
				},
				"final",                               // unique key
				0,                                     // groupId (to which descriptor set it belongs)
				0,                                     // binding point (in a descriptor set)
				ShaderStage::DONT_CARE                 // shader stage that would access the resource
			},
		})
		.setPipelineInfo(
			{
				.shaders = {
				  .vertShaderPath = "",
				  .fragShaderPath = ""
                },
				.vertexInput = {
				   .bindingDesc={0,sizeof(ExampleNode::Vertex),VertexInputRate::VERTEX},
				   .attributes = {
					   {0,0,offsetof(ExampleNode::Vertex,position),DataFormat::FLOAT3},
			           {0,1,offsetof(ExampleNode::Vertex,color),DataFormat::FLOAT3},
			       }
                },
				.rasterInfo = {
				   .cullMode = CullMode::BACK,
				   .frontFace = FrontFace::CONTER_CLOCKWISE,
                },
				.msaa = {
				   .nSamplesPerPixel=1
                },
			    .depthStencil = {
				   .enableDepth = false
                }
			}
		);

	graph.addNode(
		&node
	);
}