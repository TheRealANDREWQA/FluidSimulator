#version 430 core

uniform usampler2D CollisionMap;
uniform vec4 draw_color;
uniform uint window_width;

in vec2 uv;
out vec4 FragColor;

void main()
{
    vec2 texel_pos = uv * textureSize(CollisionMap, 0);
    ivec2 int_texel_pos = ivec2(floor(texel_pos));
    float float_alpha_value = texelFetch(CollisionMap, int_texel_pos, 0).r;
    uint alpha_sample = uint(float_alpha_value);
    uint bit_index = uint(uv.x * window_width) & 7;
    float alpha = (alpha_sample & (1 << bit_index)) != 0 ? 1.0f : 0.0f;
    FragColor = vec4(draw_color.xyz, draw_color.a * alpha);
}   
