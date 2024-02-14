#version 430 core
layout(std430, binding = 0) readonly buffer _Positions
{
    vec2 Positions[];
};

layout(std430, binding = 1) readonly buffer _TextureUvs
{
    vec2 TextureUvs[];
};

#define POSITION_FACTOR 500.0f

uniform float scale;
uniform float aspect_ratio;
  
out vec2 circle_uv;
out vec2 texture_uv;

void main()
{
    uint instance_ID = gl_InstanceID;
    vec2 vertex_positions[6] = {
        { -scale, -scale * aspect_ratio },
        { scale, -scale * aspect_ratio },
        { -scale, scale * aspect_ratio },
        { -scale, scale * aspect_ratio },
        { scale, scale * aspect_ratio },
        { scale, -scale * aspect_ratio }
    };
    vec2 uvs[6] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f }
    };

    uint vertex_id = gl_VertexID % 6;
    circle_uv = uvs[vertex_id];
    texture_uv = vec2(TextureUvs[instance_ID].x, -TextureUvs[instance_ID].y);
    gl_Position = vec4(Positions[instance_ID] / POSITION_FACTOR / vec2(aspect_ratio, 1.0f) + vertex_positions[vertex_id], 0.0, 1.0);
}
