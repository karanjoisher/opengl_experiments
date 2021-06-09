#pragma once
#include <HandmadeMath.h>
#include "memory.h"
#include "open_gl.h"

// TODO(Karan): Randomly selected constants, in future models should not be limited by these constants.
#define MAX_TEXTURES_PER_MODEL 64
#define MAX_TEXTURE_OPERATIONS 13
#define MAX_SAMPLER_UNITS 32

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

enum LightColor
{
    DIFFUSE_LIGHT_COLOR,
    SPECULAR_LIGHT_COLOR,
    AMBIENT_LIGHT_COLOR,
    MAX_LIGHT_COLORS
};

enum ProgramType
{
    LIGHTING_PROGRAM,
    DEBUG_NORMAL_VISUALIZATION_PROGRAM
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

void create_perspective_transform(hmm_mat4 *result, f32 near_plane_distance, f32 far_plane_distance, f32 fov_radians, f32 aspect_ratio, CameraCoordinateSystemLookingDirection looking_direction);
void create_to_camera_space_transform(hmm_mat4 * result, Camera *camera);
void set_rotation(Camera *camera, hmm_v3 *rotation);
void rotate_camera(Camera *camera, hmm_v3 *rotation);

/* 
 TODO(Karan): FOLLOWING  IS A VERY BAD STRUCTURE AND CODE FOR LOADING AND DEFINING 3D MODELS. THIS IS A FIRST PASS!!!
 
Issues with current model loading:

Issues with VAO:
- No generic way to create VAOs. We are only allowing xyz-uv-nxnynz currently.
- Duplication of VAO structs. After we solve the issues of creating generic VAOs we need to be able to find the correct VAO from the pool of existing VAOs and use that, if not present then create a new VAO and add that to the pool of VAOs.

Issues with Vertex data, Materials, Textures & ASSET storage:
- Vertex data, Materials, Textures remains same per model (except maybe for scenarios where objects are deformed?? E.G. Car crashed). This kind of data needs to be created only once per model. Instances of this model must refer the same data. (For scenarios where mesh maybe deformed we can probably create a new model and the instances will refer the new model???)
- All this can be thought of as ASSET data. Application can reserve some memory for the asset data and then page in/page out resources as required. This would _REQUIRE_ that Models _DO NOT_ directly reference; lets say; texureids, VAO ids, pointers to data etc otherwise we _CANNOT_ evict resources (If we do then Models will hold stale pointers/OpenGL ids). Watch Casey Muratori's ASSET STREAMING videos to better understand solution for this requirement.

Nodes & Meshes:
- Nodes store references to Meshes it contains, and this too remains same for each model. Nodes in instances of this model must refer the same mesh indicies array.
- Similarly meshes store reference to Material it uses. The above point applies to Mesh & Material relation as well.

Draw:
- Need to study on what is the right way to dispatch draw calls

Read on what are good approaches to optimize data storage for models and also draw calls performance.
*/

struct Mesh
{
    GLInterleavedAttributesVAO vertex_attributes_data_format_vao;
    GLVertexAttributesData vertex_attributes_data;
    u32 material_index;
};

struct Node
{
    char *name;
    Node *parent;
    s32 num_children;
    Node *children;
    
    hmm_v3 additional_translation;
    hmm_v3 additional_rotation_degrees;
    
    s32 num_meshes;
    s32 *mesh_indices;
};

struct TextureOp
{
    s32 sampler_unit;
    f32 texture_blend;
    s32 uv_channel;
    s32 operation;
};

struct Material
{
    char *name;
    hmm_vec4 base_colors[aiTextureType_UNKNOWN];
    s32 num_sampler_units;
    GLuint *sampler_units_to_texture_id;
    s32 num_texture_ops[aiTextureType_UNKNOWN];
    TextureOp *texture_ops[aiTextureType_UNKNOWN];
};

struct LightingProgram;
struct LightingProgramData
{
    LightingProgram *program;
    hmm_mat4 *to_world_space; 
    hmm_mat4 *to_camera_space; 
    hmm_mat4 *perspective_transform;
    bool is_lighting_disabled; 
    hmm_v3 light_position; 
    hmm_v3 light_colors[MAX_LIGHT_COLORS]; 
    GLuint skybox_cubemap_id;
};

struct DebugNormalVisualizationProgram;
struct DebugNormalVisualizationProgramData
{
    DebugNormalVisualizationProgram* program;
    hmm_mat4 *to_world_space; 
    hmm_mat4 *to_camera_space; 
    hmm_mat4 *perspective_transform;
    f32 vector_length;
    hmm_v3 vector_color;
};

struct Model
{
    Node *root;
    s32 num_meshes;
    Mesh *meshes;
    s32 num_materials;
    Material *materials;
};
