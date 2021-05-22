#pragma once
#include <HandmadeMath.h>

#define X_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{1.0f, 0.0f, 0.0f})
#define Y_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{0.0f, 1.0f, 0.0f})
#define Z_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{0.0f, 0.0f, 1.0f})