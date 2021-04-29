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
    GLuint light_position;
    GLuint light_color;
    GLuint ambient_light_fraction;
    GLuint ambientness;
    GLuint diffuseness;
    GLuint specularness;
    GLuint specular_unscatterness;
};


void create_lighting_program(LightingProgram *program)
{
    char *vertex_source = R"FOO(
    
#version 330 core

layout (location = 0) in vec3 vs_local_space_pos;
layout (location = 1) in vec2 vs_texture_uv;
layout (location = 2) in vec3 vs_normal;

uniform mat4 to_world_space;
uniform mat4 to_camera_space;
uniform mat4 perspective_projection;
uniform vec3 light_position;

out vec2 fs_texture_uv;
out vec3 light_pos_in_camera_space;
out vec3 vertex_pos_in_camera_space;
out vec3 vertex_normal_in_camera_space;

void main()
{
gl_Position =  perspective_projection * to_camera_space * to_world_space * vec4(vs_local_space_pos.xyz, 1.0f);

fs_texture_uv = vs_texture_uv;
light_pos_in_camera_space = (to_camera_space * vec4(light_position.xyz, 1.0f)).xyz;
 vertex_pos_in_camera_space =  (to_camera_space * to_world_space * vec4(vs_local_space_pos.xyz, 1.0f)).xyz;
 //vertex_normal_in_camera_space = mat3(transpose(inverse(to_camera_space * to_world_space))) * vs_normal;
 vertex_normal_in_camera_space = (to_camera_space * to_world_space * vec4(vs_normal, 0.0f)).xyz;
}

)FOO";
    
    char *fragment_source = R"FOO(
    #version 330 core
    
    in vec2 fs_texture_uv;
       in vec3 light_pos_in_camera_space;
 in vec3 vertex_pos_in_camera_space;
 in vec3 vertex_normal_in_camera_space;
 
    uniform sampler2D texture2d_sampler;
uniform vec4 object_color;
uniform int lighting_disabled;
uniform vec3 light_color;
uniform float ambient_light_fraction;
uniform float ambientness;
uniform float diffuseness;
uniform float specularness;
uniform float specular_unscatterness;
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
vec3 normal = normalize(vertex_normal_in_camera_space);
// Ambient light
vec3 incident_ambient_light = ambient_light_fraction * light_color;
vec3 reflected_ambient = ambientness * color.rgb * incident_ambient_light;

// Diffuse light
vec3 vertex_to_light_dir = normalize(light_pos_in_camera_space - vertex_pos_in_camera_space);
float diffuse_light_reflected_fraction = max(0.0f, dot(vertex_to_light_dir, normal));
vec3 reflected_diffuse = diffuseness * diffuse_light_reflected_fraction * color.rgb * light_color;

// Specular light
vec3 reflected_specular = vec3(.0f, .0f, .0f);
if(dot(vertex_to_light_dir, normal) >= 0) // Do not light from behind the surface's facing direction!
{
vec3 camera_pos = vec3(0.0f, 0.0f, 0.0f);
vec3 vertex_to_camera_dir = normalize(camera_pos - vertex_pos_in_camera_space);
vec3 reflect_light_dir = reflect(-vertex_to_light_dir, normal);
float specular_light_reflected_fraction = pow(max(0.0f, dot(reflect_light_dir, vertex_to_camera_dir)), specular_unscatterness);
reflected_specular = specularness * specular_light_reflected_fraction * color.rgb * light_color;
}
// Final Phong light
color = vec4(reflected_ambient + reflected_diffuse + reflected_specular, color.a);
}

FragColor = color;
}
)FOO";
    
    program->program_id = gl_create_program(vertex_source, fragment_source);
    
    GL(program->to_world_space = glGetUniformLocation(program->program_id, "to_world_space"));
    GL(program->to_camera_space = glGetUniformLocation(program->program_id, "to_camera_space"));
    GL(program->perspective_projection = glGetUniformLocation(program->program_id, "perspective_projection"));
    GL(program->light_position = glGetUniformLocation(program->program_id, "light_position"));
    
    gl_set_uniform_1i(program->program_id, "texture2d_sampler", 0);
    GL(program->object_color = glGetUniformLocation(program->program_id, "object_color"));
    GL(program->lighting_disabled = glGetUniformLocation(program->program_id, "lighting_disabled"));
    GL(program->light_color = glGetUniformLocation(program->program_id, "light_color"));
    GL(program->ambient_light_fraction = glGetUniformLocation(program->program_id, "ambient_light_fraction"));
    GL(program->ambientness = glGetUniformLocation(program->program_id, "ambientness"));
    GL(program->diffuseness = glGetUniformLocation(program->program_id, "diffuseness"));
    GL(program->specularness = glGetUniformLocation(program->program_id, "specularness"));
    GL(program->specular_unscatterness = glGetUniformLocation(program->program_id, "specular_unscatterness"));
}

void use_lighting_program(LightingProgram *program, GLuint texture_id, hmm_v4 object_color, hmm_mat4 *to_world_space, hmm_mat4 *to_camera_space, hmm_mat4 *perspective_transform, bool lighting_disabled, hmm_v3 light_position, hmm_v3 light_color, f32 ambient_light_fraction, f32 ambientness, f32 diffuseness, f32 specularness, f32 specular_unscatterness)
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
    GL(glUniform3fv(program->light_position, 1, light_position.Elements));
    
    GL(glUniform4fv(program->object_color, 1, (GLfloat*)(object_color.Elements)));
    GL(glUniform1i(program->lighting_disabled, lighting_disabled ? 1 : 0));
    GL(glUniform3fv(program->light_color, 1, (GLfloat*)(light_color.Elements)));
    GL(glUniform1f(program->ambient_light_fraction, ambient_light_fraction));
    GL(glUniform1f(program->ambientness, ambientness));
    GL(glUniform1f(program->diffuseness, diffuseness));
    GL(glUniform1f(program->specularness, specularness));
    GL(glUniform1f(program->specular_unscatterness, specular_unscatterness));
}


GLuint create_texture_blit_program()
{
    char *vertex_source = R"FOO(
    #version 330 core
    
layout (location = 0) in vec2 quad_xy;
layout (location = 1) in vec2 vs_quad_uv;

out vec2 fs_quad_uv;

void main()
{
gl_Position =  vec4(quad_xy, 0.0f, 1.0f);
fs_quad_uv = vs_quad_uv;
}

)FOO";
    
    char *fragment_source = R"FOO(
    #version 330 core
    
    in vec2 fs_quad_uv;
    uniform sampler2D texture2d_sampler;
out vec4 FragColor;

void main()
{
FragColor = texture(texture2d_sampler, fs_quad_uv);
}
)FOO";
    
    GLuint program_id = gl_create_program(vertex_source, fragment_source);
    gl_set_uniform_1i(program_id, "texture2d_sampler", 0);
    return program_id;
}


void use_texture_blit_program(GLuint program_id, GLuint texture_id)
{
    GL(glUseProgram(program_id));
    GL(glActiveTexture(GL_TEXTURE0)); 
    GL(glBindTexture(GL_TEXTURE_2D, texture_id));
}