#include "render.h"
#include "logging.h"

void create_perspective_transform(hmm_mat4 *result, f32 near_plane_distance, f32 far_plane_distance, f32 fov_radians, f32 aspect_ratio, PerspectiveTransformCoordinateSystemOption coordinate_system_type)
{
    // WARNING(Karan): result must be zero cleared before hand!!
    
    ASSERT(near_plane_distance > 0, "near_plane_distance is a distance value and hence should always be positive");
    ASSERT(far_plane_distance > 0, "far_plane_distance is a distance value and hence should always be positive");
    ASSERT(fov_radians > 0, "fov_radians is the absolute angle from the viewing axis to side plane");
    ASSERT(aspect_ratio > 0, "aspect_ratio must be positive");
    
    f32 multiplier = coordinate_system_type == RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS ? 1.0f : -1.0f;
    f32 n = multiplier*near_plane_distance;
    f32 f = multiplier*far_plane_distance;
    aspect_ratio = multiplier * aspect_ratio;
    f32 half_fov_radians = multiplier*(fov_radians/2.0f);
    f32 tan_half_fov_radians = HMM_TanF(half_fov_radians);
    //1st column
    result->Elements[0][0] = -1.0f/tan_half_fov_radians;
    
    //2nd column
    result->Elements[1][1] = aspect_ratio/tan_half_fov_radians;
    
    //3rd column
    result->Elements[2][2] = multiplier * ((f+n)/(f-n));
    result->Elements[2][3] = multiplier;
    //4th column
    result->Elements[3][2] = multiplier * ((-2.0f*f*n)/(f-n));
    
#ifdef ASSERTS_ON
    // NOTE(Karan): Added this to make sure the matrix derived using FOV and aspect ratio matches my derivation which uses l,r,t,b
    
    f32 width = 2.0f * near_plane_distance * HMM_TanF(fov_radians/2.0f); // Note that we are using ABSOLUTE values so width will be positive
    f32 height = width/aspect_ratio;
    f32 l = multiplier*(width/2.0f);
    f32 r = -l;
    f32 t = height/2.0f;
    f32 b = -t;
    
    hmm_mat4 compare_to_this = {};
    compare_to_this.Elements[0][0] = multiplier * ((2.0f*n)/(r-l));
    compare_to_this.Elements[1][1] = multiplier * ((2.0f*n)/(t-b));
    compare_to_this.Elements[2][0] = multiplier * (-(r+l)/(r-l));
    compare_to_this.Elements[2][1] = multiplier * (-(t+b)/(t-b));
    compare_to_this.Elements[2][2] = multiplier * ((f+n)/(f-n));
    compare_to_this.Elements[2][3] = multiplier;
    compare_to_this.Elements[3][2] = multiplier * ((-2.0f*f*n)/(f-n));
    
    f32 threshold_difference = 0.0001f;
    for(u32 i = 0; i < 4; i++)
    {
        for(u32 j = 0; j < 4; j++)
        {
            ASSERT(HMM_ABS(result->Elements[i][j] - compare_to_this.Elements[i][j]) <= threshold_difference, "Perspective Transform matrix values seem to be incorrect");
        }
    }
#endif
}
