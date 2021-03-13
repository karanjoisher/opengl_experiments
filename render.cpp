#include "render.h"

void __create_perspective_projection_using_plane_coordinates(PerspectiveProjection *p)
{
    f32 rml = p->r - p->l;
    f32 rpl = p->r + p->l;
    f32 tmb = p->t - p->b;
    f32 tpb = p->t + p->b;
    f32 fmn = p->f - p->n;
    f32 fpn = p->f + p->n;
    
    //1st column
    p->transform.Elements[0][0] = -(2.0f*p->n)/rml;
    //2nd column              
    p->transform.Elements[1][1] = -(2.0f * p->n)/tmb;
    //3rd column
    p->transform.Elements[2][0] = (rpl/rml);
    p->transform.Elements[2][1] = (tpb/tmb);
    p->transform.Elements[2][2] = -(fpn/fmn);
    p->transform.Elements[2][3] = -1.0f;
    //4th column
    p->transform.Elements[3][2] = (2.0f * p->f * p->n)/fmn;
}

void create_perspective_projection_using_fov_y_and_aspect_ratio(PerspectiveProjection *p)
{
    f32 width = 2.0f * HMM_ABS(p->n) * HMM_TanF(p->fov_y_radians/2.0f);
    f32 height = width/p->aspect_ratio;
    p->l = -width/2.0f;
    p->b = -height/2.0f;
    p->r = width + p->l;
    p->t = height + p->b;
    __create_perspective_projection_using_plane_coordinates(p);
}


void create_perspective_projection_using_plane_coordinates(PerspectiveProjection *p)
{
    f32 width = HMM_ABS(p->r - p->l);
    f32 height = HMM_ABS(p->t - p->b);
    p->fov_y_radians = 2.0f * HMM_ATanF(width/(2.0f * HMM_ABS(p->n)));
    p->aspect_ratio = width/height;
    __create_perspective_projection_using_plane_coordinates(p);
}
