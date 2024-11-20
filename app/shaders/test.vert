#version 450

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(binding = 0, set = 0) uniform UBO
{
    vec2 offset;
};

void main() {
    gl_Position = vec4(position + offset, 0.0, 1.0);
    fragColor = color;
    uv = position + 0.5;
}