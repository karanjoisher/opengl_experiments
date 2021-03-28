#include "render.h"
#include "open_gl.cpp"

struct LightingProgram
{
    GLuint program_id;
    GLuint to_world_space;
    GLuint perspective_projection;
    GLuint to_camera_space;
    GLuint object_color;
    GLuint lighting_disabled;
    GLuint light_color;
    GLuint ambient_light_fraction;
    GLuint ambientness;
};


void create_lighting_program(LightingProgram *program)
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
uniform vec4 object_color;
uniform int lighting_disabled;
uniform vec3 light_color;
uniform float ambient_light_fraction;
uniform float ambientness;
out vec4 FragColor;

void main()
{
vec4 color;
if(object_color.r >= 0 && object_color.g >= 0 && object_color.b >= 0 && object_color.a >= 0)
{
color = object_color;
}
else
{
color = texture(texture2d_sampler, fs_texture_uv);
}

if(lighting_disabled == 0)
{

vec3 incident_ambient_light = ambient_light_fraction * light_color;
vec4 reflected_ambient = vec4(ambientness * color.rgb * incident_ambient_light, color.a);
 color = reflected_ambient;
}

FragColor = color;
}
)FOO";
    
    program->program_id = gl_create_program(vertex_source, fragment_source);
    
    GL(program->to_world_space = glGetUniformLocation(program->program_id, "to_world_space"));
    GL(program->to_camera_space = glGetUniformLocation(program->program_id, "to_camera_space"));
    GL(program->perspective_projection = glGetUniformLocation(program->program_id, "perspective_projection"));
    
    gl_set_uniform_1i(program->program_id, "texture2d_sampler", 0);
    GL(program->object_color = glGetUniformLocation(program->program_id, "object_color"));
    GL(program->lighting_disabled = glGetUniformLocation(program->program_id, "lighting_disabled"));
    GL(program->light_color = glGetUniformLocation(program->program_id, "light_color"));
    GL(program->ambient_light_fraction = glGetUniformLocation(program->program_id, "ambient_light_fraction"));
    GL(program->ambientness = glGetUniformLocation(program->program_id, "ambientness"));
}

void use_lighting_program(LightingProgram *program, GLuint texture_id, hmm_v4 object_color, hmm_mat4 *to_world_space, hmm_mat4 *to_camera_space, hmm_mat4 *perspective_transform, bool lighting_disabled, hmm_v3 light_color, f32 ambient_light_fraction, f32 ambientness)
{
    // TODO(Karan): This function can probably take in a VAO spec and the Vertex Attribute stream and bind them here i.e. this function can take in values required to setup its vertex input. 
    
    GL(glUseProgram(program->program_id));
    
    if(texture_id != 0)
    {
        GL(glActiveTexture(GL_TEXTURE0)); 
        GL(glBindTexture(GL_TEXTURE_2D, texture_id));
        object_color = {-1.0f, -1.0f, -1.0f, -1.0f};
    }
    
    GL(glUniformMatrix4fv(program->to_world_space, 1, GL_FALSE, (GLfloat*)to_world_space->Elements));
    GL(glUniformMatrix4fv(program->to_camera_space, 1, GL_FALSE, (GLfloat*)to_camera_space->Elements));
    GL(glUniformMatrix4fv(program->perspective_projection, 1, GL_FALSE, (GLfloat*)(perspective_transform->Elements)));
    
    GL(glUniform4fv(program->object_color, 1, (GLfloat*)(object_color.Elements)));
    GL(glUniform1i(program->lighting_disabled, lighting_disabled ? 1 : 0));
    GL(glUniform3fv(program->light_color, 1, (GLfloat*)(light_color.Elements)));
    GL(glUniform1f(program->ambient_light_fraction, ambient_light_fraction));
    GL(glUniform1f(program->ambientness, ambientness));
}