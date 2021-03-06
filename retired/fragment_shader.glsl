#version 330 core

in vec2 texture_coord;

uniform sampler2D texture1_obj;

out vec4 FragColor;

void main()
{
    
    FragColor = texture(texture1_obj, texture_coord);
}