#include "open_gl.cpp"

struct StandardShaderProgram
{
    GLuint program_id;
    GLuint translation;
};


void create_standard_shader_program(StandardShaderProgram *standard_shader_program)
{
    char *vertex_source = R"FOO(
    
#version 330 core

layout (location = 0) in vec3 vs_local_space_pos;
layout (location = 1) in vec2 vs_texture_uv;

uniform mat4 translation;
out vec2 fs_texture_uv;

void main()
{

// Changing from right handed to left handed coordinate system... This will be included in perspective/orthographic matrix as well
gl_Position = translation * vec4(vs_local_space_pos.xy, -vs_local_space_pos.z, 1.0);
fs_texture_uv = vs_texture_uv;
}

)FOO";
    
    char *fragment_source = R"FOO(
    #version 330 core
    
    in vec2 fs_texture_uv;
uniform sampler2D texture2d_sampler;

out vec4 FragColor;

void main()
{
FragColor = texture(texture2d_sampler, fs_texture_uv);
}
)FOO";
    
    standard_shader_program->program_id = gl_create_program(vertex_source, fragment_source);
    
    GL(standard_shader_program->translation = glGetUniformLocation(standard_shader_program->program_id, "translation"));
    
    gl_set_uniform_1i(standard_shader_program->program_id, "texture2d_sampler", 0);
}


void use_standard_shader_program(StandardShaderProgram *program, GLuint texture_id, hmm_v3 *translation)
{
    GL(glUseProgram(program->program_id));
    GL(glActiveTexture(GL_TEXTURE0)); 
    GL(glBindTexture(GL_TEXTURE_2D, texture_id));
    
    GL(glUniformMatrix4fv(program->translation, 1, GL_FALSE, (GLfloat*)(HMM_Translate(*translation).Elements)));
    
}