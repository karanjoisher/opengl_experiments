#include "open_gl.h"
#include "gl_programs.h"

void create_lighting_program(LightingProgram *program)
{
    char *vertex_source = R"FOO(
    #version 430 core
    
layout (location = 0) in vec3 vs_local_space_pos;
layout (location = 1) in vec2 vs_texture_uv;
layout (location = 2) in vec3 vs_normal;

uniform mat4 mesh_transform;
uniform mat4 to_world_space;
uniform mat4 to_camera_space;
uniform mat4 perspective_projection;
uniform vec3 light_position;

out vec2 fs_texture_uv;
out vec3 light_pos_in_camera_space;
out vec3 vertex_pos_in_camera_space;
out vec3 vertex_normal_in_camera_space;
out mat4 to_camera_from_local_space;

void main()
{
to_camera_from_local_space = (to_camera_space * to_world_space  * mesh_transform);

vertex_pos_in_camera_space =  (to_camera_from_local_space * vec4(vs_local_space_pos.xyz, 1.0f)).xyz;
 vertex_normal_in_camera_space = (to_camera_from_local_space * vec4(vs_normal, 0.0f)).xyz;
 //vertex_normal_in_camera_space = mat3(transpose(inverse(to_camera_space * to_world_space))) * vs_normal;
 light_pos_in_camera_space = (to_camera_space * vec4(light_position.xyz, 1.0f)).xyz;
 
gl_Position =  perspective_projection * vec4(vertex_pos_in_camera_space, 1.0f);
fs_texture_uv = vs_texture_uv;
}

)FOO";
    
    char *fragment_source = R"FOO(
        #version 430 core
        
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 32
#define MAX_TEXTURE_OPERATIONS 13
#define MAX_MATERIAL_TEXTURE_TYPES 18
#define MAX_SAMPLER_UNITS 32
#define aiTextureType_DIFFUSE 1
#define aiTextureType_SPECULAR 2
#define aiTextureType_AMBIENT 3
#define aiTextureType_EMISSIVE 4
#define aiTextureType_HEIGHT 5
#define aiTextureType_NORMALS 6
#define aiTextureType_SHININESS 7

#define aiTextureOp_Multiply 0
#define aiTextureOp_Add 1
#define aiTextureOp_Subtract 2
#define aiTextureOp_Divide 3
#define aiTextureOp_SmoothAdd 4
#define aiTextureOp_SignedAdd 5

#define DIFFUSE_LIGHT_COLOR 0
#define SPECULAR_LIGHT_COLOR 1
#define AMBIENT_LIGHT_COLOR 2
#define MAX_LIGHT_COLORS 3

struct TextureOp
{
    int sampler_unit;
    float texture_blend;
    int uv_channel;
    int operation;
}; 

in vec2 fs_texture_uv;
in vec3 light_pos_in_camera_space;
in vec3 vertex_pos_in_camera_space;
in vec3 vertex_normal_in_camera_space;
 in mat4 to_camera_from_local_space;
 
uniform vec4 base_colors[MAX_MATERIAL_TEXTURE_TYPES];

uniform int num_sampler_units;
// layout(binding=0) Binds first sampler in this array to UNIT 0, second sampler to UNIT 1 and so on...
layout(binding=0) uniform sampler2D texture2d_samplers[MAX_SAMPLER_UNITS];

uniform int num_texture_ops[MAX_MATERIAL_TEXTURE_TYPES];
uniform TextureOp texture_ops[MAX_MATERIAL_TEXTURE_TYPES][MAX_TEXTURE_OPERATIONS];

uniform int is_lighting_disabled;
uniform vec3 light_colors[MAX_LIGHT_COLORS];
uniform float ambient_light_fraction;

uniform samplerCube skybox;

out vec4 FragColor;

// TODO(Karan): Currently only suppors one UV channel
vec2 get_uv(int uv_channel)
{
    return fs_texture_uv; 
}

float safe_divide(float a, float b)
{
    if(abs(b) > 0.001f)
    {
        return a/b;
    }
    return a;
}

