#version 430 core

uniform sampler2D circle_alpha;
uniform sampler2D color_texture;

in vec2 circle_uv;
in vec2 texture_uv;
out vec4 FragColor;

void main()
{
    float alpha = texture(circle_alpha, circle_uv).r;
    vec4 color_value = texture(color_texture, texture_uv); 
    FragColor = vec4(color_value.xyz, alpha);
}   
