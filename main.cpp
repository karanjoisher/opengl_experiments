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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "types.h"
#include "main.h"
#include "memory.cpp"
#include "open_gl.cpp"
#include "math.cpp"

#include "render.cpp"
#include "gl_programs.cpp"

State global_state = {};

void gl_set_default_framebuffer_viewport(GLFWwindow* window, f32 aspect_ratio)
{
    s32 window_width;
    s32 window_height;
    glfwGetWindowSize(window, &window_width, &window_height);	
    
    u32 x_padding = 0;
	u32 y_padding = 0;
    u32 width = (u32)window_width;
    u32 height = (u32)window_height;
    
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
    
    GL(glViewport(x_padding, y_padding, width, height));
} 

GLFWwindow* startup(State* state, u32 window_width, f32 aspect_ratio, char* model_file_path)
{
    state->aspect_ratio = aspect_ratio;
    
    //glfw startup
    GLFWwindow* window = 0;
    u32 window_height = (u32)(((f32)window_width/aspect_ratio) + 0.5f);
    if (!glfwInit()) return window;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(window_width, window_height, "OpenGL", NULL, NULL);
    if (!window){glfwTerminate();return window;}
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(window);
    
    //Glew startup
    GLenum err = glewInit();
    if (GLEW_OK != err){fprintf(stderr, "GLEW_ERROR: %s\n", glewGetErrorString(err));}
    GL(glEnable(GL_DEPTH_TEST));
    //GL(glDepthFunc(GL_GREATER));
    //GL(glClearDepth(-1.0f));
    
    //// Initialize global state
    
    // Create Shaders
    create_lighting_program(&(state->lighting_program));
    state->texture_blit_program = create_texture_blit_program();
    
    // Cube properties
    state->model_scale = 0.05f;
    state->model_translation = {0.0f, -1.0f, -3.0f};
    state->show_demo_window = false;
    
    // Create a generic position, textureUV, Normal VAO which can be reused by other attribute streams that have the same format
    GLAttributeFormat attribute_formats_xyz_uv_nxnynz[3] = {{3, GL_FLOAT, GL_FALSE, 0}, {2, GL_FLOAT, GL_FALSE, 0}, {3, GL_FLOAT, GL_FALSE, 0}};
    GLAttributeFormat attribute_formats_xy_uv[2] = {{2, GL_FLOAT, GL_FALSE, 0}, {2, GL_FLOAT, GL_FALSE, 0}};
    state->xyz_uv_nxnynz = gl_create_interleaved_attributes_vao(attribute_formats_xyz_uv_nxnynz, ARRAY_LENGTH(attribute_formats_xyz_uv_nxnynz));
    state->xy_uv = gl_create_interleaved_attributes_vao(attribute_formats_xy_uv, ARRAY_LENGTH(attribute_formats_xy_uv));
    
    f32 quad_vertex_data[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    
    state->quad_xy_uv = gl_create_vertex_attributes_data(quad_vertex_data, sizeof(quad_vertex_data), 0, 0);
    
    // Camera properties
    state->camera.near_plane_distance = 0.1f;
    state->camera.far_plane_distance = 100.f;
    state->camera.fov_radians = HMM_ToRadians(90.0f);
    state->camera.aspect_ratio = aspect_ratio;
    state->camera.looking_direction = RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS;
    hmm_v3 zero_rot = {};
    set_rotation(&state->camera, &zero_rot);
    state->camera_speed_per_sec = 2.0f;
    state->camera_sensitivity = 0.05f;
    
    // Light properties
    // TODO(Karan): Maybe add light fractions/separate light color values for ambient, diffuse, specular
    state->light_colors[DIFFUSE_LIGHT_COLOR] = {.3f, .3f, .3f};
    state->light_colors[SPECULAR_LIGHT_COLOR] = {.6f, .6f, .6f};
    state->light_colors[AMBIENT_LIGHT_COLOR] = {.1f, .1f, .1f};
    state->light_position = {0.0f, 0.0f, -1.5f};
    
    // Debug framebuffer
    GLuint color_texture_id,  depth_buffer_object, framebuffer_width = 1920, framebuffer_height = 1080;
    GL(glGenTextures(1, &color_texture_id));
    
    GL(glBindTexture(GL_TEXTURE_2D, color_texture_id));
    GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    
    GL(glGenRenderbuffers(1, &depth_buffer_object));
    GL(glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_object));  
    GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height));
    GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));  
    
    GL(glGenFramebuffers(1, &state->debug_fbo));
    GL(glBindFramebuffer(GL_FRAMEBUFFER, state->debug_fbo)); 
    GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_id, 0));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_object)); 
    
    GLenum fbo_status;
    GL(fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT(fbo_status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete, expected status: %d, actual status: %d", GL_FRAMEBUFFER_COMPLETE, fbo_status);
    state->debug_fbo_color_buffer_texture_id = color_texture_id;
    state->debug_fbo_width = framebuffer_width;
    state->debug_fbo_height = framebuffer_height;
    GL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
    
    // Test Assimp
    state->test_model = temp_load_obj(model_file_path, &(state->memory), state->xyz_uv_nxnynz);
    state->test_cube_model = temp_load_obj("cube.obj", &(state->memory), state->xyz_uv_nxnynz);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    
    return window;
}


inline void frame_start(GLFWwindow *window, State* state)
{
    glfwPollEvents();
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if(state->enable_debug_fbo)
    {
        GL(glBindFramebuffer(GL_FRAMEBUFFER, state->debug_fbo)); 
        GL(glViewport(0, 0, state->debug_fbo_width, state->debug_fbo_height));
    }
    else
    {
        GL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
        gl_set_default_framebuffer_viewport(window, state->aspect_ratio);
    }
    
    GL(glClearColor(0.317f, 0.458f, 0.741f, 1.0f));
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if(state->show_demo_window) ImGui::ShowDemoWindow(&state->show_demo_window);
}

inline void frame_end(GLFWwindow *window, State* state)
{
    if(state->enable_debug_fbo)
    {
        GL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
        GL(glDisable(GL_DEPTH_TEST));
        gl_set_default_framebuffer_viewport(window, state->aspect_ratio);
        GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GL(glClear(GL_COLOR_BUFFER_BIT));
        gl_bind_vao(&state->xy_uv, &state->quad_xy_uv);
        use_texture_blit_program(state->texture_blit_program, state->debug_fbo_color_buffer_texture_id);
        GL(glDrawArrays(GL_TRIANGLES, 0, 6));  
        GL(glEnable(GL_DEPTH_TEST));
    }
    
    state->enable_debug_fbo = state->enable_debug_fbo_on_next_frame;
    
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

void imgui_model_tree_control(Node *node)
{
    if(ImGui::TreeNode(node->name))
    {
        ImGui::DragFloat3("translate", node->additional_translation.Elements, 1.0f, 0.0f, 0.0f);
        ImGui::DragFloat3("rotation", node->additional_rotation_degrees.Elements, 1.0f, 0.0f, 0.0f);
        //ImGui::DragFloat4("row0", &node->_transform.a1, 1.0f, 0.0f, 0.0f);
        //ImGui::DragFloat4("row1", &node->_transform.b1, 1.0f, 0.0f, 0.0f);
        //ImGui::DragFloat4("row2", &node->_transform.c1, 1.0f, 0.0f, 0.0f);
        //ImGui::DragFloat4("row3", &node->_transform.d1, 1.0f, 0.0f, 0.0f);
        
        for(s32 i = 0; i < node->num_children; i++)
        {
            imgui_model_tree_control(node->children + i);
        }
        ImGui::TreePop();
    }
}

int main(int argc, char* argv[])
{
    char *model_file_path = "lego.fbx";
    if(argc < 2)
    {
        LOG_ERR("Usage: main.exe <path to 3d model file>\nDefaulting to lego.fbx\n");
    }
    else
    {
        model_file_path = argv[1];
    }
    
    u32 window_width = 1280;
    global_state.memory = create_memory(32*1024*1024);
    GLFWwindow* window = startup(&global_state, window_width, 16.0f/9.0f, model_file_path);
    if(!window) return -1;
    
    f64 start_time, end_time;
    bool dragging = false;
    f32 d_t = 0.0f;
    start_time = glfwGetTime();
    hmm_v2 mouse_pos, previous_mouse_pos;
    glfw_get_cursor_pos(window, &previous_mouse_pos);
    while (!glfwWindowShouldClose(window))
    {
        frame_start(window, &global_state);
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        //// Imgui Inputs
        
        // Model properties
        ImGui::DragFloat3("model pos", global_state.model_translation.Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat3("model rot", global_state.model_rotation.Elements, 1.00f, 0.0f, 0.0f);
        ImGui::DragFloat("model scale", &global_state.model_scale, 0.01f, 0.0f, 0.0f); 
        ImGui::BulletText("model tree:");
        imgui_model_tree_control(global_state.test_model->root);
        
        
        // Camera properties
        ImGui::Dummy(ImVec2(0.0f, 20.0f)); 
        ImGui::BulletText("In order to move camera use WASD. R & F to move camera up and down.");
        ImGui::BulletText("While moving you can use the mouse to rotate the camera");
        ImGui::BulletText("While not moving you can hold down ALT+Left mouse button and then drag the mouse to rotate camera");
        ImGui::DragFloat("cam speed", &global_state.camera_speed_per_sec, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("cam sensitivity", &global_state.camera_sensitivity, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("near", &global_state.camera.near_plane_distance, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("far", &global_state.camera.far_plane_distance, 0.01f, 0.0f, 0.0f); 
        ImGui::DragFloat("fov(radians)", &global_state.camera.fov_radians, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat("aspect ratio", &global_state.camera.aspect_ratio, 0.01f, 0.0f, 0.0f);
        ImGui::RadioButton("Looking down +ve Z Axis", (s32*)&global_state.camera.looking_direction, (s32)RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_POSITIVE_Z_AXIS); ImGui::SameLine();
        ImGui::RadioButton("Looking down -ve Z Axis", (s32*)&global_state.camera.looking_direction, (s32)RIGHT_HANDED_COORDINATE_SYSTEM_VIEWING_ALONG_NEGATIVE_Z_AXIS);
        
        // Debug FBO settings
        ImGui::Dummy(ImVec2(0.0f, 20.0f)); 
        ImGui::BulletText("Debug FBO is a RGBA buffer with each component being f32.");
        ImGui::BulletText("This can be used to capture and debug floating point values in Render Doc.");
        ImGui::BulletText("(Default FBO clamps negative values to 0)");
        ImGui::RadioButton("Enable debug fbo", (s32*)&global_state.enable_debug_fbo_on_next_frame, 1); ImGui::SameLine();
        ImGui::RadioButton("Disable debug fbo", (s32*)&global_state.enable_debug_fbo_on_next_frame, 0);
        
        
        // Light properties
        ImGui::Dummy(ImVec2(0.0f, 20.0f)); 
        ImGui::DragFloat3("light pos", global_state.light_position.Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat3("ambient light color", global_state.light_colors[AMBIENT_LIGHT_COLOR].Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat3("diffuse light color", global_state.light_colors[DIFFUSE_LIGHT_COLOR].Elements, 0.01f, 0.0f, 0.0f);
        ImGui::DragFloat3("specular light color", global_state.light_colors[SPECULAR_LIGHT_COLOR].Elements, 0.01f, 0.0f, 0.0f);
        
        //// Actual processing
        
        // Camera movements
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
        
        
        hmm_mat4 to_world_space  = HMM_Translate(global_state.model_translation) * Z_ROTATE(global_state.model_rotation.Z) * Y_ROTATE(global_state.model_rotation.Y) * X_ROTATE(global_state.model_rotation.X) * HMM_Scale({global_state.model_scale, global_state.model_scale, global_state.model_scale});
        
        hmm_mat4 to_camera_space = {};
        create_to_camera_space_transform(&to_camera_space, &global_state.camera);
        
        hmm_mat4 perspective_transform = {};
        create_perspective_transform(&perspective_transform, global_state.camera.near_plane_distance, global_state.camera.far_plane_distance, global_state.camera.fov_radians, global_state.camera.aspect_ratio, global_state.camera.looking_direction);
        
        // DRAW
        temp_draw_model(&global_state.lighting_program, global_state.test_model, &to_world_space, &to_camera_space, &perspective_transform, false, global_state.light_position, global_state.light_colors);
        to_world_space = HMM_Translate(global_state.light_position) * HMM_Scale({0.05f, 0.05f, 0.05f});
        //hmm_v4 light_color = {global_state.light_color.X, global_state.light_color.Y, global_state.light_color.Z, 1.0f};
        temp_draw_model(&global_state.lighting_program,global_state.test_cube_model, &to_world_space, &to_camera_space, &perspective_transform, true, global_state.light_position, global_state.light_colors);
        
        frame_end(window, &global_state);
        
        end_time = glfwGetTime();
        d_t = (f32)(end_time - start_time);
        start_time = end_time;
        previous_mouse_pos = mouse_pos;
    }
    
    glfwTerminate();
    return 0;
}