#version 430 core
layout(std430, binding = 0) readonly buffer _Positions
{
    vec2 Positions[];
};

layout(std430, binding = 1) buffer _Velocities
{
    vec2 Velocities[];
};

#define POSITION_FACTOR 500.0f

uniform float scale;
uniform float aspect_ratio;
uniform float max_speed;

uniform sampler1D Heatmap;
  
out vec3 vertex_color;
out vec2 uv;

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

    float speed = length(Velocities[instance_ID]);
	float speedT = clamp(speed / max_speed, 0.0, 0.99);

    vertex_color = texture(Heatmap, speedT).xyz;
    uint vertex_id = gl_VertexID % 6;
    uv = uvs[vertex_id];
    gl_Position = vec4(Positions[instance_ID] / POSITION_FACTOR / vec2(aspect_ratio, 1.0f) + vertex_positions[vertex_id], 0.0, 1.0);
}
