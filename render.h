#pragma once
#include <HandmadeMath.h>

#define X_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{1.0f, 0.0f, 0.0f})
#define Y_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{0.0f, 1.0f, 0.0f})
#define Z_ROTATE(angle) HMM_Rotate(angle, hmm_vec3{0.0f, 0.0f, 1.0f})

enum PerspectiveTransformCoordinateSystemOption
{
    RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS,
    RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS
};



