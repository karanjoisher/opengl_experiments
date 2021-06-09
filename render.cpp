#include "render.h"
#include "logging.h"
#include "memory.h"
#include "math.h"
#include "gl_programs.h"

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


// TODO(Karan): Very hacky 3d model loading! REFER TO NOTES IN render.h !!!!!!
void temp_load_node_tree(aiNode *ai_node, Node* parent, Node *node, Memory* memory)
{
    if(ai_node && node)
    {
        s32 name_len = len_cstring(ai_node->mName.C_Str());
        node->name = PUSH_ARRAY(memory, char, name_len + 1);
        COPY(ai_node->mName.C_Str(), node->name, name_len + 1);
        node->parent = parent;
        
        ai_get_column(&(ai_node->mTransformation), 3, &(node->additional_translation));
        
        hmm_v3 standard_axis[3] = {};
        standard_axis[X_AXIS].X = 1.0f;
        standard_axis[Y_AXIS].Y = 1.0f;
        standard_axis[Z_AXIS].Z = 1.0f;
        
        hmm_v3 rotated_axis = {};
        ai_get_row(&(ai_node->mTransformation), X_AXIS, &rotated_axis);
        node->additional_rotation_degrees.X = HMM_GetAngleDegreesBetween(&rotated_axis, &(standard_axis[X_AXIS]));
        
        ai_get_row(&(ai_node->mTransformation), Y_AXIS, &rotated_axis);
        node->additional_rotation_degrees.Y = HMM_GetAngleDegreesBetween(&rotated_axis, &(standard_axis[Y_AXIS]));
        
        ai_get_row(&(ai_node->mTransformation), Z_AXIS, &rotated_axis);
        node->additional_rotation_degrees.Z = HMM_GetAngleDegreesBetween(&rotated_axis, &(standard_axis[Z_AXIS]));
        
        if(ai_node->mNumMeshes > 0 && ai_node->mMeshes)
        {
            node->mesh_indices = PUSH_ARRAY(memory, s32, ai_node->mNumMeshes);
            if(node->mesh_indices)
            {
                node->num_meshes = ai_node->mNumMeshes;
                COPY(ai_node->mMeshes, node->mesh_indices, sizeof(s32) * node->num_meshes);
            }
        }
        
        node->children = PUSH_ARRAY(memory, Node, ai_node->mNumChildren);
        if(node->children)
        {
            node->num_children = ai_node->mNumChildren;
            for(s32 i = 0; i < node->num_children; i++)
            {
                Node *child = node->children + i;
                aiNode *child_ai_node = ai_node->mChildren[i];
                temp_load_node_tree(child_ai_node, node, child, memory);
            }
        }
    }
}

