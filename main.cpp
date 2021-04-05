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
#include "math.cpp"
#include "render.cpp"
#include "gl_programs.cpp"

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

struct State
{
    LightingProgram lighting_program;
    
    GLInterleavedAttributesVAO xyz_uv_nxnynz;
    GLuint container_texture_id;
    f32 aspect_ratio;
    
    GLVertexAttributesData cube_xyz_uv_nxnynz;
    hmm_v3 cube_translation;
    hmm_v3 cube_rotation;
    f32 cube_ambientness;
    f32 cube_diffuseness;
    f32 cube_specularness;
    f32 cube_specular_unscatterness;
    
    bool show_demo_window;
    
    Camera camera;
    f32 camera_speed_per_sec;
    f32 camera_sensitivity;
    
    hmm_v3 light_color;
    hmm_v3 light_position;
    f32 ambient_light_fraction;
};

State global_state = {};

f32 global_cube_vertex_data[] = {
    //// Local Space Coordinates, Texture UVs
    // Front face
    .5f, -.5f,  .5f,  (1.0f/3.0f), 0.5f, .0f, .0f, 1.0f, //A
    .5f,  .5f,  .5f,  (1.0f/3.0f), 1.0f, .0f, .0f, 1.0f, //B 
    -.5f,  .5f,  .5f, 0.0f       , 1.0f, .0f, .0f, 1.0f, //C
    -.5f, -.5f,  .5f, 0.0f       , 0.5f, .0f, .0f, 1.0f, //D
    
    // Rear face
    .5f, -.5f, -.5f,  (1.0f/3.0f)    , .5f, .0f, .0f, -1.0f, //E
    .5f,  .5f, -.5f,  (1.0f/3.0f)    , 1.f, .0f, .0f, -1.0f, //F
    -.5f,  .5f, -.5f, 2.f*(1.0f/3.0f), 1.f, .0f, .0f, -1.0f, //G
    -.5f, -.5f, -.5f, 2.f*(1.0f/3.0f), .5f, .0f, .0f, -1.0f, //H
    
    // Right face
    .5f,  .5f,  .5f, .0f        , .5f, 1.0f, .0f, .0f, //B
    .5f, -.5f,  .5f, .0f        , .0f, 1.0f, .0f, .0f, //A
    .5f, -.5f, -.5f, (1.0f/3.0f), .0f, 1.0f, .0f, .0f, //E
    .5f,  .5f, -.5f, (1.0f/3.0f), .5f, 1.0f, .0f, .0f, //F
    
    // Left face
    -.5f, -.5f,  .5f, 1.f            , .5f, -1.0f, .0f, .0f, //D
    -.5f,  .5f,  .5f, 1.f            , 1.f, -1.0f, .0f, .0f, //C
    -.5f,  .5f, -.5f, 2.f*(1.0f/3.0f), 1.f, -1.0f, .0f, .0f, //G
    -.5f, -.5f, -.5f, 2.f*(1.0f/3.0f), .5f, -1.0f, .0f, .0f, //H
    
    // Top face
    .5f,  .5f,  .5f, 2.f*(1.0f/3.0f), .0f, .0f, 1.0f, .0f, //B 
    .5f,  .5f, -.5f, 2.f*(1.0f/3.0f), .5f, .0f, 1.0f, .0f, //F
    -.5f,  .5f, -.5f,(1.0f/3.0f)    , .5f, .0f, 1.0f, .0f, //G
    -.5f,  .5f,  .5f,(1.0f/3.0f)    , .0f, .0f, 1.0f, .0f, //C
    
    // Bottom face
    .5f, -.5f,  .5f,  1.f            , .5f, .0f, -1.0f, .0f, //A
    .5f, -.5f, -.5f,  1.f            , .0f, .0f, -1.0f, .0f, //E
    -.5f, -.5f, -.5f, 2.f*(1.0f/3.0f), .0f, .0f, -1.0f, .0f, //H
    -.5f, -.5f,  .5f, 2.f*(1.0f/3.0f), .5f, .0f, -1.0f, .0f //D
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
    GL(glClearColor(0.317, 0.458, 0.741, 1.0f));
    framebuffer_size_callback(window, window_width, window_height);
    
    // Initialize global state
    global_state.cube_translation = {0.0f, 0.0f, -3.0f};
    global_state.cube_ambientness = 1.0f;
    global_state.cube_diffuseness = 1.0f;
    global_state.cube_specularness = 1.0f;
    global_state.cube_specular_unscatterness = 1.0f;
    global_state.show_demo_window = false;
    create_lighting_program(&(global_state.lighting_program));
    global_state.container_texture_id = gl_create_texture2d("container_cube.jpg", GL_RGB, GL_UNSIGNED_BYTE);
    
    // Create a generic position, textureUV, Normal VAO which can be reused by other attribute streams that have the same format
    GLAttributeFormat attribute_formats[3] = {{3, GL_FLOAT, GL_FALSE, 0}, {2, GL_FLOAT, GL_FALSE, 0}, {3, GL_FLOAT, GL_FALSE, 0}};
    global_state.xyz_uv_nxnynz = gl_create_interleaved_attributes_vao(attribute_formats, ARRAY_LENGTH(attribute_formats));
    // Upload attribute stream data to GPU
    global_state.cube_xyz_uv_nxnynz = gl_create_vertex_attributes_data(global_cube_vertex_data, sizeof(global_cube_vertex_data), global_cube_index_data, sizeof(global_cube_index_data));
    
    global_state.camera.near_plane_distance = 0.1f;
    global_state.camera.far_plane_distance = 100.f;
    global_state.camera.fov_radians = HMM_ToRadians(90.0f);
    global_state.camera.aspect_ratio = aspect_ratio;
    global_state.camera.looking_direction = RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS;
    hmm_v3 zero_rot = {};
    set_rotation(&global_state.camera, &zero_rot);
    global_state.camera_speed_per_sec = 2.0f;
    global_state.camera_sensitivity = 0.05f;
    
    global_state.light_color = {1.0f, 1.0f, 1.0f};
    global_state.light_position = {0.0f, 0.0f, -1.5f};
    global_state.ambient_light_fraction = 0.3f;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
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

inline void glfw_get_cursor_pos(GLFWwindow *window, hmm_v2 *mouse_pos)
{
    f64 x, y;
    glfwGetCursorPos(window, &x, &y);
    mouse_pos->X = (f32)x;
    mouse_pos->Y = (f32)y;
}

int main()
{
    u32 window_width = 1280;
    GLFWwindow* window = startup(window_width, 16.0f/9.0f);
    
    if(!window) return -1;
    
    f64 start_time, end_time;
    bool dragging = false;
    f32 d_t = 0.0f;
    start_time = glfwGetTime();
    hmm_v2 mouse_pos, previous_mouse_pos;
    glfw_get_cursor_pos(window, &previous_mouse_pos);
    while (!glfwWindowShouldClose(window))
    {
        frame_start(window);
        
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        ImGui::DragFloat3("cube pos", global_state.cube_translation.Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat3("cube rot", global_state.cube_rotation.Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("cam speed", &global_state.camera_speed_per_sec, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("cam sensitivity", &global_state.camera_sensitivity, 0.01f, 0.0f, 0.0f);
        
        ImGui::RadioButton("Looking down +ve Z Axis", (s32*)&global_state.camera.looking_direction, (s32)RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS); ImGui::SameLine();
        ImGui::RadioButton("Looking down -ve Z Axis", (s32*)&global_state.camera.looking_direction, (s32)RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS);
        
        ImGui::DragFloat("near", &global_state.camera.near_plane_distance, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("far", &global_state.camera.far_plane_distance, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("fov(radians)", &global_state.camera.fov_radians, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("aspect ratio", &global_state.camera.aspect_ratio, 0.01f, 0.0f, 0.0f);
        
        ImGui::BulletText("In order to move camera use WASD");
        ImGui::BulletText("While moving you can use the mouse to rotate the camera");
        ImGui::BulletText("While not moving you can hold down ALT+Left mouse button and then drag the mouse to rotate camera");
        
        ImGui::DragFloat3("light pos", global_state.light_position.Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat3("light color", global_state.light_color.Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("ambient light factor", &global_state.ambient_light_fraction, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("cube ambientness", &global_state.cube_ambientness, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("cube diffuseness", &global_state.cube_diffuseness, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("cube specularness", &global_state.cube_specularness, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("cube specular unscatterness", &global_state.cube_specular_unscatterness, 0.01f, 0.0f, 0.0f);
        
        // Camera movement
        hmm_vec3 camera_movement_dir = {};
        if(glfwGetKey(window, GLFW_KEY_W)) camera_movement_dir += ((f32)global_state.camera.looking_direction*global_state.camera.axis[Z_AXIS]);
        if(glfwGetKey(window, GLFW_KEY_S)) camera_movement_dir -= ((f32)global_state.camera.looking_direction*global_state.camera.axis[Z_AXIS]);
        if(glfwGetKey(window, GLFW_KEY_A)) camera_movement_dir += ((f32)global_state.camera.looking_direction*global_state.camera.axis[X_AXIS]);
        if(glfwGetKey(window, GLFW_KEY_D)) camera_movement_dir -= ((f32)global_state.camera.looking_direction*global_state.camera.axis[X_AXIS]);
        if(glfwGetKey(window, GLFW_KEY_R)) camera_movement_dir += global_state.camera.axis[Y_AXIS];
        if(glfwGetKey(window, GLFW_KEY_F)) camera_movement_dir -= global_state.camera.axis[Y_AXIS];
        camera_movement_dir = HMM_NormalizeVec3(camera_movement_dir);
        bool camera_moved = camera_movement_dir.X != 0.0f || camera_movement_dir.Y != 0.0f || camera_movement_dir.Z != 0.0f; 
        global_state.camera.pos += (camera_movement_dir * global_state.camera_speed_per_sec * d_t);
        
        // Camera pitch yaw control using mouse
        bool alt_is_down = glfwGetKey(window, GLFW_KEY_LEFT_ALT);
        s32 left_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if(dragging && (left_button == GLFW_RELEASE))
        {
            dragging = false;
        }
        else if(!dragging && (left_button == GLFW_PRESS))
        {
            dragging = true;
        }
        
        glfw_get_cursor_pos(window, &mouse_pos);
        
        if(camera_moved || (alt_is_down && dragging))
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
            hmm_vec2 diff = (previous_mouse_pos - mouse_pos) * global_state.camera_sensitivity;
            hmm_v3 rotation = {-global_state.camera.looking_direction * diff.Y, diff.X, 0.0f};
            rotate_camera(&global_state.camera, &rotation);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  
        }
        
        
        hmm_mat4 to_world_space  = HMM_Translate(global_state.cube_translation) * Z_ROTATE(global_state.cube_rotation.Z) * Y_ROTATE(global_state.cube_rotation.Y) * X_ROTATE(global_state.cube_rotation.X);
        
        hmm_mat4 to_camera_space = {};
        create_to_camera_space_transform(&to_camera_space, &global_state.camera);
        
        hmm_mat4 perspective_transform = {};
        create_perspective_transform(&perspective_transform, global_state.camera.near_plane_distance, global_state.camera.far_plane_distance, global_state.camera.fov_radians, global_state.camera.aspect_ratio, global_state.camera.looking_direction);
        
        // Setup attribute stream, setup the shader and draw!
        gl_bind_vao(&global_state.xyz_uv_nxnynz, &global_state.cube_xyz_uv_nxnynz);
        use_lighting_program(&global_state.lighting_program, global_state.container_texture_id, {}, &to_world_space, &to_camera_space, &perspective_transform, false, global_state.light_position, global_state.light_color, global_state.ambient_light_fraction, global_state.cube_ambientness, global_state.cube_diffuseness, global_state.cube_specularness, global_state.cube_specular_unscatterness);
        GL(glDrawElements(GL_TRIANGLES, global_state.cube_xyz_uv_nxnynz.num_indices, GL_UNSIGNED_INT, 0));
        
        
        to_world_space = HMM_Translate(global_state.light_position) * HMM_Scale({0.05f, 0.05f, 0.05f});
        hmm_v4 light_color = {global_state.light_color.X, global_state.light_color.Y, global_state.light_color.Z, 1.0f};
        use_lighting_program(&global_state.lighting_program, 0, light_color, &to_world_space, &to_camera_space, &perspective_transform, true, {}, {}, .0f, .0f, .0f, .0f, .0f);
        GL(glDrawElements(GL_TRIANGLES, global_state.cube_xyz_uv_nxnynz.num_indices, GL_UNSIGNED_INT, 0));
        
        frame_end(window);
        end_time = glfwGetTime();
        d_t = (f32)(end_time - start_time);
        start_time = end_time;
        previous_mouse_pos = mouse_pos;
    }
    
    glfwTerminate();
    return 0;
}