vec4 compute_material_color(int material_type)
{

    vec4 output_material_color = base_colors[material_type];
    for(int i = 0; i < num_texture_ops[material_type]; i++)
    {
        int sampler_unit = texture_ops[material_type][i].sampler_unit;
        int uv_channel = texture_ops[material_type][i].uv_channel;
        float texture_blend = texture_ops[material_type][i].texture_blend;
        int operation = texture_ops[material_type][i].operation;
        
        vec4 texture_color = texture(texture2d_samplers[sampler_unit], get_uv(uv_channel));
        
        texture_color *= texture_blend;
        
        if(operation == aiTextureOp_Multiply)
        {
            output_material_color= output_material_color * texture_color;
        }
        else if(operation == aiTextureOp_Add)
        {
            output_material_color= output_material_color + texture_color;
        }
        else if(operation == aiTextureOp_Subtract)
        {
            output_material_color= output_material_color - texture_color;
        }
        else if(operation == aiTextureOp_Divide)
        {
            output_material_color.r = safe_divide(output_material_color.r, texture_color.r);
            output_material_color.g = safe_divide(output_material_color.g, texture_color.g);
            output_material_color.b = safe_divide(output_material_color.b, texture_color.b);
        }
        else if(operation == aiTextureOp_SmoothAdd)
        {
            output_material_color = (output_material_color + texture_color) - (output_material_color - texture_color);
        }
        else if(operation == aiTextureOp_SignedAdd)
        {
            output_material_color = output_material_color + (texture_color - 0.5f);
        }
        else
        {
            output_material_color = (texture_color.a * texture_color) + ((1.0f - texture_color.a) * output_material_color);
        }
    }
    
    return output_material_color;
}


void main()
{

    vec4 diffuse_color = compute_material_color(aiTextureType_DIFFUSE);
    vec4 specular_color = compute_material_color(aiTextureType_SPECULAR);
    vec4 normal_map_color = compute_material_color(aiTextureType_NORMALS);
    vec4 shininess_color = compute_material_color(aiTextureType_SHININESS);
    vec4 ambient_color = diffuse_color;//compute_material_color(aiTextureType_AMBIENT);
    float shininess = shininess_color.r * 255.0f;
    
    vec4 color = diffuse_color + ambient_color + specular_color;
    vec3 normal = vec3(0.0f, 0.0f, 0.0f);
    
    vec4 reflected_skybox = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    if(is_lighting_disabled == 0)
    {
        if(normal_map_color.r == 0.0f && normal_map_color.g == 0.0f && normal_map_color.b == 0.0f){
            normal = normalize(vertex_normal_in_camera_space);
            }
        else
        {
            normal = normalize(normal_map_color.xyz);
        normal = normalize((to_camera_from_local_space * vec4(normal.xyz, 0.0f)).xyz);
        }
        
        // Reflection
        vec3 camera_to_vertex = vertex_pos_in_camera_space;
        vec3 camera_to_vertex_reflected = reflect(normalize(camera_to_vertex), normal);
         reflected_skybox = 0.5 * texture(skybox, camera_to_vertex_reflected);
         
        // Ambient light
        vec4 reflected_ambient = ambient_color * vec4(light_colors[AMBIENT_LIGHT_COLOR], 1.0f);
        
        // Diffuse light
        vec4 reflected_diffuse =  vec4(.0f, .0f, .0f, 1.0f);
        vec3 vertex_to_light_dir = normalize(light_pos_in_camera_space - vertex_pos_in_camera_space);
        bool is_vertex_facing_light = dot(vertex_to_light_dir, normal) >= 0;
        if(is_vertex_facing_light)
        {
            float diffuse_light_reflected_fraction = max(0.0f, dot(vertex_to_light_dir, normal));
            reflected_diffuse = diffuse_light_reflected_fraction * diffuse_color * vec4(light_colors[DIFFUSE_LIGHT_COLOR], 1.0f);
        }
        
        // Specular light
        vec4 reflected_specular = vec4(.0f, .0f, .0f, 1.0f);
        if(is_vertex_facing_light)
        {
            vec3 camera_pos = vec3(0.0f, 0.0f, 0.0f);
            vec3 vertex_to_camera_dir = normalize(camera_pos - vertex_pos_in_camera_space);
            vec3 reflect_light_dir = reflect(-vertex_to_light_dir, normal);
            float specular_light_reflected_fraction = pow(max(0.0f, dot(reflect_light_dir, vertex_to_camera_dir)), shininess);
            reflected_specular = specular_light_reflected_fraction * specular_color * vec4(light_colors[SPECULAR_LIGHT_COLOR], 1.0f);
        }
        
        // Final Phong light
        color = reflected_skybox + reflected_ambient + reflected_diffuse + reflected_specular;
    }
    
    FragColor = color;
}
)FOO";
    
    program->program_id = gl_create_program(vertex_source, fragment_source, 0);
    
    GL(program->mesh_transform = glGetUniformLocation(program->program_id, "mesh_transform"));
    GL(program->to_world_space = glGetUniformLocation(program->program_id, "to_world_space"));
    GL(program->to_camera_space = glGetUniformLocation(program->program_id, "to_camera_space"));
    GL(program->perspective_projection = glGetUniformLocation(program->program_id, "perspective_projection"));
    GL(program->light_position = glGetUniformLocation(program->program_id, "light_position"));
    
    GL(program->base_colors = glGetUniformLocation(program->program_id, "base_colors"));
    GL(program->num_sampler_units = glGetUniformLocation(program->program_id, "num_sampler_units"));
    GL(program->num_texture_ops = glGetUniformLocation(program->program_id, "num_texture_ops"));
    
    char temp_str[64] = {};
    for(u32 i = 0; i < aiTextureType_UNKNOWN; i++)
    {
        for(u32 j = 0; j < MAX_TEXTURE_OPERATIONS; j++)
        {
            snprintf(temp_str, 64, "texture_ops[%d][%d].sampler_unit\0", i, j);
            GL(program->texture_ops[i][j].sampler_unit = glGetUniformLocation(program->program_id, temp_str));
            snprintf(temp_str, 64, "texture_ops[%d][%d].texture_blend\0", i, j);
            GL(program->texture_ops[i][j].texture_blend = glGetUniformLocation(program->program_id, temp_str));
            snprintf(temp_str, 64, "texture_ops[%d][%d].uv_channel\0", i, j);
            GL(program->texture_ops[i][j].uv_channel = glGetUniformLocation(program->program_id, temp_str));
            snprintf(temp_str, 64, "texture_ops[%d][%d].operation\0", i, j);
            GL(program->texture_ops[i][j].operation = glGetUniformLocation(program->program_id, temp_str));
        }
    }
    
    GL(program->is_lighting_disabled = glGetUniformLocation(program->program_id, "is_lighting_disabled"));
    GL(program->light_colors = glGetUniformLocation(program->program_id, "light_colors"));
    GL(program->skybox_sampler = glGetUniformLocation(program->program_id, "skybox"));
}