Model *temp_load_obj(char* filename, Memory *memory, GLInterleavedAttributesVAO xyz_uv_nxnynz)
{
    Model *model = 0;
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        LOG_ERR("Assimp failed to import file %s: %s", filename,  importer.GetErrorString());
        return model;
    }
    
    model = PUSH_TYPE(memory, Model);
    if(!model)
    {
        return model;
    }
    
    // Load materials
    model->materials = PUSH_ARRAY(memory, Material, scene->mNumMaterials);
    s32 textures_loaded = 0;
    const char *texture_paths_loaded[MAX_TEXTURES_PER_MODEL] = {};
    GLuint texture_ids[MAX_TEXTURES_PER_MODEL] = {};
    
    if(model->materials)
    {
        model->num_materials = scene->mNumMaterials;
        for(s32 i = 0; i < model->num_materials; i++)
        {
            aiMaterial *ai_material = scene->mMaterials[i];
            
            Material *material = model->materials + i;
            s32 name_len = len_cstring(ai_material->GetName().C_Str());
            material->name = PUSH_ARRAY(memory, char, name_len + 1);
            COPY(ai_material->GetName().C_Str(), material->name, name_len + 1);
            
            // Get base colors for all material types
            aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_DIFFUSE,  (aiColor4D*)&material->base_colors[aiTextureType_DIFFUSE]);
            aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_AMBIENT, (aiColor4D*)&material->base_colors[aiTextureType_AMBIENT]);
            aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_SPECULAR, (aiColor4D*)&material->base_colors[aiTextureType_SPECULAR]);
            aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_EMISSIVE, (aiColor4D*)&material->base_colors[aiTextureType_EMISSIVE]);
            
            f32 shininess = 0.0f;
            aiGetMaterialFloat(ai_material,AI_MATKEY_SHININESS, &shininess);
            shininess /= 255.0f;
            material->base_colors[aiTextureType_SHININESS].X = shininess;
            material->base_colors[aiTextureType_SHININESS].Y = shininess;
            material->base_colors[aiTextureType_SHININESS].Z = shininess;
            material->base_colors[aiTextureType_SHININESS].W = 1.0f;
            
            s32 samples_bound = 0;
            GLuint sampler_units_to_texture_id[MAX_SAMPLER_UNITS] = {};
            
            // Get texture operations for each type of material
            for(aiTextureType texture_type = aiTextureType_NONE; texture_type < aiTextureType_UNKNOWN; texture_type = (aiTextureType)((u32)texture_type+1))
            {
                u32 texture_stack_length = aiGetMaterialTextureCount(ai_material, texture_type);
                s32 num_texture_ops = 0;
                TextureOp *texture_ops = PUSH_ARRAY(memory, TextureOp, texture_stack_length);
                if(texture_ops)
                {
                    for(u32 j = 0; j < texture_stack_length; j++)
                    {
                        aiString texture_path; u32 uv_channel = 0; f32 texture_blend = 1.0f; aiTextureOp texture_op = (aiTextureOp)((u32)aiTextureOp_SignedAdd + 1); GLuint texture_id = 0;
                        aiGetMaterialTexture(ai_material, texture_type, j, &texture_path, 0, &uv_channel, &texture_blend, &texture_op, 0, 0);
                        
                        if(texture_path.length > 0 && texture_path.C_Str())
                        {
                            // Check if texture is already loaded
                            s32 index = -1;
                            for(s32 k = 0; k < textures_loaded; k++)
                            {
                                if(strcmp(texture_paths_loaded[k], texture_path.C_Str()) == 0)
                                {
                                    index = k;
                                    texture_id = texture_ids[k];
                                    break;
                                }
                            }
                            
                            // If not loaded then load it
                            if((index < 0) && (textures_loaded < ARRAY_LENGTH(texture_paths_loaded)))
                            {
                                if(!scene->GetEmbeddedTexture(texture_path.C_Str()))
                                {
                                    u32 length = (u32)strlen(texture_path.C_Str());
                                    char *filepath = PUSH_ARRAY(memory, char, length + 1);
                                    COPY(texture_path.C_Str(), filepath, length);
                                    if(filepath)
                                    {
                                        texture_paths_loaded[textures_loaded] = filepath;
                                        texture_ids[textures_loaded] = gl_create_texture2d(texture_path.C_Str());
                                        texture_id = texture_ids[textures_loaded];
                                        ++textures_loaded;
                                    }
                                }
                                else
                                {
                                    // TODO(Karan): Handle embedded textures
                                    LOG_ERR("Currently model loading does not handle embedded textures");
                                }
                            }
                        }
                        
                        s32 sampler_unit = -1; 
                        if(texture_id != 0)
                        {
                            // Check if texture is already bound to a SAMPLER UNIT
                            for(s32 k = 0; k < samples_bound; k++)
                            {
                                if(sampler_units_to_texture_id[k] == texture_id)
                                {
                                    sampler_unit = k;
                                    break;
                                }
                            }
                            
                            // If not then bind to a SAMPLER UNIT
                            if((sampler_unit < 0) && (samples_bound < ARRAY_LENGTH(sampler_units_to_texture_id)))
                            {
                                sampler_unit = samples_bound++;
                                sampler_units_to_texture_id[sampler_unit] = texture_id;
                            }
                        }
                        
                        // DEFINE THE OPERATION FOR THE TEXTURE BOUND TO THE SAMPLER UNIT
                        if(sampler_unit != -1)
                        {
                            texture_ops[num_texture_ops].sampler_unit = sampler_unit;
                            texture_ops[num_texture_ops].uv_channel = uv_channel;
                            texture_ops[num_texture_ops].texture_blend = texture_blend;
                            texture_ops[num_texture_ops].operation = (s32)texture_op;
                            ++num_texture_ops;
                        }
                    }
                    
                    material->texture_ops[texture_type] = texture_ops;
                    material->num_texture_ops[texture_type] = num_texture_ops;
                }
            }
            
            material->sampler_units_to_texture_id = PUSH_ARRAY(memory, GLuint, samples_bound);
            if(material->sampler_units_to_texture_id)
            {
                material->num_sampler_units = samples_bound;
                COPY(sampler_units_to_texture_id, material->sampler_units_to_texture_id, samples_bound * sizeof(GLuint));
            }
            
