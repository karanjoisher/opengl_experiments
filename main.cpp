#define GLEW_STATIC 

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>
#undef HANDMADE_MATH_IMPLEMENTATION

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "types.h"
#include "open_gl.cpp"
#include "render.cpp"
#include "gl_programs.cpp"

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

struct State
{
    StandardShaderProgram standard_shader_program;
    GLuint container_texture_id;
    f32 aspect_ratio;
    
    hmm_v3 cube_translation;
    hmm_v3 cube_rotation;
    
    bool show_demo_window;
    
    f32 fov_radians;
    f32 near_plane_distance;
    f32 far_plane_distance;
    PerspectiveTransformCoordinateSystemOption coordinate_system_type;
};

State global_state = {};

f32 global_cube_vertex_data[] = {
    //// Local Space Coordinates, Texture UVs
    // Front face
    .5f, -.5f,  .5f,  (1.0f/3.0f), 0.5f, //A
    .5f,  .5f,  .5f,  (1.0f/3.0f), 1.0f, //B 
    -.5f,  .5f,  .5f, 0.0f, 1.0f, //C
    -.5f, -.5f,  .5f, 0.0f, 0.5f, //D
    
    // Rear face
    .5f, -.5f, -.5f,  (1.0f/3.0f), .5f,//E
    .5f,  .5f, -.5f,  (1.0f/3.0f), 1.f,//F
    -.5f,  .5f, -.5f, 2.f*(1.0f/3.0f), 1.f, //G
    -.5f, -.5f, -.5f, 2.f*(1.0f/3.0f), .5f, //H
    
    // Right face
    .5f,  .5f,  .5f, .0f, .5f,//B
    .5f, -.5f,  .5f, .0f, .0f,//A
    .5f, -.5f, -.5f, (1.0f/3.0f), .0f,//E
    .5f,  .5f, -.5f, (1.0f/3.0f), .5f,//F
    
    // Left face
    -.5f, -.5f,  .5f, 1.f, .5f,//D
    -.5f,  .5f,  .5f, 1.f, 1.f,//C
    -.5f,  .5f, -.5f, 2.f*(1.0f/3.0f), 1.f,//G
    -.5f, -.5f, -.5f, 2.f*(1.0f/3.0f), .5f,//H
    
    // Top face
    .5f,  .5f,  .5f, 2.f*(1.0f/3.0f), .0f,//B 
    .5f,  .5f, -.5f, 2.f*(1.0f/3.0f), .5f,//F
    -.5f,  .5f, -.5f,(1.0f/3.0f), .5f, //G
    -.5f,  .5f,  .5f,(1.0f/3.0f), .0f, //C
    
    // Bottom face
    .5f, -.5f,  .5f,  1.f, .5f,//A
    .5f, -.5f, -.5f,  1.f, .0f,//E
    -.5f, -.5f, -.5f, 2.f*(1.0f/3.0f), .0f, //H
    -.5f, -.5f,  .5f, 2.f*(1.0f/3.0f), .5f //D
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

void framebuffer_size_callback(GLFWwindow* window, int window_width, int window_height)
{
    u32 x_padding = 0;
	u32 y_padding = 0;
    u32 width = (u32)window_width;
    u32 height = (u32)window_height;
    
    State* state = (State*)glfwGetWindowUserPointer(window);
    if(state)
    {
        f32 aspect_ratio = state->aspect_ratio;
        ASSERT(aspect_ratio > 0, "Aspect ratio must be positive");
        if(aspect_ratio > 1.0f)
        {
            width = (u32)(window_width + 0.5f);
            height = (u32)(((f32)window_width/aspect_ratio) + 0.5f);
            if((u32)window_height > height) y_padding = ((u32)window_height - height)/2;
        }
        else
        {
            height = (u32)(window_height + 0.5f);
            width = (u32)(((f32)window_height * aspect_ratio) + 0.5f);
            if((u32)window_width > width) x_padding = (window_width - width)/2;
        }
    }
    
    GL(glViewport(x_padding, y_padding, width, height));
} 

GLFWwindow* startup(u32 window_width, f32 aspect_ratio)
{
    global_state.aspect_ratio = aspect_ratio;
    
    //glfw startup
    GLFWwindow* window = 0;
    u32 window_height = (u32)(((f32)window_width/aspect_ratio) + 0.5f);
    if (!glfwInit()) return window;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(window_width, window_height, "OpenGL", NULL, NULL);
    if (!window){glfwTerminate();return window;}
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &global_state);	
    
    //Glew startup
    GLenum err = glewInit();
    if (GLEW_OK != err){fprintf(stderr, "GLEW_ERROR: %s\n", glewGetErrorString(err));}
    
    GL(glEnable(GL_DEPTH_TEST));
    //GL(glDepthFunc(GL_GREATER));
    //GL(glClearDepth(-1.0f));
    GL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    
    framebuffer_size_callback(window, window_width, window_height);
    
    create_standard_shader_program(&(global_state.standard_shader_program));
    global_state.show_demo_window = false;
    global_state.container_texture_id = gl_create_texture2d("container_cube.jpg", GL_RGB, GL_UNSIGNED_BYTE);
    global_state.near_plane_distance = 0.1f;
    global_state.far_plane_distance = 100.f;
    global_state.fov_radians = HMM_ToRadians(90.0f);
    global_state.coordinate_system_type = RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS;
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
    glfwPollEvents();
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if(global_state.show_demo_window) ImGui::ShowDemoWindow(&global_state.show_demo_window);
    
}

