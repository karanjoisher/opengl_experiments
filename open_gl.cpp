#pragma once
#include "logging.h"
#include "stb_image.h"
#define GL(f) f; gl_log_errors(#f, __FILE__, __LINE__) 

struct GLAttributeFormat
{
    GLint num_components;
    GLenum data_type;
    GLboolean should_normalize;
    GLuint offset_in_source_buffer;
    GLuint source_buffer_binding_point;
};

struct GLInterleavedAttributesVAO
{
    GLuint handle;
    GLsizei stride;
    GLuint source_buffer_binding_point;
};

struct GLVertexAttributesData
{
    GLuint vbo;
    GLuint ebo;
    GLuint num_indices;
};

#if 0
int gl_get_compile_link_status_if_not_success_log_err(u32 shader_or_program, Shader_Type type)
{
    int  success;
    char info_log[512];
    char *step = 0;
    char *compilation = "Compilation step";
    char *linking  = "Linking step";
    if(type == Shader_Type_shader)
    {
        
        u32 shader = shader_or_program;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) glGetShaderInfoLog(shader, 512, NULL, info_log);
        step = compilation;
    }
    else
    {
        u32 program = shader_or_program;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success) glGetProgramInfoLog(program, 512, NULL, info_log);
        step = linking;
    }
    
    if(!success) LOG_ERR("%s failed: %s\n", step, info_log);
    return success;
}
#endif

void gl_log_errors(char *function_invocation_expression, char *file, s32 line)
{
    GLenum error;
    error = glGetError();
    while(error != GL_NO_ERROR)
    {
        char* error_string = (char*)glewGetErrorString(error);
        LOG_ERR("OPENGL_ERROR %d: %s @ %s:%d %s\n", error, error_string, file, line, function_invocation_expression);
        error = glGetError();
    }
}

GLInterleavedAttributesVAO gl_create_interleaved_attributes_vao(GLAttributeFormat *attribute_formats, u32 num_attributes)
{
    GLInterleavedAttributesVAO vao = {};
    
    GL(glGenVertexArrays(1, &(vao.handle)));
    GL(glBindVertexArray(vao.handle));
    
    ASSERT(attribute_formats, "Attribute formats array is null");
    vao.source_buffer_binding_point = attribute_formats->source_buffer_binding_point;
    
    
    for(u32 i = 0; i < num_attributes; i++)
    {
        GLAttributeFormat *attribute_format = attribute_formats + i; 
        GL(glEnableVertexAttribArray(i));
        GL(glVertexAttribFormat(i, attribute_format->num_components, attribute_format->data_type, attribute_format->should_normalize, attribute_format->offset_in_source_buffer));
        GL(glVertexAttribBinding(i, attribute_format->source_buffer_binding_point));
        
        switch(attribute_format->data_type)
        {
            case GL_FLOAT:
            {
                vao.stride += (sizeof(f32) * attribute_format->num_components);
            }break;
            default:
            {
                ASSERT(false, "Attribute data type is currently not handled for stride calculation"); 
            }break;
        }
    }
    
    GL(glBindVertexArray(0));
    
    return vao;
}

GLVertexAttributesData gl_create_vertex_attributes_data(void *vertex_data, u32 vertex_data_size, void *index_data, u32 index_data_size)
{
    GLVertexAttributesData result = {};
    GL(glGenBuffers(1, &(result.vbo)));
    GL(glBindBuffer(GL_ARRAY_BUFFER, result.vbo));
    GL(glBufferData(GL_ARRAY_BUFFER, vertex_data_size, vertex_data, GL_STATIC_DRAW));
    
    if(index_data)
    {
        GL(glGenBuffers(1, &(result.ebo)));
        GL(glBindBuffer(GL_ARRAY_BUFFER, result.ebo));
        GL(glBufferData(GL_ARRAY_BUFFER, index_data_size, index_data, GL_STATIC_DRAW));
        result.num_indices = index_data_size/sizeof(u32);
    }
    
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    
    return result;
}


void gl_bind_vao(GLInterleavedAttributesVAO *vao, GLVertexAttributesData *attributes_data)
{
    GL(glBindVertexArray(vao->handle));
    GL(glBindVertexBuffer(vao->source_buffer_binding_point, attributes_data->vbo, 0, vao->stride));
    if(attributes_data->ebo)
    {
        GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, attributes_data->ebo));
    }
}

GLuint gl_create_program(char *vertex_source, char *fragment_source)
{
    GLuint vs, fs, program_id;
    
    GL(vs = glCreateShader(GL_VERTEX_SHADER));
    GL(glShaderSource(vs, 1, &vertex_source, 0));
    GL(glCompileShader(vs));
    
    GL(fs = glCreateShader(GL_FRAGMENT_SHADER));
    GL(glShaderSource(fs, 1, &fragment_source, 0));
    GL(glCompileShader(fs));
    
    GL(program_id = glCreateProgram());
    GL(glAttachShader(program_id, vs));
    GL(glAttachShader(program_id, fs));
    GL(glLinkProgram(program_id));
    
    GL(glValidateProgram(program_id));
    GLint linked = false;
    GL(glGetProgramiv(program_id, GL_LINK_STATUS, &linked));
    
    if(!linked)
    {
        GLsizei ignored;
        char errors[4096];
        
        GL(glGetShaderInfoLog(vs, sizeof(errors), &ignored, errors));
        LOG_ERR("Vertex shader compilation error: %s", errors);
        
        GL(glGetShaderInfoLog(fs, sizeof(errors), &ignored, errors));
        LOG_ERR("Fragment shader compilation error: %s", errors);
        
        GL(glGetProgramInfoLog(program_id, sizeof(errors), &ignored, errors));
        LOG_ERR("Program linking error: %s", errors);
        
        ASSERT(false, "OpenGL Program creation failed");
    }
    
    GL(glDeleteShader(vs));
    GL(glDeleteShader(fs));
    
    return program_id;
}


GLuint gl_create_texture2d(char *image_path, GLint format, GLenum channel_data_type)
{
    GLuint texture_id;
    stbi_set_flip_vertically_on_load(true);
    s32 image_width, image_height, image_channels;
    u8 *image_data = stbi_load(image_path, &image_width, &image_height, &image_channels, 0);
    
    GL(glGenTextures(1, &texture_id));
    GL(glBindTexture(GL_TEXTURE_2D, texture_id));
    
    GL(glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, channel_data_type, image_data));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    stbi_image_free(image_data);
    
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    
    return texture_id;
}

void gl_set_uniform_1i(GLuint program_id, char *uniform_name, s32 value)
{
    GL(glUseProgram(program_id));
    GL(u32 uniform_location = glGetUniformLocation(program_id, uniform_name));
    GL(glUniform1i(uniform_location, value));
    // TODO(Karan): Instead of setting to 0 consider setting the current program to the program that was bound before this function was called
    GL(glUseProgram(0));
}