#if 0
            char *temp_material_type_names[] = {"NONE", "DIFFUSE", "SPECULAR", "AMBIENT", "EMISSIVE", "HEIGHT", "NORMALS", "SHININESS", "OPACITY", "DISPLACEMENT", "LIGHTMAP", "REFLECTION", "BASE_COLOR", "NORMAL_CAMERA", "EMISSION_COLOR", "METALNESS", "ROUGHNESS", "AMBIENT_OCCLUSION"};
            char *temp_texture_op_names[] = {"MULTIPLY", "ADD", "SUBTRACT", "DIVIDE", "SMOOTH_ADD", "SIGNED_ADD", "OVERWRITE"};
            
            LOG("Material: %s\n", ai_material->GetName().C_Str());
            LOG("BASE COLORS:\n");
            for(s32 x = 0; x < ARRAY_LENGTH(material->base_colors); x++)
            {
                if(material->base_colors[x].X >= 0.001f && material->base_colors[x].Y >= 0.001f && material->base_colors[x].Z >= 0.001f)
                    LOG("\t%s: %.2f %.2f %.2f %.2f\n", temp_material_type_names[x], material->base_colors[x].X, material->base_colors[x].Y, material->base_colors[x].Z, material->base_colors[x].W);
            }
            
            LOG("%d SAMPLER UNITS ALLOCATED\n", material->num_sampler_units);
            for(s32 x = 0; x < material->num_sampler_units; x++)
            {
                LOG("\tSAMPLER UNIT %d: TEXTURE ID %d\n", x, material->sampler_units_to_texture_id[x]);
            }
            
            for(s32 x = 0; x < ARRAY_LENGTH(material->texture_ops); x++)
            {
                TextureOps* _texture_ops = material->texture_ops[x];
                if(_texture_ops)
                {
                    LOG("%d operations for %s TEXTURE STACK\n", material->num_texture_ops[x], temp_material_type_names[x]);
                    for(s32 y = 0; y < material->num_texture_ops[x]; y++)
                    {
                        TextureOps *texture_op = _texture_ops + y;
                        LOG("\tOP %d: SAMPLER UNIT=%d, TEXTURE_BLEND=%.2f, UV_CHANNEL=%d, OPERATION=%s\n", y, texture_op->sampler_unit, texture_op->texture_blend, texture_op->uv_channel, temp_texture_op_names[texture_op->operation]);
                    }
                }
            }
            LOG("-----------------------------------------------------------\n");
#endif
        }
    }
    
#if 0
    for(s32 x = 0; x < textures_loaded; x++)
    {
        LOG("%d TEXTURE_ID = %s\n", texture_ids[x], texture_paths_loaded[x]);
    }
