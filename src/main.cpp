#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "filePath.h"
#include "shader.h"
#include "texture.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Callback prototypes
void errorCallback(int error, const char* description);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Helper functions
void getMaxWindowSize(GLFWmonitor* monitor, int imgWidth, int imgHeight, int& winWidth, int& winHeight);

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    } else if (argc > 2) {
        std::cerr << "Warning: Extra arguments ignored" << std::endl;
    }
    // Open image file
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(getFilePath(argv[1]).c_str(), &width, &height, &nrChannels, 0);
    if(!data) {
        std::cerr << "Failed to load image" << std::endl;
        return -1;
    }

    if(!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); //Required for MacOS
    #endif

    // Create Window
    int winWidth, winHeight;
    getMaxWindowSize(glfwGetPrimaryMonitor(), width, height, winWidth, winHeight); 
    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "DoG filter", NULL, NULL);
    if(!window) {
        std::cerr << "Failed to create window" << std::endl;
        return -1;
    }

    // Set callbacks
    glfwMakeContextCurrent(window);
    glfwSetErrorCallback(errorCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    // glfwSetCursorPosCallback(window, mouseCallback);
    
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");


    // set up fragment & vertex shader program
    GLuint shaderProgram = createShaderProgram("basic.vert", "basic.frag");

    // set up filter shader programs
    

    // store the image in a texture
    GLuint texture = createTexture(data, width, height, nrChannels);
    stbi_image_free(data);

    glUseProgram(shaderProgram);
    bindTextureToShader(texture, 0, shaderProgram, "computedTexture");

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // setting up dog parameters
    
    float k; // scalar multiplier on second gaussian
    float epsilon; // threshold value
    float phi; // parameter for tanh function
    float tau; // parameter controlling the weight of each gaussian
    float sigma_c; // stdev of gaussian blur applied to SST
    float sigma_e; // stdev of gradient aligned gaussian blur
    float sigma_m; // stdev of gaussian used for line integral convolution
    float sigma_a; // stdev of second LIC gaussian for anti-aliasing
    int kernel_size = 5;

    // main loop
    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();

        // Start imGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("DoG Filter");
        ImGui::SliderFloat("k", &k, 0.0f, 1.0f);
        ImGui::SliderFloat("epsilon", &epsilon, 0.0f, 255.0f);
        ImGui::SliderFloat("phi", &phi, 0.0f, 1.0f);
        ImGui::SliderFloat("tau", &tau, 0.0f, 50.0f);
        ImGui::SliderFloat("sigma_c", &sigma_c, 0.0f, 10.0f);
        ImGui::SliderFloat("sigma_e", &sigma_e, 0.0f, 10.0f);
        ImGui::SliderFloat("sigma_m", &sigma_m, 0.0f, 10.0f);
        ImGui::SliderFloat("sigma_a", &sigma_a, 0.0f, 10.0f);
        ImGui::End();

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &VAO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void errorCallback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

void resizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void getMaxWindowSize(GLFWmonitor* monitor, int imgWidth, int imgHeight, int& winWidth, int& winHeight) {
    int screenWidth, screenHeight;  
    int xpos, ypos; 

    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &screenWidth, &screenHeight);

    float imgAspectRatio = (float)imgWidth / (float)imgHeight;
    float screenAspectRatio = (float)screenWidth / (float)screenHeight;

    if (imgAspectRatio > screenAspectRatio) {
        winWidth = screenWidth;
        winHeight = (int)(winWidth / imgAspectRatio);
    } else {
        winHeight = screenHeight;
        winWidth = (int)(winHeight * imgAspectRatio);
    }
}