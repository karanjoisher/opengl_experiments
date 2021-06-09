#pragma once
#include <GL\gl.h>
#include "render.h"

struct TextureOpsUniformLocations
{
    GLuint sampler_unit;
    GLuint texture_blend;
    GLuint uv_channel;
    GLuint operation;
};

struct LightingProgram
{
    GLuint program_id;
    GLuint to_world_space;
    GLuint to_camera_space;
    GLuint mesh_transform;
    GLuint perspective_projection;
    GLuint light_position;
    
    GLuint base_colors;
    GLuint num_sampler_units;
    GLuint num_texture_ops;
    TextureOpsUniformLocations texture_ops[aiTextureType_UNKNOWN][32];
    GLuint is_lighting_disabled;
    GLuint light_colors;
    GLuint ambient_light_fraction;
    GLuint skybox_sampler;
};

struct SkyboxProgram
{
    GLuint program_id;
    GLuint to_camera_space;
    GLuint perspective_projection;
};

struct DebugNormalVisualizationProgram
{
    GLuint program_id;
    GLuint mesh_transform;
    GLuint to_world_space;
    GLuint to_camera_space;
    GLuint perspective_projection;
    GLuint vector_length;
};


void use_texture_blit_program(GLuint program_id, GLuint texture_id);
void use_lighting_program(LightingProgram *program, hmm_mat4 *mesh_transform, hmm_mat4 *to_world_space, hmm_mat4 *to_camera_space, hmm_mat4 *perspective_transform, Material* material,  bool is_lighting_disabled, hmm_v3 light_position, hmm_v3 light_colors[MAX_LIGHT_COLORS], GLuint skybox_cubemap_id);
void use_debug_normal_visualization_program(DebugNormalVisualizationProgram *program, hmm_mat4 *mesh_transform, hmm_mat4 *to_world_space, hmm_mat4 *to_camera_space, hmm_mat4 *perspective_transform, f32 vector_length);