#endif
    
    // Load meshes
    model->meshes = PUSH_ARRAY(memory, Mesh, scene->mNumMeshes);
    if(model->meshes)
    {
        model->num_meshes = scene->mNumMeshes;
        for(s32 i = 0; i < model->num_meshes; i++)
        {
            aiMesh* ai_mesh = scene->mMeshes[i];
            Mesh* mesh = model->meshes + i;
            s32 num_verticies = ai_mesh->mNumVertices;
            
            // TODO(Karan): Currently we can only use xyz uv nxnynz format. Make this generic.
            mesh->vertex_attributes_data_format_vao = xyz_uv_nxnynz;
            s32 vertex_size = 8*sizeof(f32);
            u8* vertex_data_buffer = PUSH_ARRAY(memory, u8, vertex_size * num_verticies);
            
            // TODO(Karan): Currently only allows for TRIANGLE indicies. Handle all primitives.
            s32 num_indicies = 3 * ai_mesh->mNumFaces;
            u32 *index_buffer = PUSH_ARRAY(memory, u32, num_indicies);
            
            mesh->material_index = ai_mesh->mMaterialIndex;
            
            if(vertex_data_buffer && index_buffer)
            {
                for(s32 j = 0; j < num_verticies; j++)
                {
                    f32 *attribute_data = (f32*)(vertex_data_buffer + (j*vertex_size));
                    if(ai_mesh->mVertices)
                    {
                        COPY(ai_mesh->mVertices + j, attribute_data, sizeof(f32)*3);
                    }
                    attribute_data += 3;
                    
                    if(ai_mesh->mTextureCoords[0])
                    {
                        COPY(ai_mesh->mTextureCoords[0] + j, attribute_data, sizeof(f32)*2);
                    }
                    attribute_data += 2;
                    
                    if(ai_mesh->mNormals)
                    {
                        COPY(ai_mesh->mNormals + j, attribute_data, sizeof(f32)*3);
                    }
                }
                
                for(u32 j = 0; j < ai_mesh->mNumFaces; j++)
                {
                    u32 *triangle_indicies = index_buffer + (j*3);
                    aiFace* ai_face = ai_mesh->mFaces + j;
                    COPY(ai_face->mIndices, triangle_indicies, sizeof(u32) * 3);
                }
                
                mesh->vertex_attributes_data = gl_create_vertex_attributes_data(vertex_data_buffer, vertex_size * num_verticies, index_buffer, sizeof(u32) * num_indicies);
            }
        }
    }
    
    model->root = PUSH_TYPE(memory, Node);
    if(model->root)
    {
        temp_load_node_tree(scene->mRootNode, 0, model->root, memory);
    }
    
    return model;
}

void temp_draw_node_tree(Model *model, Node *node, hmm_mat4 *cummulated_transforms_from_root_to_parent, ProgramType program_type, void *program_data)
{
    
    if(node)
    {
        hmm_mat4 cummulated_transforms_from_root_to_node = *cummulated_transforms_from_root_to_parent * HMM_Translate(node->additional_translation) * Z_ROTATE(node->additional_rotation_degrees.Z) * Y_ROTATE(node->additional_rotation_degrees.Y) * X_ROTATE(node->additional_rotation_degrees.X);
        for(s32 i = 0; i < node->num_meshes; i++)
        {
            Mesh* mesh = model->meshes + node->mesh_indices[i];
            if(mesh)
            {
                Material empty_material = {};
                Material *material = &empty_material;
                u32 material_index = mesh->material_index;
                if(model->materials && model->materials + material_index) material = model->materials + material_index;
                
                gl_bind_vao(&(mesh->vertex_attributes_data_format_vao), &(mesh->vertex_attributes_data));
                
                
                switch(program_type)
                {
                    case LIGHTING_PROGRAM:
                    {
                        LightingProgramData *data = (LightingProgramData*)program_data;
                        use_lighting_program(data->program, &cummulated_transforms_from_root_to_node, data->to_world_space, data->to_camera_space, data->perspective_transform, material, data->is_lighting_disabled, data->light_position, data->light_colors, data->skybox_cubemap_id);
                    }break;
                    case DEBUG_NORMAL_VISUALIZATION_PROGRAM:
                    {
                        DebugNormalVisualizationProgramData *data = (DebugNormalVisualizationProgramData*)program_data;
                        use_debug_normal_visualization_program(data->program, &cummulated_transforms_from_root_to_node, data->to_world_space, data->to_camera_space, data->perspective_transform, data->vector_length);
                    }break;
                    default:
                    {
                        ASSERT(false, "Invalid program type:%d while drawing model", program_type); 
                    }
                }
                
                GL(glDrawElements(GL_TRIANGLES, mesh->vertex_attributes_data.num_indices, GL_UNSIGNED_INT, 0));
            }
        }
        
        for(s32 i = 0; i < node->num_children; i++)
        {
            temp_draw_node_tree(model, node->children + i, &cummulated_transforms_from_root_to_node, program_type, program_data);
        }
    }
}

void temp_draw_model(Model *model, ProgramType program_type, void *program_data)
{
    hmm_mat4 identity = HMM_Mat4d(1.0f);
    temp_draw_node_tree(model, model->root, &identity, program_type, program_data);
}