void use_lighting_program(LightingProgram *program, hmm_mat4 *mesh_transform, hmm_mat4 *to_world_space, hmm_mat4 *to_camera_space, hmm_mat4 *perspective_transform, Material* material,  bool is_lighting_disabled, hmm_v3 light_position, hmm_v3 light_colors[MAX_LIGHT_COLORS], GLuint skybox_cubemap_id)
{
    // TODO(Karan): This function can probably take in a VAO spec and the Vertex Attribute stream and bind them here i.e. this function can take in values required to setup its vertex input. 
    
    GL(glUseProgram(program->program_id));
    
    GL(glUniformMatrix4fv(program->mesh_transform, 1, GL_FALSE, (GLfloat*)mesh_transform->Elements));
    GL(glUniformMatrix4fv(program->to_world_space, 1, GL_FALSE, (GLfloat*)to_world_space->Elements));
    GL(glUniformMatrix4fv(program->to_camera_space, 1, GL_FALSE, (GLfloat*)to_camera_space->Elements));
    GL(glUniformMatrix4fv(program->perspective_projection, 1, GL_FALSE, (GLfloat*)(perspective_transform->Elements)));
    GL(glUniform3fv(program->light_position, 1, light_position.Elements));
    
    GL(glUniform4fv(program->base_colors, aiTextureType_UNKNOWN, material->base_colors[0].Elements));
    GL(glUniform1i(program->num_sampler_units, material->num_sampler_units));
    
    for(s32 i = 0; i < material->num_sampler_units; i++)
    {
        GL(glActiveTexture(GL_TEXTURE0 + i)); 
        GL(glBindTexture(GL_TEXTURE_2D, material->sampler_units_to_texture_id[i]));
    }
    
    s32 skybox_sampler_unit = material->num_sampler_units;
    GL(glActiveTexture(GL_TEXTURE0 + skybox_sampler_unit)); 
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    GL(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap_id));
    GL(glUniform1i(program->skybox_sampler, skybox_sampler_unit));
    
    GL(glUniform1iv(program->num_texture_ops, aiTextureType_UNKNOWN, material->num_texture_ops));
    for(u32 i = 0; i < aiTextureType_UNKNOWN; i++)
    {
        for(u32 j = 0; j < (u32)material->num_texture_ops[i]; j++)
        {
            glUniform1i(program->texture_ops[i][j].sampler_unit, material->texture_ops[i][j].sampler_unit);
            glUniform1f(program->texture_ops[i][j].texture_blend, material->texture_ops[i][j].texture_blend);
            glUniform1i(program->texture_ops[i][j].uv_channel, material->texture_ops[i][j].uv_channel);
            glUniform1i(program->texture_ops[i][j].operation, material->texture_ops[i][j].operation);
        }
    }
    
    GL(glUniform1i(program->is_lighting_disabled, is_lighting_disabled ? 1 : 0));
    if(!is_lighting_disabled)
    {
        GL(glUniform3fv(program->light_colors, MAX_LIGHT_COLORS, light_colors[0].Elements));
    }
}


