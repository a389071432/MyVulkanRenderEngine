#version 450

layout(binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 color = texture(tex, fragTexCoord).rgb + vec3(0.2,0,0);
    outColor = vec4(color,1.0);
}