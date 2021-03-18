#include "render.h"
#include "open_gl.cpp"

struct StandardShaderProgram
{
    GLuint program_id;
    GLuint to_world_space;
    GLuint perspective_projection;
    GLuint to_camera_space;
};


void create_standard_shader_program(StandardShaderProgram *standard_shader_program)
{
    char *vertex_source = R"FOO(
    
#version 330 core

layout (location = 0) in vec3 vs_local_space_pos;
layout (location = 1) in vec2 vs_texture_uv;

uniform mat4 to_world_space;
uniform mat4 to_camera_space;
uniform mat4 perspective_projection;

out vec2 fs_texture_uv;

void main()
{

gl_Position =  perspective_projection * to_camera_space * to_world_space * vec4(vs_local_space_pos.xyz, 1.0);
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
    
    GL(standard_shader_program->to_world_space = glGetUniformLocation(standard_shader_program->program_id, "to_world_space"));
    GL(standard_shader_program->to_camera_space = glGetUniformLocation(standard_shader_program->program_id, "to_camera_space"));
    GL(standard_shader_program->perspective_projection = glGetUniformLocation(standard_shader_program->program_id, "perspective_projection"));
    
    gl_set_uniform_1i(standard_shader_program->program_id, "texture2d_sampler", 0);
}


void use_standard_shader_program(StandardShaderProgram *program, GLuint texture_id, hmm_v3 *translation, hmm_v3 *rotation, hmm_mat4 *to_camera_space, f32 near_plane_distance, f32 far_plane_distance, f32 fov_radians, f32 aspect_ratio, PerspectiveTransformCoordinateSystemOption coordinate_system_type)
{
    GL(glUseProgram(program->program_id));
    GL(glActiveTexture(GL_TEXTURE0)); 
    GL(glBindTexture(GL_TEXTURE_2D, texture_id));
    
    hmm_mat4 to_world_space  = HMM_Translate(*translation) * Z_ROTATE(rotation->Z) * Y_ROTATE(rotation->Y) * X_ROTATE(rotation->X);
    hmm_mat4 perspective_transform = {};
    create_perspective_transform(&perspective_transform, near_plane_distance, far_plane_distance, fov_radians, aspect_ratio, coordinate_system_type);
    GL(glUniformMatrix4fv(program->to_world_space, 1, GL_FALSE, (GLfloat*)to_world_space.Elements));
    GL(glUniformMatrix4fv(program->to_camera_space, 1, GL_FALSE, (GLfloat*)to_camera_space->Elements));
    GL(glUniformMatrix4fv(program->perspective_projection, 1, GL_FALSE, (GLfloat*)(perspective_transform.Elements)));
}