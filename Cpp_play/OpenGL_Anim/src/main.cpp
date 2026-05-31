#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>

#include "glad.h"
#include <GLFW/glfw3.h>
#include "math_utils.h"
#include "shader.h"
#include "mesh.h"
#include "camera.h"

// ---- Globals ----
static int g_width = 1200, g_height = 800;
static float g_time = 0.0f;
static int g_frame_count = 0;
static float g_fps_timer = 0.0f;
static float g_fps = 0.0f;

// ---- Callbacks ----
static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    g_width = w; g_height = h;
    glViewport(0, 0, w, h);
}

static void key_callback(GLFWwindow* win, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GLFW_TRUE);
}

// ---- Helpers ----
static std::string load_file(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Cannot open: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static vec3 hsv_to_rgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    vec3 rgb;
    if      (h < 60)  rgb = {c, x, 0};
    else if (h < 120) rgb = {x, c, 0};
    else if (h < 180) rgb = {0, c, x};
    else if (h < 240) rgb = {0, x, c};
    else if (h < 300) rgb = {x, 0, c};
    else              rgb = {c, 0, x};
    return {rgb.x + m, rgb.y + m, rgb.z + m};
}

// ---- Draw a mesh with transform ----
static void draw_mesh(Shader& shader, Mesh& mesh, const mat4& model,
                      const vec3& color, Camera& cam, float time) {
    shader.use();
    shader.set_mat4("model", model);
    shader.set_vec3("objectColor", color);
    shader.set_vec3("lightPos", {5.0f, 5.0f, 5.0f});
    shader.set_vec3("viewPos", cam.position());
    shader.set_vec3("lightColor", {1.0f, 1.0f, 1.0f});
    shader.set_float("time", time);
    mesh.draw();
}

// ---- Main ----
int main() {
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(g_width, g_height,
        "Geometric Galaxy - OpenGL Animation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1); // vsync

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return -1;
    }

    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

    // OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load shaders
    std::string vert_src = load_file("shaders/scene.vert");
    std::string frag_src = load_file("shaders/scene.frag");
    if (vert_src.empty() || frag_src.empty()) {
        std::cerr << "Failed to load shaders\n";
        return -1;
    }
    Shader shader(vert_src, frag_src);

    // Create meshes
    Mesh cube_mesh = Mesh::cube();
    Mesh sphere_mesh = Mesh::sphere(32, 16);
    Mesh torus_mesh = Mesh::torus(48, 16);
    Mesh small_sphere = Mesh::sphere(16, 8);

    // Camera
    Camera camera(8.0f, 0.15f);
    camera.set_pitch(0.35f);

    // Projection
    mat4 projection = mat4::perspective(45.0f,
        static_cast<float>(g_width) / g_height, 0.1f, 100.0f);

    // Time tracking
    float last_time = static_cast<float>(glfwGetTime());

    // ---- Render loop ----
    while (!glfwWindowShouldClose(window)) {
        float current_time = static_cast<float>(glfwGetTime());
        float dt = current_time - last_time;
        last_time = current_time;
        g_time += dt;

        // FPS counter
        g_frame_count++;
        g_fps_timer += dt;
        if (g_fps_timer >= 1.0f) {
            g_fps = g_frame_count / g_fps_timer;
            g_frame_count = 0;
            g_fps_timer = 0.0f;
            std::string title = "Geometric Galaxy | FPS: " + std::to_string(static_cast<int>(g_fps));
            glfwSetWindowTitle(window, title.c_str());
        }

        // Update camera
        camera.update(dt);

        // Update projection on resize
        if (g_width > 0 && g_height > 0) {
            projection = mat4::perspective(45.0f,
                static_cast<float>(g_width) / g_height, 0.1f, 100.0f);
        }

        // Clear
        float bg = 0.05f + 0.02f * std::sin(g_time * 0.5f);
        glClearColor(bg, bg, bg + 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.set_mat4("projection", projection);
        shader.set_mat4("view", camera.view_matrix());

        // === Central rotating cube ===
        {
            mat4 model = mat4::rotate_y(g_time * 0.5f) * mat4::rotate_x(g_time * 0.3f);
            model = mat4::scale(0.5f, 0.5f, 0.5f) * model;
            vec3 color = hsv_to_rgb(std::fmod(g_time * 30.0f, 360.0f), 0.8f, 1.0f);
            draw_mesh(shader, cube_mesh, model, color, camera, g_time);
        }

        // === Orbiting sphere 1 ===
        {
            float angle = g_time * 0.8f;
            float x = 3.0f * std::cos(angle);
            float z = 3.0f * std::sin(angle);
            mat4 model = mat4::translate(x, 0.5f * std::sin(g_time * 1.2f), z)
                       * mat4::scale(0.5f, 0.5f, 0.5f)
                       * mat4::rotate_y(g_time * 2.0f);
            vec3 color = hsv_to_rgb(std::fmod(g_time * 30.0f + 120.0f, 360.0f), 0.9f, 1.0f);
            draw_mesh(shader, sphere_mesh, model, color, camera, g_time);
        }

        // === Orbiting sphere 2 (opposite) ===
        {
            float angle = g_time * 0.8f + PI;
            float x = 3.0f * std::cos(angle);
            float z = 3.0f * std::sin(angle);
            mat4 model = mat4::translate(x, 0.5f * std::cos(g_time * 1.5f), z)
                       * mat4::scale(0.4f, 0.4f, 0.4f)
                       * mat4::rotate_x(g_time * 1.5f);
            vec3 color = hsv_to_rgb(std::fmod(g_time * 30.0f + 240.0f, 360.0f), 0.85f, 1.0f);
            draw_mesh(shader, sphere_mesh, model, color, camera, g_time);
        }

        // === Rotating torus (tilted) ===
        {
            mat4 model = mat4::rotate_x(1.2f)
                       * mat4::rotate_y(g_time * 0.4f)
                       * mat4::rotate_z(g_time * 0.2f);
            model = mat4::scale(1.0f, 1.0f, 1.0f) * model;
            vec3 color = hsv_to_rgb(std::fmod(g_time * 20.0f + 60.0f, 360.0f), 0.7f, 0.9f);
            draw_mesh(shader, torus_mesh, model, color, camera, g_time);
        }

        // === Small orbiting moons ===
        for (int i = 0; i < 3; i++) {
            float base_angle = g_time * 1.2f + i * (2.0f * PI / 3.0f);
            float radius = 2.0f + 0.5f * std::sin(g_time * 0.5f + i);
            float x = radius * std::cos(base_angle);
            float y = 1.5f * std::sin(g_time * 0.7f + i * 1.5f);
            float z = radius * std::sin(base_angle);
            mat4 model = mat4::translate(x, y, z)
                       * mat4::scale(0.15f, 0.15f, 0.15f)
                       * mat4::rotate_y(g_time * 3.0f + i);
            vec3 color = hsv_to_rgb(std::fmod(g_time * 40.0f + i * 90.0f, 360.0f), 1.0f, 1.0f);
            draw_mesh(shader, small_sphere, model, color, camera, g_time);
        }

        // === Wireframe outer ring ===
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            mat4 model = mat4::rotate_x(PI / 2.0f) * mat4::rotate_y(g_time * 0.1f);
            model = mat4::scale(1.2f, 1.2f, 1.2f) * model;
            vec3 ring_color = {0.3f, 0.3f, 0.4f};
            draw_mesh(shader, torus_mesh, model, ring_color, camera, g_time);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
