#include "open_gl.cpp"

struct StandardShaderProgram
{
    GLuint program_id;
    GLuint vertex_color;
};


void create_standard_shader_program(StandardShaderProgram *standard_shader_program)
{
    char *vertex_source = R"FOO(
    
#version 330 core

layout (location = 0) in vec3 local_space_pos;
layout (location = 1) in vec2 texture_uv;

void main()
{
gl_Position = vec4(local_space_pos, 1.0);
}

)FOO";
    
    char *fragment_source = R"FOO(
    #version 330 core
    
uniform vec4 vertex_color;
out vec4 FragColor;

void main()
{
FragColor = vertex_color;
}
)FOO";
    
    standard_shader_program->program_id = gl_create_program(vertex_source, fragment_source);
    
    GL(standard_shader_program->vertex_color = glGetUniformLocation(standard_shader_program->program_id, "vertex_color"));
}


void use_standard_shader_program(StandardShaderProgram *program, f32 r, f32 g, f32 b, f32 a)
{
    GL(glUseProgram(program->program_id));
    GL(glUniform4f(program->vertex_color, r, g, b, a));
}