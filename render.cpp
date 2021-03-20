#include "render.h"
#include "logging.h"

void create_perspective_transform(hmm_mat4 *result, f32 near_plane_distance, f32 far_plane_distance, f32 fov_radians, f32 aspect_ratio, CameraCoordinateSystemLookingDirection looking_direction)
{
    // WARNING(Karan): result must be zero cleared before hand!!
    
    ASSERT(near_plane_distance > 0, "near_plane_distance is a distance value and hence should always be positive");
    ASSERT(far_plane_distance > 0, "far_plane_distance is a distance value and hence should always be positive");
    ASSERT(fov_radians > 0, "fov_radians is the absolute angle from the viewing axis to side plane");
    ASSERT(aspect_ratio > 0, "aspect_ratio must be positive");
    
    f32 n = looking_direction*near_plane_distance;
    f32 f = looking_direction*far_plane_distance;
    f32 half_fov_radians = looking_direction*(fov_radians/2.0f);
    f32 tan_half_fov_radians = HMM_TanF(half_fov_radians);
    //1st column
    result->Elements[0][0] = -1.0f/tan_half_fov_radians;
    
    //2nd column
    result->Elements[1][1] = looking_direction * (aspect_ratio/tan_half_fov_radians);
    
    //3rd column
    result->Elements[2][2] = looking_direction * ((f+n)/(f-n));
    result->Elements[2][3] = (f32)looking_direction;
    //4th column
    result->Elements[3][2] = looking_direction * ((-2.0f*f*n)/(f-n));
    
#ifdef ASSERTS_ON
    // NOTE(Karan): Added this to make sure the matrix derived using FOV and aspect ratio matches my derivation which uses l,r,t,b
    
    f32 width = 2.0f * near_plane_distance * HMM_TanF(fov_radians/2.0f); // Note that we are using ABSOLUTE values so width will be positive
    f32 height = width/aspect_ratio;
    f32 l = looking_direction*(width/2.0f);
    f32 r = -l;
    f32 t = height/2.0f;
    f32 b = -t;
    
    hmm_mat4 compare_to_this = {};
    compare_to_this.Elements[0][0] = looking_direction * ((2.0f*n)/(r-l));
    compare_to_this.Elements[1][1] = looking_direction * ((2.0f*n)/(t-b));
    compare_to_this.Elements[2][0] = looking_direction * (-(r+l)/(r-l));
    compare_to_this.Elements[2][1] = looking_direction * (-(t+b)/(t-b));
    compare_to_this.Elements[2][2] = looking_direction * ((f+n)/(f-n));
    compare_to_this.Elements[2][3] = (f32)looking_direction;
    compare_to_this.Elements[3][2] = looking_direction * ((-2.0f*f*n)/(f-n));
    
    f32 threshold_difference = 0.0001f;
    for(u32 i = 0; i < 4; i++)
    {
        for(u32 j = 0; j < 4; j++)
        {
            
            ASSERT(HMM_ABS(result->Elements[i][j] - compare_to_this.Elements[i][j]) <= threshold_difference, "Perspective Transform matrix values seem to be incorrect | Element[%d][%d] fov_asp version: %.3f, lrtb version: %.3f", j, i, result->Elements[i][j], compare_to_this.Elements[i][j]);
        }
    }
#endif
}

void create_to_camera_space_transform(hmm_mat4 * result, Camera *camera)
{
    hmm_mat4 camera_axis = {};
    HMM_SetRow(&camera_axis, X_AXIS, &camera->axis[X_AXIS]);
    HMM_SetRow(&camera_axis, Y_AXIS, &camera->axis[Y_AXIS]);
    HMM_SetRow(&camera_axis, Z_AXIS, &camera->axis[Z_AXIS]);
    camera_axis.Elements[3][3] = 1.0f;
    
    *result = camera_axis * HMM_Translate(-camera->pos);
}

void set_rotation(Camera *camera, hmm_v3 *rotation)
{
    camera->rotation = *rotation;
    hmm_mat4 standard_axis_rotated = Z_ROTATE(camera->rotation.Z) * Y_ROTATE(camera->rotation.Y) * X_ROTATE(camera->rotation.X);
    camera->axis[X_AXIS] = {standard_axis_rotated.Elements[X_AXIS][0], standard_axis_rotated.Elements[X_AXIS][1], standard_axis_rotated.Elements[X_AXIS][2]};
    camera->axis[Y_AXIS] = {standard_axis_rotated.Elements[Y_AXIS][0], standard_axis_rotated.Elements[Y_AXIS][1], standard_axis_rotated.Elements[Y_AXIS][2]};
    camera->axis[Z_AXIS] = {standard_axis_rotated.Elements[Z_AXIS][0], standard_axis_rotated.Elements[Z_AXIS][1], standard_axis_rotated.Elements[Z_AXIS][2]};
}

void rotate_camera(Camera *camera, hmm_v3 *rotation)
{
    camera->rotation += *rotation;
    set_rotation(camera, &camera->rotation);
}