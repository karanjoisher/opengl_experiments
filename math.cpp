#include "math.h"

HMM_INLINE float HMM_PREFIX(ToDegrees)(float Radians)
{
    float Result = Radians * (180.0f / HMM_PI32);
    return (Result);
}

HMM_INLINE void HMM_PREFIX(SetRow)(hmm_mat4 *mat, u32 row, hmm_vec3 *row_values)
{
    mat->Elements[0][row] = row_values->X;
    mat->Elements[1][row] = row_values->Y;
    mat->Elements[2][row] = row_values->Z;
}