inline void frame_end(GLFWwindow *window)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

int main()
{
    u32 window_width = 1280;
    GLFWwindow* window = startup(window_width, 16.0f/9.0f);
    
    if(!window) return -1;
    
    // Create a generic position, textureUV VAO which can be reused by other attribute streams that have the same format
    GLAttributeFormat attribute_formats[2] = {{3, GL_FLOAT, GL_FALSE, 0, 0}, {2, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0}};
    GLInterleavedAttributesVAO vao = gl_create_interleaved_attributes_vao(attribute_formats, ARRAY_LENGTH(attribute_formats));
    
    // Upload attribute stream data to GPU
    GLVertexAttributesData attributes_data = gl_create_vertex_attributes_data(global_cube_vertex_data, sizeof(global_cube_vertex_data), global_cube_index_data, sizeof(global_cube_index_data));
    
    gl_bind_vao(&vao, &attributes_data);
    f64 start_time, end_time;
    f32 d_t = 0.0f;
    start_time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        frame_start(window);
        
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        ImGui::DragFloat3("translate", global_state.cube_translation.Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat3("rotate", global_state.cube_rotation.Elements, 0.01f, 0.0f, 0.0f);
        
        ImGui::RadioButton("Looking down +ve Z Axis", (s32*)&global_state.coordinate_system_type, (s32)RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS); ImGui::SameLine();
        ImGui::RadioButton("Looking down -ve Z Axis", (s32*)&global_state.coordinate_system_type, (s32)RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS);
        
        ImGui::DragFloat("near", &global_state.near_plane_distance, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("far", &global_state.far_plane_distance, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("fov(radians)", &global_state.fov_radians, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("aspect ratio", &global_state.aspect_ratio, 0.01f, 0.0f, 0.0f);
        
        hmm_mat4 identity = HMM_Mat4d(1.0f);
        use_standard_shader_program(&(global_state.standard_shader_program), global_state.container_texture_id, &global_state.cube_translation, &global_state.cube_rotation, &identity, global_state.near_plane_distance, global_state.far_plane_distance, global_state.fov_radians, global_state.aspect_ratio, global_state.coordinate_system_type);
        GL(glDrawElements(GL_TRIANGLES, attributes_data.num_indices, GL_UNSIGNED_INT, 0));
        
        frame_end(window);
        end_time = glfwGetTime();
        d_t = (f32)(end_time - start_time);
        start_time = end_time;
    }
    
    
    glfwTerminate();
    return 0;
}