GLuint create_texture_blit_program()
{
    char *vertex_source = R"FOO(
    #version 430 core
    
    layout (location = 0) in vec2 quad_xy;
    layout (location = 1) in vec2 vs_quad_uv;
    
    out vec2 fs_quad_uv;
    
    void main()
    {
    gl_Position =  vec4(quad_xy, 0.0f, 1.0f);
    fs_quad_uv = vs_quad_uv;
    }
    
    )FOO";
    
    char *fragment_source = R"FOO(
    #version 430 core
    
    in vec2 fs_quad_uv;
    uniform sampler2D texture2d_sampler;
    out vec4 FragColor;
    
    void main()
    {
    FragColor = texture(texture2d_sampler, fs_quad_uv);
    }
    )FOO";
    
    GLuint program_id = gl_create_program(vertex_source, fragment_source, 0);
    gl_set_uniform_1i(program_id, "texture2d_sampler", 0);
    return program_id;
}


void use_texture_blit_program(GLuint program_id, GLuint texture_id)
{
    GL(glUseProgram(program_id));
    GL(glActiveTexture(GL_TEXTURE0)); 
    GL(glBindTexture(GL_TEXTURE_2D, texture_id));
}


void create_skybox_program(SkyboxProgram* program)
{
    char *vertex_source = R"FOO(
    #version 430 core
    
    layout (location = 0) in vec3 pos;
    
    uniform mat4 to_camera_space;
    uniform mat4 perspective_projection;
    
    out vec3 skybox_sample_dir;
    
    void main()
    {
    skybox_sample_dir = pos;
    vec3 pos_in_camera_space = (to_camera_space * vec4(pos.xyz, 0.0f)).xyz;
    gl_Position = perspective_projection * vec4(pos_in_camera_space.xyz, 1.0f);
    }
    
    )FOO";
    
    char *fragment_source = R"FOO(
    #version 430 core
    
    in vec3 skybox_sample_dir;
    out vec4 FragColor;
    uniform samplerCube skybox;
    
    void main()
    {
    FragColor = texture(skybox, skybox_sample_dir);
    }
    )FOO";
    
    program->program_id = gl_create_program(vertex_source, fragment_source, 0);
    GL(program->to_camera_space = glGetUniformLocation(program->program_id, "to_camera_space"));
    GL(program->perspective_projection = glGetUniformLocation(program->program_id, "perspective_projection"));
    
    gl_set_uniform_1i(program->program_id, "skybox", 0);
}

void use_skybox_program(SkyboxProgram *program, hmm_mat4 *to_camera_space, hmm_mat4* perspective_projection, GLuint skybox_cubemap_id)
{
    GL(glUseProgram(program->program_id));
    GL(glUniformMatrix4fv(program->to_camera_space, 1, GL_FALSE, (GLfloat*)to_camera_space->Elements));
    GL(glUniformMatrix4fv(program->perspective_projection, 1, GL_FALSE, (GLfloat*)(perspective_projection->Elements)));
    
    GL(glActiveTexture(GL_TEXTURE0)); 
    GL(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap_id));
}


