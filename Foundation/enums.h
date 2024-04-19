#pragma once

typedef enum GraphResourceType {
	TEXTURE_TO_SAMPLE,
	BUFFER,
	RENDER_TARGET,
	DEPTH_MAP
}GraphResourceType;

enum GraphNodeType {
	GRAPHICS,
	COMPUTE
};