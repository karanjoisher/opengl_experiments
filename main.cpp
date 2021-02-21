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
#include "open_gl.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

f32 global_third = (1.0f/3.0f);
f32 global_cube_vertex_data[] = {
    //// Local Coordinates
    // Front face
    .5f, -.5f,  .5f, //A
    .5f,  .5f,  .5f, //B 
    -.5f,  .5f,  .5f, //C
    -.5f, -.5f,  .5f, //D
    
    // Rear face
    .5f, -.5f, -.5f, //E
    .5f,  .5f, -.5f, //F
    -.5f,  .5f, -.5f, //G
    -.5f, -.5f, -.5f, //H
    
    // Right face
    .5f,  .5f,  .5f, //B
    .5f, -.5f,  .5f, //A
    .5f, -.5f, -.5f, //E
    .5f,  .5f, -.5f, //F
    
    // Left face
    -.5f, -.5f,  .5f, //D
    -.5f,  .5f,  .5f, //C
    -.5f,  .5f, -.5f, //G
    -.5f, -.5f, -.5f, //H
    
    // Top face
    .5f,  .5f,  .5f, //B 
    .5f,  .5f, -.5f, //F
    -.5f,  .5f, -.5f, //G
    -.5f,  .5f,  .5f, //C
    
    // Bottom face
    .5f, -.5f,  .5f, //A
    .5f, -.5f, -.5f, //E
    -.5f, -.5f, -.5f, //H
    -.5f, -.5f,  .5f, //D
    
    //// UVs
    // Front face
    global_third, 0.5f,
    global_third, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.5f,
    
    //// Rear face
    global_third, .5f,
    global_third, 1.f,
    2.f*global_third, 1.f,
    2.f*global_third, .5f,
    
    //// Right face
    .0f, .5f,
    .0f, .0f,
    global_third, .0f,
    global_third, .5f,
    
    //// Left face
    1.f, .5f,
    1.f, 1.f,
    2.f*global_third, 1.f,
    2.f*global_third, .5f,
    
    //// Top face
    2.f*global_third, .0f,
    2.f*global_third, .5f,
    global_third, .5f,
    global_third, .0f,
    
    //// Bottom face
    1.f, .5f,
    1.f, .0f,
    2.f*global_third, .0f,
    2.f*global_third, .5f
};


u32 global_cube_index_data[]  = 
{
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} 


GLFWwindow* startup(u32 window_width, u32 window_height)
{
    //glfw startup
    GLFWwindow* window = 0;
    if (!glfwInit()) return window;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(window_width, window_height, "OpenGL", NULL, NULL);
    if (!window){glfwTerminate();return window;}
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    glfwMakeContextCurrent(window);
    
    //Glew startup
    GLenum err = glewInit();
    if (GLEW_OK != err){fprintf(stderr, "GLEW_ERROR: %s\n", glewGetErrorString(err));}
    
    GL(glViewport(0, 0, window_width, window_height));
    
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
    
    return window;
}


inline void frame_start(GLFWwindow *window)
{
    bool show_demo_window = false;
    glfwPollEvents();
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if(show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
    
}

inline void frame_end(GLFWwindow *window)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

int main()
{
    u32 window_width = 1280, window_height = 720;
    GLFWwindow* window = startup(window_width, window_height);
    if(!window) return -1;
    
    
    // Create a generic position, textureUV VAO which can be reused by other attribute streams that have the same format
    GLAttributeFormat attribute_formats[2] = {{3, GL_FLOAT, GL_FALSE, 0, 0}, {2, GL_FLOAT, GL_FALSE, 0, 3 * 4}};
    GLInterleavedAttributesVAO vao = gl_create_interleaved_attributes_vao(attribute_formats, ARRAY_LENGTH(attribute_formats));
    
    // Upload attribute stream data to GPU
    GLVertexAttributesData attributes_data = gl_create_vertex_attributes_data(global_cube_vertex_data, ARRAY_LENGTH(global_cube_vertex_data), global_cube_index_data, ARRAY_LENGTH(global_cube_index_data));
    
    gl_bind_vao(&vao, attributes_data);
    while (!glfwWindowShouldClose(window))
    {
        frame_start(window);
        
        GL(glClearColor(1.0f, 0.0f, 0.0f, 1.0f));
        GL(glClear(GL_COLOR_BUFFER_BIT));
        GL(glDrawElements(GL_TRIANGLES, ARRAY_LENGTH(global_cube_index_data), GL_UNSIGNED_INT, 0));
        
        frame_end(window);
    }
    
    glfwTerminate();
    return 0;
}