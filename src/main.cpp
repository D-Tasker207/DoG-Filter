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
    int imWidth, imHeight, nrChannels;
    unsigned char* data = stbi_load(getFilePath(argv[1]).c_str(), &imWidth, &imHeight, &nrChannels, 0);
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
    getMaxWindowSize(glfwGetPrimaryMonitor(), imWidth, imHeight, winWidth, winHeight); 
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
    GLuint displayProgram = createShaderProgram("basic.vert", "basic.frag");

    // set up filter shader programs
    GLuint greyProgram = createShaderProgram("basic.vert", "greyscale.frag");
    GLuint etfProgram = createShaderProgram("basic.vert", "etf.frag");
    GLuint sstProgram = createShaderProgram("basic.vert", "etfblur.frag"); 
    GLuint gradientblurProgram = createShaderProgram("basic.vert", "gradalignblur.frag");
    GLuint dogProgram = createShaderProgram("basic.vert", "dog.frag");
    GLuint licProgram = createShaderProgram("basic.vert", "lic.frag");
    GLuint thresholdProgram = createShaderProgram("basic.vert", "threshold.frag");

    // store the image in a texture
    GLuint texture = createTexture(data, imWidth, imHeight, nrChannels);
    stbi_image_free(data);

    //creating framebuffers for each stage of the pipeline
    GLuint* framebuffers = new GLuint[9];
    glGenFramebuffers(8, framebuffers);

    //creating textures for each stage of the pipeline
    GLuint* textures = new GLuint[9];
    glGenTextures(8, textures);

    //loop over the textures and framebuffers to bind them together
    for(int i = 0; i < 9; i++) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer not complete" << std::endl;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(displayProgram);
    bindTextureToShader(texture, 0, displayProgram, "originalTexture");

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // setting up filter parameters
    float k = 5; // scalar multiplier on second gaussian
    float epsilon = 30; // threshold value
    float phi = 0.2; // parameter for tanh function
    float tau = 25; // parameter controlling the weight of each gaussian
    float sigma_c = 3; // stdev of gaussian blur applied to SST
    float sigma_e = 1.4; // stdev of gradient aligned gaussian blur
    float sigma_m = 3; // stdev of gaussian used for line integral convolution
    float sigma_a = 3; // stdev of second LIC gaussian for anti-aliasing
    int kernelSize = 5; // size of the kernel used for gaussian blurs

    // main loop
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start imGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("DoG Filter");
        ImGui::SliderFloat("k", &k, 1.0f, 10.0f);
        ImGui::SliderFloat("epsilon", &epsilon, 0.0f, 255.0f);
        ImGui::SliderFloat("phi", &phi, 0.0f, 1.0f);
        ImGui::SliderFloat("tau", &tau, 0.0f, 50.0f);
        ImGui::SliderFloat("sigma_c", &sigma_c, 0.01f, 6.0f);
        ImGui::SliderFloat("sigma_e", &sigma_e, 0.01f, 6.0f);
        ImGui::SliderFloat("sigma_m", &sigma_m, 0.01f, 6.0f);
        ImGui::SliderFloat("sigma_a", &sigma_a, 0.01f, 6.0f);
        ImGui::SliderInt("kernelSize", &kernelSize, 0, 10);
        ImGui::End();

        //running filters on the image textures
        for(int i = 0; i < 9; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
            glViewport(0, 0, imWidth, imHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            switch(i){
                case 0:
                    glUseProgram(greyProgram);
                    bindTextureToShader(texture, 0, greyProgram, "imageSampler");
                    break;
                case 1:
                    glUseProgram(etfProgram);
                    bindTextureToShader(textures[0], 0, etfProgram, "imageSampler");
                    break;
                case 2:
                    glUseProgram(sstProgram);
                    bindTextureToShader(textures[1], 0, sstProgram, "etfSampler");
                    glUniform1f(glGetUniformLocation(sstProgram, "sigma"), sigma_c);
                    glUniform1i(glGetUniformLocation(sstProgram, "kernelSize"), kernelSize);
                    break;
                case 3:
                    glUseProgram(gradientblurProgram);
                    bindTextureToShader(textures[0], 0, gradientblurProgram, "imageSampler");
                    bindTextureToShader(textures[2], 1, gradientblurProgram, "etfSampler");
                    glUniform1f(glGetUniformLocation(gradientblurProgram, "sigma"), sigma_e);
                    glUniform1i(glGetUniformLocation(gradientblurProgram, "kernelSize"), kernelSize);
                    break;
                case 4:
                    glUseProgram(gradientblurProgram);
                    bindTextureToShader(textures[0], 0, gradientblurProgram, "imageSampler");
                    bindTextureToShader(textures[2], 1, gradientblurProgram, "etfSampler");
                    glUniform1f(glGetUniformLocation(gradientblurProgram, "sigma"), sigma_e * k);
                    glUniform1i(glGetUniformLocation(gradientblurProgram, "kernelSize"), kernelSize);
                    break;
                case 5:
                    glUseProgram(dogProgram);
                    bindTextureToShader(textures[3], 0, dogProgram, "blurredImage1");
                    bindTextureToShader(textures[4], 1, dogProgram, "blurredImage2");
                    glUniform1f(glGetUniformLocation(dogProgram, "tau"), tau);
                    break;
                case 6:
                    glUseProgram(licProgram);
                    bindTextureToShader(textures[5], 0, licProgram, "imageSampler");
                    bindTextureToShader(textures[2], 1, licProgram, "etfSampler");
                    glUniform1f(glGetUniformLocation(licProgram, "sigma"), sigma_m);
                    glUniform1i(glGetUniformLocation(licProgram, "kernelSteps"), kernelSize / 2);
                    break;
                case 7:
                    glUseProgram(licProgram);
                    bindTextureToShader(textures[6], 0, licProgram, "imageSampler");
                    bindTextureToShader(textures[2], 1, licProgram, "etfSampler");
                    glUniform1f(glGetUniformLocation(licProgram, "sigma"), sigma_a);
                    glUniform1i(glGetUniformLocation(licProgram, "kernelSteps"), kernelSize / 2);
                    break;
                case 8:
                    glUseProgram(thresholdProgram);
                    bindTextureToShader(textures[7], 0, thresholdProgram, "imageSampler");
                    glUniform1f(glGetUniformLocation(thresholdProgram, "epsilon"), epsilon);
                    glUniform1f(glGetUniformLocation(thresholdProgram, "phi"), phi);
                    break;
            }
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        // Display
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, winWidth, winHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(displayProgram);
        bindTextureToShader(textures[8], 1, displayProgram, "computedTexture");
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteProgram(displayProgram);
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

void getMaxWindowSize(GLFWmonitor* monitor, int imWidth, int imHeight, int& winWidth, int& winHeight) {
    int screenWidth, screenHeight;  
    int xpos, ypos; 

    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &screenWidth, &screenHeight);

    float imgAspectRatio = (float)imWidth / (float)imHeight;
    float screenAspectRatio = (float)screenWidth / (float)screenHeight;

    if (imgAspectRatio > screenAspectRatio) {
        winWidth = screenWidth;
        winHeight = (int)(winWidth / imgAspectRatio);
    } else {
        winHeight = screenHeight;
        winWidth = (int)(winHeight * imgAspectRatio);
    }
}