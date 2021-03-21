#pragma once
#include <HandmadeMath.h>

enum CameraCoordinateSystemLookingDirection
{
    RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS = -1,
    RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS = 1
};

enum Axis
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS,
    
    NUM_AXIS
};


/* DILEMMA(Karan): Camera structure design dilemma

Design 1: We store only camera position and its 3 axis in Camera structure
- Can directly read/write position
- Moving camera requires upto 3 v3 adds
- Reading rotation requires 3 dot products and 3 ACosF
- Writing rotation requires 3-4 mat4 multiplies
- Can directly read/write camera axis values
- Computing camera space transform requires 2 mat4 multiplies

Design 2: We store only camera position and rotation values
- Can directly read/write position
- Moving camera requires computing camera's axis(3 mat4 multiplies) and upto 3 v3 adds
- Can directly read/write rotation
- Reading camera axis requires 3 mat4 multiplies
- Setting camera axis requires 3 dot products and 3 ACosF
- Computing camera space transform requires computing camera's axis(3 mat4 multiplies) and 2 mat4 multiplies : total 5 mat multiplies
Design 3: We store camera pos, rotation, 3 axis
- Can directly read/write position
- Moving camera requires upto 3 v3 adds
- Can directly read/write rotation
- Can directly read/write camera axis values
- Computing camera space transform requires 2 mat4 multiplies
- Rotation AND Axis writes must go through a function which updates the state, if they are updated directly then they won't be in-sync!

TODO(Karan): Going with design 3 since it has cost savings of both design 1 & 2, just need to be careful to use the function for writes to rotation and axis*/
struct Camera
{
    hmm_v3 pos;
    hmm_v3 rotation;
    hmm_v3 axis[NUM_AXIS];
    
    // Perspective projection properties
    f32 near_plane_distance;
    f32 far_plane_distance;
    f32 fov_radians;
    f32 aspect_ratio;
    CameraCoordinateSystemLookingDirection looking_direction;
};
