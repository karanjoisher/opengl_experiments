#define GLEW_STATIC 

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "types.h"
#include "file_utility.cpp"
#include "malloc.h"
#include "gl_utility.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} 


int main()
{
    u64 file_size = get_file_size("../vertex_shader.glsl");
    u8 *vs_source = (u8*)malloc(sizeof(u8) * (file_size + 1));
    read_entire_file("../vertex_shader.glsl", vs_source, file_size);
    vs_source[file_size] = '\0';
    
    file_size = get_file_size("../fragment_shader.glsl");
    u8 *fs_source = (u8*)malloc(sizeof(u8) * (file_size + 1));
    read_entire_file("../fragment_shader.glsl", fs_source, file_size);
    fs_source[file_size] = '\0';
    
    stbi_set_flip_vertically_on_load(true);
    s32 image1_width, image1_height, image1_channels;
    char *image1_path = "../data/container_cube.jpg";
    u8 *image1_data = stbi_load(image1_path, &image1_width, &image1_height, &image1_channels, 0);
    
    //glfw startup
    GLFWwindow* window;
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(800, 800, "OpenGL", NULL, NULL);
    if (!window){glfwTerminate();return -1;}
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    glfwMakeContextCurrent(window);
    
    //Glew startup
    GLenum err = glewInit();
    if (GLEW_OK != err){fprintf(stderr, "GLEW_ERROR: %s\n", glewGetErrorString(err));}
    
    GL_CALL(glViewport(0, 0, 800, 800));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    
    //GL_CALL(glFrontFace(GL_CCW));
    //GL_CALL(glCullFace(GL_BACK));
    //GL_CALL( glEnable(GL_CULL_FACE));
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    f32 third = 1.0f/3.0f;
    f32 cube_vertex_data[] = 
    {
        // Local Coordinates
        //// Front face
        .5f, -.5f,  .5f, //A
        .5f,  .5f,  .5f, //B 
        -.5f,  .5f,  .5f, //C
        -.5f, -.5f,  .5f, //D
        
        //// Rear face
        .5f, -.5f, -.5f, //E
        .5f,  .5f, -.5f, //F
        -.5f,  .5f, -.5f, //G
        -.5f, -.5f, -.5f, //H
        
        //// Right face
        .5f,  .5f,  .5f, //B
        .5f, -.5f,  .5f, //A
        .5f, -.5f, -.5f, //E
        .5f,  .5f, -.5f, //F
        
        //// Left face
        -.5f, -.5f,  .5f, //D
        -.5f,  .5f,  .5f, //C
        -.5f,  .5f, -.5f, //G
        -.5f, -.5f, -.5f, //H
        
        //// Top face
        .5f,  .5f,  .5f, //B 
        .5f,  .5f, -.5f, //F
        -.5f,  .5f, -.5f, //G
        -.5f,  .5f,  .5f, //C
        
        //// Bottom face
        .5f, -.5f,  .5f, //A
        .5f, -.5f, -.5f, //E
        -.5f, -.5f, -.5f, //H
        -.5f, -.5f,  .5f, //D
        
        // UVs
        //// Front face
        third, 0.5f,
        third, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.5f,
        
        //// Rear face
        third, .5f,
        third, 1.f,
	    2.f*third, 1.f,
	    2.f*third, .5f,
        
        //// Right face
        .0f, .5f,
        .0f, .0f,
        third, .0f,
        third, .5f,
        
        //// Left face
        1.f, .5f,
        1.f, 1.f,
        2.f*third, 1.f,
        2.f*third, .5f,
        
        //// Top face
        2.f*third, .0f,
        2.f*third, .5f,
	    third, .5f,
	    third, .0f,
        
        //// Bottom face
        1.f, .5f,
        1.f, .0f,
        2.f*third, .0f,
        2.f*third, .5f
    };
    
    u32 cube_vertex_index_data[]  = 
    {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    
    u32 vao;
    GL_CALL(glGenVertexArrays(1, &vao));
    GL_CALL(glBindVertexArray(vao));
    
    u32 vbo;
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_data), cube_vertex_data, GL_STATIC_DRAW));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3  * sizeof(f32), (void*)0));
    GL_CALL(glEnableVertexAttribArray(0)); 
    GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)(6 * 4 * 3 *  sizeof(f32))));
    GL_CALL(glEnableVertexAttribArray(1));
    
    u32 ebo;
    GL_CALL(glGenBuffers(1, &ebo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_vertex_index_data), cube_vertex_index_data, GL_STATIC_DRAW));
    
    u32 cube_texture;
    GL_CALL(glGenTextures(1, &cube_texture));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, cube_texture));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image1_width, image1_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image1_data));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    stbi_image_free(image1_data);
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    
    u32 vs;
    GL_CALL(vs = glCreateShader(GL_VERTEX_SHADER));
    GL_CALL(glShaderSource(vs, 1, (const GLchar *const *)(&vs_source), 0));
    GL_CALL(glCompileShader(vs));
    gl_get_compile_link_status_if_not_success_log_err(vs, Shader_Type_shader);
    
    u32 fs;
    GL_CALL(fs = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CALL(glShaderSource(fs, 1, (const GLchar *const *)(&fs_source), 0));
    GL_CALL(glCompileShader(fs));
    gl_get_compile_link_status_if_not_success_log_err(fs, Shader_Type_shader);
    
    u32 shader_program;
    GL_CALL(shader_program = glCreateProgram());
    GL_CALL(glAttachShader(shader_program, vs));
    GL_CALL(glAttachShader(shader_program, fs));
    GL_CALL(glLinkProgram(shader_program));
    gl_get_compile_link_status_if_not_success_log_err(shader_program, Shader_Type_program);
    GL_CALL(glDeleteShader(vs));
    GL_CALL(glDeleteShader(fs));
    GL_CALL(glUseProgram(shader_program));
    GL_CALL(glBindVertexArray(vao));
    GL_CALL(glActiveTexture(GL_TEXTURE0)); 
    GL_CALL(glBindTexture(GL_TEXTURE_2D, cube_texture));
    
    bool show_demo_window = true;
    GL_CALL(u32 texture1_obj_uniform_loc = glGetUniformLocation(shader_program, "texture1_obj"));
    GL_CALL(u32 to_world_uniform_loc = glGetUniformLocation(shader_program, "to_world"));
    GL_CALL(u32 to_camera_space_uniform_loc = glGetUniformLocation(shader_program, "to_camera_space"));
    
    GL_CALL(u32 nonlinear_perspective_uniform_loc = glGetUniformLocation(shader_program, "nonlinear_perspective"));
    GL_CALL(u32 linear_perspective_uniform_loc = glGetUniformLocation(shader_program, "linear_perspective"));
    GL_CALL(u32 projection_type_uniform_loc = glGetUniformLocation(shader_program, "projection_type"));
    
    GL_CALL(glUniform1i(glGetUniformLocation(shader_program, "texture1_obj"), 0));
    
    f32 cube_rotation_axis_arr[3] = {0.0f, 1.0f, 0.0f};
    f32 cube_position_arr[3] = {0.0f, 0.0f, 0.0f};
    f32 cube_rotation = 0.0f;
    f32 cube_scale = 1.0f;
    glm::vec3 cube_rotation_axis;
    
    f32 camera_rotation_axis_arr[3] = {0.0f, 1.0f, 0.0f};
    f32 camera_position_arr[3] = {0.0f, 0.0f, 0.0f};
    f32 camera_rotation = 0.0f;
    f32 projection_frustum[] = {.1f, 5.f, 2.f, -2.f, -2.f, 2.f};
    s32 projection_type = 1;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if(show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
        
        ImGui::DragFloat3("cube position", cube_position_arr, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("cube scaling", &cube_scale, 0.01f, 0.0f, 0.0f);
        ImGui::InputFloat3("cube rotation axis", cube_rotation_axis_arr);
        ImGui::SliderAngle("cube rotation angle", &cube_rotation);
        
        ImGui::DragFloat3("camera position", camera_position_arr, 0.01f, 0.0f, 0.0f);
        ImGui::InputFloat3("camera rotation axis", camera_rotation_axis_arr);
        ImGui::SliderAngle("camera rotation angle", &camera_rotation);
        
        ImGui::DragFloat2("near far", projection_frustum, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat2("left right", projection_frustum + 2, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat2("bottom top", projection_frustum + 4, 0.01f, 0.0f, 0.0f);
        
        const char* projection_types[2] = { "Linear Z Persp.", "NonLinear Z Persp "};
        const char* projection_name = (projection_type >= 0 && projection_type < 2) ? projection_types[projection_type] : "Unknown";
        ImGui::SliderInt("slider enum", &projection_type, 0, 2 - 1, projection_name);
        
        cube_rotation_axis = glm::normalize(glm::vec3(cube_rotation_axis_arr[0], cube_rotation_axis_arr[1], cube_rotation_axis_arr[2]));
        
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(cube_scale, cube_scale, cube_scale));
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), cube_rotation, glm::normalize(cube_rotation_axis));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(cube_position_arr[0], cube_position_arr[1], cube_position_arr[2]));
        
        glm::mat4 to_world = translate * rotate * scale;
        
        glm::mat4 to_camera_space = glm::translate(glm::mat4(1.0f), glm::vec3(-camera_position_arr[0], -camera_position_arr[1], -camera_position_arr[2]));
        
        to_camera_space = glm::transpose(glm::rotate(glm::mat4(1.0f), camera_rotation, glm::normalize(glm::vec3(camera_rotation_axis_arr[0], camera_rotation_axis_arr[1],
                                                                                                                camera_rotation_axis_arr[2])))) * to_camera_space; 
        
        f32 n = projection_frustum[0];
        f32 f = projection_frustum[1];
        f32 l = projection_frustum[2];
        f32 r = projection_frustum[3];
        f32 b = projection_frustum[4];
        f32 t = projection_frustum[5];
        
        f32 A = (2.f*n)/(r-l);
        f32 B = -1.f * (r+l)/(r-l);
        f32 C = (2.f*n)/(t-b);
        f32 D = -1.f * (t+b)/(t-b);
        f32 E = -1.f * (f+n)/(f-n);
        f32 F = (2.f*n*f)/(f-n);
        
        f32 perspective_projection_nonlinear_z_arr[16] = {
            A  , .0f, .0f,   B,
            .0f,   C, .0f,   D,
            .0f, .0f,   E,   F,
            .0f, .0f, 1.f, .0f
        };
        
        f32 perspective_projection_linear_z_arr[16] = {
            A  , .0f, .0f,   B,
            .0f,   C, .0f,   D,
            .0f, .0f, 2.f/(f-n), E,
            .0f, .0f, 1.f, .0f
        };
        
        glm::mat4 perspective_projection_nonlinear = glm::transpose(glm::make_mat4(perspective_projection_nonlinear_z_arr));
        glm::mat4 perspective_projection_linear = glm::transpose(glm::make_mat4(perspective_projection_linear_z_arr));
        
        GL_CALL(glUniformMatrix4fv(to_world_uniform_loc, 1, GL_FALSE, glm::value_ptr(to_world)));
        GL_CALL(glUniformMatrix4fv(to_camera_space_uniform_loc, 1, GL_FALSE, glm::value_ptr(to_camera_space)));
        GL_CALL(glUniformMatrix4fv(nonlinear_perspective_uniform_loc, 1, GL_FALSE, glm::value_ptr(perspective_projection_nonlinear)));
        GL_CALL(glUniformMatrix4fv(linear_perspective_uniform_loc, 1, GL_FALSE, glm::value_ptr(perspective_projection_linear)));
        GL_CALL(glUniform1i(projection_type_uniform_loc, projection_type));
        
        if(projection_type == 0)
        {
            GL_CALL(glDepthFunc(GL_LESS));
            GL_CALL(glClearDepth(1.0));
        }
        else
        {
            GL_CALL(glDepthFunc(GL_GREATER));
            GL_CALL(glClearDepth(0.0));
        }
        
        GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        GL_CALL(glDrawElements(GL_TRIANGLES, sizeof(cube_vertex_index_data)/sizeof(u32), GL_UNSIGNED_INT, 0));
        //GL_CALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(u32))));
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}