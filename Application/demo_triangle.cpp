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
			   {
				  800,                     // width
				  600,                     // height
				  1,                       // depth
				  TextureType::Texture2D,
				  DataFormat::FLOAT4
			   },                                     // details
			   "final",                               // unique key
			   0,                                     // groupId (to which descriptor set it belongs)
			   {},                                    // binding point (in a descriptor set)
			   ShaderStage::DONT_CARE                 // shader stage that would access the resource (no care for output resources)
			},
		})
		.s
	graph.addNode(
		node
	);
}