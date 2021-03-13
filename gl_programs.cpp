#include "render.h"
#include "open_gl.cpp"

struct StandardShaderProgram
{
    GLuint program_id;
    GLuint to_world;
    GLuint perspective_projection;
};


void create_standard_shader_program(StandardShaderProgram *standard_shader_program)
{
    char *vertex_source = R"FOO(
    
#version 330 core

layout (location = 0) in vec3 vs_local_space_pos;
layout (location = 1) in vec2 vs_texture_uv;

uniform mat4 to_world;
uniform mat4 perspective_projection;

out vec2 fs_texture_uv;

void main()
{

gl_Position =  perspective_projection * to_world * vec4(vs_local_space_pos.xyz, 1.0);
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
    
    GL(standard_shader_program->to_world = glGetUniformLocation(standard_shader_program->program_id, "to_world"));
    GL(standard_shader_program->perspective_projection = glGetUniformLocation(standard_shader_program->program_id, "perspective_projection"));
    
    gl_set_uniform_1i(standard_shader_program->program_id, "texture2d_sampler", 0);
}


void use_standard_shader_program(StandardShaderProgram *program, GLuint texture_id, hmm_v3 *translation, hmm_v3 *rotation, PerspectiveProjection *p)
{
    GL(glUseProgram(program->program_id));
    GL(glActiveTexture(GL_TEXTURE0)); 
    GL(glBindTexture(GL_TEXTURE_2D, texture_id));
    
    hmm_mat4 rotation_mat = Z_ROTATE(rotation->Z) * Y_ROTATE(rotation->Y) * X_ROTATE(rotation->X);
    
    hmm_mat4 to_world = HMM_Translate(*translation) * rotation_mat;
    
    GL(glUniformMatrix4fv(program->to_world, 1, GL_FALSE, (GLfloat*)to_world.Elements));
    GL(glUniformMatrix4fv(program->perspective_projection, 1, GL_FALSE, (GLfloat*)(p->transform.Elements)));
}