#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord_attr;

uniform mat4 to_world;
uniform mat4 to_camera_space;
uniform mat4 nonlinear_perspective;
uniform mat4 linear_perspective;
uniform int projection_type;

out vec2 texture_coord;

void main()
{
    gl_Position = to_camera_space * to_world * vec4(pos, 1.0);
    texture_coord = texture_coord_attr;
    
    if(projection_type == 0)
    {
        gl_Position = linear_perspective * gl_Position;
        //gl_Position.z = gl_Position.z * gl_Position.w;
    }
    else
    {
        gl_Position = nonlinear_perspective * gl_Position;
    }
    
    //gl_Position = vec4(gl_Position.xy, gl_Position.z, gl_Position.w);
}