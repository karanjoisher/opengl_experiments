#pragma once
#include "gl_programs.h"
#include "render.h"
#include "open_gl.h"

// TODO(Karan): Think about how State needs to be organized
// A bunch of shit has been dumped here and I think it needs to be separated but I can't think of a starting point yet. Let's finish 3d model loading first and then clean this up
struct State
{
    LightingProgram lighting_program;
    GLuint texture_blit_program;
    
    GLInterleavedAttributesVAO xyz_uv_nxnynz;
    GLInterleavedAttributesVAO xy_uv;
    
    GLVertexAttributesData quad_xy_uv;
    
    hmm_v3 model_translation;
    hmm_v3 model_rotation;
    f32 model_scale;
    
    Memory memory;
    Model *test_model;
    Model *test_cube_model;
    
    f32 aspect_ratio;
    bool show_demo_window;
    
    Camera camera;
    f32 camera_speed_per_sec;
    f32 camera_sensitivity;
    
    hmm_v3 light_colors[MAX_LIGHT_COLORS];
    hmm_v3 light_position;
    
    // NOTE(Karan): These properties correspond to creation of a floating point framebuffer which can be used to debug vectors as it provides  negative values as well (default framebuffer clips negative values to 0)
    b32 enable_debug_fbo;
    b32 enable_debug_fbo_on_next_frame;
    GLuint debug_fbo;
    GLuint debug_fbo_color_buffer_texture_id;
    GLuint debug_fbo_width;
    GLuint debug_fbo_height;
};
