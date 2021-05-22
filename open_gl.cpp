#include "open_gl.h"

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
        GL(glVertexAttribFormat(i, attribute_format->num_components, attribute_format->data_type, attribute_format->should_normalize, vao.stride));
        ASSERT(vao.source_buffer_binding_point == attribute_format->source_buffer_binding_point, "Source buffer binding point for attribute %d is not the same as other attributes. For an interleaved VAO all attribute formats must read from same vertex attribute data buffer", i);
        GL(glVertexAttribBinding(i, vao.source_buffer_binding_point));
        
        switch(attribute_format->data_type)
        {
            case GL_FLOAT:
            {
                vao.stride += (sizeof(f32) * attribute_format->num_components);
            }break;
            default:
            {
                ASSERT(false, "Attribute data type for attribute %d  is currently not handled for stride calculation", i); 
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
        // TODO(Karan): Index values can be 16 bits, in order to support that we need to take in the size of individual index as an argument. Currently we are allowing only  32-bits
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
        LOG_ERR("Vertex shader compilation error: %s\n", errors);
        
        GL(glGetShaderInfoLog(fs, sizeof(errors), &ignored, errors));
        LOG_ERR("Fragment shader compilation error: %s\n", errors);
        
        GL(glGetProgramInfoLog(program_id, sizeof(errors), &ignored, errors));
        LOG_ERR("Program linking error: %s\n", errors);
        
        ASSERT(false, "OpenGL Program creation failed");
    }
    
    GL(glDeleteShader(vs));
    GL(glDeleteShader(fs));
    
    return program_id;
}


GLuint gl_create_texture2d(const char *image_path)
{
    GLuint texture_id = 0;
    stbi_set_flip_vertically_on_load(true);
    s32 image_width, image_height, image_channels;
    u8 *image_data = stbi_load(image_path, &image_width, &image_height, &image_channels, 0);
    
    if(image_data)
    {
        GLint format;
        if(image_channels == 3)
        {
            format = GL_RGB;
        }
        else if(image_channels == 4)
        {
            format = GL_RGBA;
        }
        else
        {
            format = GL_RGBA;
            ASSERT(false, "Cannot create texture2d. Currently does not handle %d channels", image_channels);
        }
        
        GLenum channel_data_type = GL_UNSIGNED_BYTE;
        
        GL(glGenTextures(1, &texture_id));
        GL(glBindTexture(GL_TEXTURE_2D, texture_id));
        
        
        GL(glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, channel_data_type, image_data));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        
        stbi_image_free(image_data);
        
        GL(glBindTexture(GL_TEXTURE_2D, 0));
    }
    return texture_id;
}

void gl_set_uniform_1i(GLuint program_id, char *uniform_name, s32 value)
{
    GLuint currently_bound_program_id;
    GL(glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&currently_bound_program_id));
    GL(glUseProgram(program_id));
    GL(u32 uniform_location = glGetUniformLocation(program_id, uniform_name));
    GL(glUniform1i(uniform_location, value));
    GL(glUseProgram(currently_bound_program_id));
}