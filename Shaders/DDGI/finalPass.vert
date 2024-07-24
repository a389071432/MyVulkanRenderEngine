#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 pos;
layout(location = 1) out vec3 color;
layout(location = 2) out vec3 normal;

void main() {
    gl_Position = vec4(inPosition, 1.0);
	pos = inPosition;
    color = inColor;
	normal = inNormal;
}