#pragma once
#include "logging.h"
#include "stb_image.h"

#ifdef GL_CHECK_ERRORS
#define GL(f) f; gl_log_errors(#f, __FILE__, __LINE__) 
#else
#define GL(f) f;
#endif

struct GLAttributeFormat
{
    GLint num_components;
    GLenum data_type;
    GLboolean should_normalize;
    GLuint source_buffer_binding_point;
};


/*
DILEMMA(Karan): Interleaved attributes in single buffer VS Continious attributes where each attribute has their own buffer
Consider an interleaved attribute data stream : xyz_uv

- If some object has the same position values but don't have any texture applied to it i.e. the object is just a geometry and has no use of texture UVs (e.g. a Solid colored cube), in such a scenario we can still continue to use the xyz_uv VAO and the attribute data : we would just ignore the uv data stream in vertex shader. So I don't think there is any dilemma wrt this scenario
- However consider an object which has same position values but different texture UV values, in this scenario we would have to create a different attribute data buffer that contains same pos vals but different texture uv vals. So we are duplicating the cube vertex data!! In order to prevent this we could probably have isolated buffers for each attribute stream (for e.g. a different buffer for both pos and uv vals), and then we can reuse the pos data but create a new buffer for the different texture uv vals.

 TODO(Karan): We currently have come across scenario 1 (i.e. we can simply ignore unwanted attribute data streams) but we need to consider what to do for second scenario!
*/
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

void gl_log_errors(char *function_invocation_expression, char *file, s32 line);
GLInterleavedAttributesVAO gl_create_interleaved_attributes_vao(GLAttributeFormat *attribute_formats, u32 num_attributes);
GLVertexAttributesData gl_create_vertex_attributes_data(void *vertex_data, u32 vertex_data_size, void *index_data, u32 index_data_size);
void gl_bind_vao(GLInterleavedAttributesVAO *vao, GLVertexAttributesData *attributes_data);
GLuint gl_create_program(char *vertex_source, char *fragment_source);
GLuint gl_create_texture2d(char *image_path );
void gl_set_uniform_1i(GLuint program_id, char *uniform_name, s32 value);