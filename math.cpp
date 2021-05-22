#include "math.h"
#include "assimp/matrix4x4.h"

HMM_INLINE f32 HMM_PREFIX(ToDegrees)(f32 Radians)
{
    f32 Result = Radians * (180.0f / HMM_PI32);
    return (Result);
}

HMM_INLINE void HMM_PREFIX(SetRow)(hmm_mat4 *mat, u32 row, hmm_vec3 *row_values)
{
    mat->Elements[0][row] = row_values->X;
    mat->Elements[1][row] = row_values->Y;
    mat->Elements[2][row] = row_values->Z;
}

HMM_INLINE void HMM_PREFIX(SetColumn)(hmm_mat4 *mat, u32 col, hmm_vec3 *col_values)
{
    mat->Elements[col][0] = col_values->X;
    mat->Elements[col][1] = col_values->Y;
    mat->Elements[col][2] = col_values->Z;
}

HMM_INLINE void HMM_PREFIX(GetColumn)(hmm_mat4 *mat, u32 col, hmm_vec3 *col_values)
{
    col_values->X = mat->Elements[col][0];
    col_values->Y = mat->Elements[col][1];
    col_values->Z = mat->Elements[col][2];
}


HMM_INLINE f32 HMM_PREFIX(GetAngleDegreesBetween)(hmm_vec3 *vec1, hmm_vec3 *vec2)
{
    f32 dot = HMM_DotVec3(HMM_NormalizeVec3(*vec1), HMM_NormalizeVec3(*vec2));
    f32 radians = HMM_ACosF(dot);
    return HMM_ToDegrees(radians);
}

// Assimp tp HandmadeMath converters
void ai_get_row(aiMatrix4x4 *source, s32 row, hmm_vec3 *dest)
{
    f32 *r = (f32*)(&(source->a1)) + (4*row);
    dest->X = *r;
    dest->Y = *(r+1);
    dest->Z = *(r+2);
}

void ai_get_column(aiMatrix4x4 *source, s32 column, hmm_vec3 *dest)
{
    f32 *col = (f32*)(&(source->a1)) + column;
    
    dest->X = *col;
    dest->Y = *(col+4);
    dest->Z = *(col+8);
}

hmm_mat4 ai_convert_to_hmm_mat4(aiMatrix4x4* ai_mat4)
{
    hmm_mat4 result = {};
    result.Elements[0][0] = ai_mat4->a1;
    result.Elements[1][0] = ai_mat4->a2;
    result.Elements[2][0] = ai_mat4->a3;
    result.Elements[3][0] = ai_mat4->a4;
    
    result.Elements[0][1] = ai_mat4->b1;
    result.Elements[1][1] = ai_mat4->b2;
    result.Elements[2][1] = ai_mat4->b3;
    result.Elements[3][1] = ai_mat4->b4;
    
    result.Elements[0][2] = ai_mat4->c1;
    result.Elements[1][2] = ai_mat4->c2;
    result.Elements[2][2] = ai_mat4->c3;
    result.Elements[3][2] = ai_mat4->c4;
    
    result.Elements[0][3] = ai_mat4->d1;
    result.Elements[1][3] = ai_mat4->d2;
    result.Elements[2][3] = ai_mat4->d3;
    result.Elements[3][3] = ai_mat4->d4;
    
    return result;
}