void create_debug_normal_visualization_program(DebugNormalVisualizationProgram *program)
{
    char *vertex_source = R"FOO(
    #version 430 core
    
layout (location = 0) in vec3 vs_local_space_pos;
layout (location = 1) in vec2 vs_texture_uv;
layout (location = 2) in vec3 vs_normal;

uniform mat4 mesh_transform;
uniform mat4 to_world_space;
uniform mat4 to_camera_space;
uniform mat4 perspective_projection;

out VS_OUT {
    vec3 vertex_normal_in_camera_space;
} vs_out;

void main()
{
mat4 to_camera_from_local_space = (to_camera_space * to_world_space  * mesh_transform);

  vs_out.vertex_normal_in_camera_space = (to_camera_from_local_space * vec4(vs_normal, 0.0f)).xyz;
 //vertex_normal_in_camera_space = mat3(transpose(inverse(to_camera_space * to_world_space))) * vs_normal;
 
gl_Position =  perspective_projection * to_camera_from_local_space * vec4(vs_local_space_pos.xyz, 1.0f);
}

)FOO";
    
    char *geometry_source = R"FOO(
    #version 430 core
    
    layout (triangles) in;
    layout (line_strip, max_vertices = 6) out;
    
in VS_OUT {
    vec3 vertex_normal_in_camera_space;
} gs_in[];

uniform float vector_length;

void generate_vector(vec4 origin, vec3 direction, float length)
    {
    gl_Position = origin;
    EmitVertex();
    gl_Position = origin + (vec4(direction, 0.0f) * length);
    EmitVertex();
    EndPrimitive();
}

    void main()
    {
    vec3 normal = normalize(gs_in[0].vertex_normal_in_camera_space);
    generate_vector(gl_in[0].gl_Position, normal, vector_length);
    normal = normalize(gs_in[1].vertex_normal_in_camera_space);
    generate_vector(gl_in[1].gl_Position, normal, vector_length);
    normal = normalize(gs_in[2].vertex_normal_in_camera_space);
    generate_vector(gl_in[2].gl_Position, normal, vector_length);
    }
    )FOO";
    
    
    char *fragment_source = R"FOO(
    #version 430 core
    
out vec4 FragColor;

    void main()
    {
    vec3 vector_color = vec3(1.0f, 0.0f, 0.0f);
    FragColor = vec4(vector_color, 1.0f);
    }
    )FOO";
    
    program->program_id = gl_create_program(vertex_source, fragment_source, geometry_source);
    
    GL(program->mesh_transform = glGetUniformLocation(program->program_id, "mesh_transform"));
    GL(program->to_world_space = glGetUniformLocation(program->program_id, "to_world_space"));
    GL(program->to_camera_space = glGetUniformLocation(program->program_id, "to_camera_space"));
    GL(program->perspective_projection = glGetUniformLocation(program->program_id, "perspective_projection"));
    
    GL(program->vector_length = glGetUniformLocation(program->program_id, "vector_length"));
    //GL(program->vector_color = glGetUniformLocation(program->program_id, "vector_color"));
}

void use_debug_normal_visualization_program(DebugNormalVisualizationProgram *program, hmm_mat4 *mesh_transform, hmm_mat4 *to_world_space, hmm_mat4 *to_camera_space, hmm_mat4 *perspective_transform, f32 vector_length)
{
    GL(glUseProgram(program->program_id));
    
    GL(glUniformMatrix4fv(program->to_camera_space, 1, GL_FALSE, (GLfloat*)to_camera_space->Elements));
    GL(glUniformMatrix4fv(program->perspective_projection, 1, GL_FALSE, (GLfloat*)(perspective_transform->Elements)));
    
    GL(glUniformMatrix4fv(program->mesh_transform, 1, GL_FALSE, (GLfloat*)mesh_transform->Elements));
    GL(glUniformMatrix4fv(program->to_world_space, 1, GL_FALSE, (GLfloat*)to_world_space->Elements));
    GL(glUniformMatrix4fv(program->to_camera_space, 1, GL_FALSE, (GLfloat*)to_camera_space->Elements));
    GL(glUniformMatrix4fv(program->perspective_projection, 1, GL_FALSE, (GLfloat*)(perspective_transform->Elements)));
    
    GL(glUniform1f(program->vector_length, vector_length));
}