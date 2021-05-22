#version 430 core

#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 32
#define MAX_TEXTURE_OPERATIONS 32
#define MAX_MATERIAL_TEXTURE_TYPES 18
#define aiTextureType_DIFFUSE 1
#define aiTextureType_SPECULAR 2
#define aiTextureType_AMBIENT 3
#define aiTextureType_EMISSIVE 4
#define aiTextureType_NORMALS 6
#define aiTextureType_SHININESS 7

#define aiTextureOp_Multiply 0
#define aiTextureOp_Add 1
#define aiTextureOp_Subtract 2
#define aiTextureOp_Divide 3
#define aiTextureOp_SmoothAdd 4
#define aiTextureOp_SignedAdd 5

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

uniform vec4 base_colors[MAX_MATERIAL_TEXTURE_TYPES];

uniform int num_sampler_units;
uniform sampler2D texture2d_samplers[GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];

uniform int num_texture_ops[MAX_MATERIAL_TEXTURE_TYPES];
//uniform TextureOp texture_ops[MAX_MATERIAL_TEXTURE_TYPES][MAX_TEXTURE_OPERATIONS];

uniform int is_lighting_disabled;
uniform vec3 light_color;
uniform float ambient_light_fraction;

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
        int sampler_unit = 0;//texture_ops[material_type][i].sampler_unit;
        int uv_channel = 0;//texture_ops[material_type][i].uv_channel;
        float texture_blend = 0;//texture_ops[material_type][i].texture_blend;
        int operation = 0;//texture_ops[material_type][i].operation;
        
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
    
    vec4 ambient_color = compute_material_color(aiTextureType_AMBIENT);
    vec4 diffuse_color = compute_material_color(aiTextureType_DIFFUSE);
    vec4 specular_color = compute_material_color(aiTextureType_SPECULAR);
    vec4 normal_map_color = compute_material_color(aiTextureType_NORMALS);
    vec4 shininess_color = compute_material_color(aiTextureType_SHININESS);
    float shininess = shininess_color.r * 255.0f;
    
    vec4 color = diffuse_color + ambient_color + specular_color;
    if(is_lighting_disabled == 0)
    {
        
        vec3 normal;
        if(normal_map_color.r == 0.0f && normal_map_color.g == 0.0f && normal_map_color.b == 0.0f){
            normal = normalize(vertex_normal_in_camera_space);
        }
        else
        {
            normal = normalize(normal_map_color.xyz);
        }
        
        // Ambient light
        vec3 incident_ambient_light = ambient_light_fraction * light_color;
        vec4 reflected_ambient = ambient_color * vec4(incident_ambient_light, 1.0f);
        
        // Diffuse light
        vec4 reflected_diffuse =  vec4(.0f, .0f, .0f, 1.0f);
        vec3 vertex_to_light_dir = normalize(light_pos_in_camera_space - vertex_pos_in_camera_space);
        bool is_vertex_facing_light = dot(vertex_to_light_dir, normal) >= 0;
        if(is_vertex_facing_light)
        {
            float diffuse_light_reflected_fraction = max(0.0f, dot(vertex_to_light_dir, normal));
            reflected_diffuse = diffuse_light_reflected_fraction * diffuse_color * vec4(light_color, 1.0f);
        }
        
        // Specular light
        vec4 reflected_specular = vec4(.0f, .0f, .0f, 1.0f);
        if(is_vertex_facing_light)
        {
            vec3 camera_pos = vec3(0.0f, 0.0f, 0.0f);
            vec3 vertex_to_camera_dir = normalize(camera_pos - vertex_pos_in_camera_space);
            vec3 reflect_light_dir = reflect(-vertex_to_light_dir, normal);
            float specular_light_reflected_fraction = pow(max(0.0f, dot(reflect_light_dir, vertex_to_camera_dir)), shininess);
            reflected_specular = specular_light_reflected_fraction * specular_color * vec4(light_color, 1.0f);
        }
        
        // Final Phong light
        color = reflected_ambient + reflected_diffuse + reflected_specular;
    }
    
    FragColor = color;
}