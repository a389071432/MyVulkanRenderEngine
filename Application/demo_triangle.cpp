#include"Foundation/GPUDevice.h"
#include"Foundation/RenderGraph.h"
#include"Foundation/Engine.h"
#include"RenderGraphExamples/ExampleRender.h"
#include"Foundation/SimpleScene.h"

using namespace zzcVulkanRenderEngine;

// TODO: a better way to handle the initialization of device, scene and engine
int main() {
	GPUDeviceCreation gpuCI{};
	GPUDevice device(gpuCI);
	Engine engine;
	RenderGraph graph;
	SimpleScene scene;
	ExampleRender render;

	//define the render graph
	GraphNode node;
	node.setType(GraphNodeType::GRAPHICS)
		.setOutputs(std::vector<GraphResource>{
			GraphResource{
				.isExternal = false,                                 
				.type = GraphResourceType::TEXTURE,            
				.info = ResourceInfo{                          
					.texture = {                       
						800,                           // width
						600,                           // height
						1,                             // depth
						TextureType::Texture2D,
						DataFormat::UNORM4,
						VK_ATTACHMENT_LOAD_OP_CLEAR,   // Add loadOp
						VK_ATTACHMENT_STORE_OP_STORE,  // Add storeOp
						TextureHandle{}                // Add texHandle (initialize as needed)
					}
				},
				.key = "final",                               
				.groupId = 0,                                     // groupId (to which descriptor set it belongs)
				.binding = 0,                                     // binding point (in a descriptor set)
				.accessStage = ShaderStage::DONT_CARE             // shader stage that would access the resource
			},
		})
		.setGraphicsPipelineInfo(
			new GraphicsPipelineInfo{
				.shaders = {
				  .vertShaderPath = "Shaders/vert.spv",
				  .fragShaderPath = "Shaders/frag.spv"
                },
				//.vertexInput = {
				//   .bindingDesc={0,sizeof(ExampleNode::Vertex),VertexInputRate::VERTEX},
				//   .attributes = {
				//	   {0,0,offsetof(ExampleNode::Vertex,position),DataFormat::FLOAT3},
			 //          {0,1,offsetof(ExampleNode::Vertex,color),DataFormat::FLOAT3},
			 //      }
    //            },
				.vertexInput = {
			        .bindingDesc=scene.vertexInfo.bindingDesc,
					.attributes=scene.vertexInfo.attributes
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
			)
			.register_render(&render);

	graph.addNode(
		&node
	);
	engine.setDevice(&device);
	engine.setRenderGraph(&graph);
	engine.setScene(&scene);
	engine.init();
	scene.add_model("");
	engine.run();

}