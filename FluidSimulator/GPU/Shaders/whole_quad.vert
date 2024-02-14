#version 430 core

layout(location = 0) in vec2 position;

out vec2 uv;

void main() {
    uv = (position + vec2(1.0f, 1.0f)) * vec2(0.5f, 0.5f);
    gl_Position = vec4(position, 0, 1.0f);
}
