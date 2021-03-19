#pragma once
#include <HandmadeMath.h>

enum PerspectiveTransformCoordinateSystemOption
{
    RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS,
    RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS
};

struct Camera
{
    hmm_v3 pos;
    hmm_v3 axis[3];
};
