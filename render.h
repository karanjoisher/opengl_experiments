#pragma once
#include <HandmadeMath.h>

#define X_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{1.0f, 0.0f, 0.0f})
#define Y_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{0.0f, 1.0f, 0.0f})
#define Z_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{0.0f, 0.0f, 1.0f})

struct PerspectiveProjection
{
    f32 n;
    f32 f;
    f32 l;
    f32 r;
    f32 t;
    f32 b;
    f32 aspect_ratio; // width = abs(r-l) height = abs(t-b), aspect_ratio = width/height
    f32 fov_y_radians;
    hmm_mat4 transform;
};

