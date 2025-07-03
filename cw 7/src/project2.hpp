// =============================================
// project.hpp - Minimalny test cieni
// =============================================

#pragma once
#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>


// Skybox vertices
float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

// Global variables
namespace texture {
    GLuint europe;
    GLuint europeNormal;
    GLuint bigben;
}

GLuint skyboxCubemap;
GLuint program;
GLuint programSun;
GLuint programTex;

Core::Shader_Loader shaderLoader;
Core::RenderContext europeContext;
Core::RenderContext bigbenContext;
Core::RenderContext lightSphereContext;

// Camera
glm::vec3 cameraPos = glm::vec3(0.f, 1.f, 5.f);
glm::vec3 cameraFront = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

GLuint skyboxVAO, skyboxVBO;
float aspectRatio = 1.f;

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 500.0f;
float lastY = 500.0f;
bool firstMouse = true;

// Shadow Mapping Variables
GLuint shadowMapFBO;
GLuint shadowMap;
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
GLuint programShadowMap;
glm::mat4 lightProjection, lightView, lightSpaceMatrix;

// Light properties - NOWA ZMIENNA GLOBALNA
glm::vec3 actualLightPosition = glm::vec3(5.0f, 10.0f, 5.0f);  // STAŁA POZYCJA ŚWIATŁA
glm::vec3 sunDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 sunPosition = glm::vec3(5.0f, 10.0f, 5.0f);  // Ustaw na actualLightPosition

// Shadow parameters
float shadowDistance = 10.0f;
float shadowNearPlane = 0.1f;
float shadowFarPlane = 20.0f;

// Function declarations
void init(GLFWwindow* window);
void renderLoop(GLFWwindow* window);
void shutdown(GLFWwindow* window);
void renderScene(GLFWwindow* window);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

glm::mat4 createCameraMatrix();
glm::mat4 createPerspectiveMatrix();
void setupShadowMapping();
void calculateLightSpaceMatrix();
void renderShadowMap();
void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalID, bool receiveShadows = true);
void drawSkybox();
void loadModelToContext(std::string path, Core::RenderContext& context);

// =============================================
// IMPLEMENTATIONS
// =============================================

glm::mat4 createCameraMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 createPerspectiveMatrix() {
    glm::mat4 perspectiveMatrix;
    float n = 0.05;
    float f = 80.;
    perspectiveMatrix = glm::mat4({
        1,0.,0.,0.,
        0.,aspectRatio,0.,0.,
        0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
        0.,0.,-1.,0.,
        });
    perspectiveMatrix = glm::transpose(perspectiveMatrix);
    return perspectiveMatrix;
}

void setupShadowMapping() {
    std::cout << "Setting up shadow mapping..." << std::endl;

    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "Shadow mapping setup complete. Shadow map ID: " << shadowMap << std::endl;
}

// NOWA NAPRAWIONA FUNKCJA calculateLightSpaceMatrix
void calculateLightSpaceMatrix() {


    // UŻYWAJ STAŁEJ POZYCJI ŚWIATŁA actualLightPosition
    glm::vec3 lightPosition = actualLightPosition;
    glm::vec3 lookAtTarget = glm::vec3(0.0f, 0.0f, 0.0f);    // Środek sceny
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);        // Normalny up

    // Projekcja ortograficzna
    float orthoSize = 15.0f;  // Większy obszar
    lightProjection = glm::ortho(-orthoSize, orthoSize,
        -orthoSize, orthoSize,
        0.1f, 25.0f);  // near/far

    // View matrix
    lightView = glm::lookAt(lightPosition, lookAtTarget, upVector);
    lightSpaceMatrix = lightProjection * lightView;

    sunPosition = lightPosition;
}

void renderShadowMap() {
    calculateLightSpaceMatrix();

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(programShadowMap);

    GLint lightSpaceLocation = glGetUniformLocation(programShadowMap, "lightSpaceMatrix");
    GLint modelLocation = glGetUniformLocation(programShadowMap, "model");

    glUniformMatrix4fv(lightSpaceLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    // Render Big Ben to shadow map
    glm::mat4 bigbenModel =
        glm::translate(glm::vec3(-1.3f, -0.03f, -0.7f)) *
        glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::vec3(0.3f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(bigbenModel));
    Core::DrawContext(bigbenContext);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Restore viewport
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    glViewport(0, 0, width, height);
}

// NAPRAWIONA FUNKCJA drawLightSphere - używa actualLightPosition
void drawLightSphere() {
    // Użyj STAŁEJ pozycji światła
    glm::vec3 lightPosition = actualLightPosition;

    // Stwórz macierz dla kuli światła
    glm::mat4 lightSphereModel =
        glm::translate(lightPosition) *           // Pozycja światła
        glm::scale(glm::vec3(0.5f));             // Mała kula

    // Użyj prostego shadera z kolorem
    GLuint prog = program;  // Shader bez tekstur
    glUseProgram(prog);

    glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
    glm::mat4 transformation = viewProjectionMatrix * lightSphereModel;

    glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&lightSphereModel);
    glUniform3f(glGetUniformLocation(prog, "color"), 1.0f, 1.0f, 0.0f);  // ŻÓŁTA KULA
    glUniform3f(glGetUniformLocation(prog, "lightPos"), lightPosition.x, lightPosition.y, lightPosition.z);

    Core::DrawContext(lightSphereContext);
    glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalID, bool receiveShadows) {
    GLuint prog = programTex;
    glUseProgram(prog);

    glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
    glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

    // Standard uniforms
    glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
    glUniformMatrix4fv(glGetUniformLocation(prog, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform3f(glGetUniformLocation(prog, "lightPos"), sunPosition.x, sunPosition.y, sunPosition.z);
    glUniform1i(glGetUniformLocation(prog, "receiveShadows"), receiveShadows ? 1 : 0);

    // Textures
    Core::SetActiveTexture(textureID, "colorTexture", prog, 0);
    Core::SetActiveTexture(normalID, "normalMap", prog, 1);
    Core::SetActiveTexture(shadowMap, "shadowMap", prog, 2);

    Core::DrawContext(context);
    glUseProgram(0);
}

void drawSkybox() {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(programSun);

    glm::mat4 view = glm::mat4(glm::mat3(createCameraMatrix()));
    glm::mat4 projection = createPerspectiveMatrix();

    glUniformMatrix4fv(glGetUniformLocation(programSun, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(programSun, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

void renderScene(GLFWwindow* window) {
    // Shadow pass
    renderShadowMap();

    // Main render pass
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSkybox();

    // Europa (receives shadows)
    glm::mat4 europeModel = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(2.2f));
    drawObjectTexture(europeContext, europeModel, texture::europe, texture::europeNormal, true);

    // Big Ben (visual - original size)
    glm::mat4 bigbenModel =
        glm::translate(glm::vec3(-1.3f, -0.03f, -0.7f)) *
        glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::vec3(0.3f));  // Original size for visual
    drawObjectTexture(bigbenContext, bigbenModel, texture::bigben, texture::bigben, false);

    drawLightSphere();

    glUseProgram(0);
}

void loadModelToContext(std::string path, Core::RenderContext& context) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    context.initFromAssimpMesh(scene->mMeshes[0]);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float sensitivity = 0.1f;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    aspectRatio = width / float(height);
    glViewport(0, 0, width, height);
}

// NAPRAWIONA FUNKCJA processInput z debugiem
void processInput(GLFWwindow* window) {
    float cameraSpeed = 0.025f;

    // Normalne sterowanie kamerą
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Light position control - sterowanie pozycją światła
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        actualLightPosition.x -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        actualLightPosition.x += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        actualLightPosition.y += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        actualLightPosition.y -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        actualLightPosition.z += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        actualLightPosition.z -= 0.1f;

    // DEBUG - Klawisz P
    static bool pKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed) {
        pKeyPressed = true;

        std::cout << "\n=== DEBUG SHADOW MAPPING ===" << std::endl;
        std::cout << "ACTUAL Light Position: " << actualLightPosition.x << ", " << actualLightPosition.y << ", " << actualLightPosition.z << std::endl;
        std::cout << "sunPosition: " << sunPosition.x << ", " << sunPosition.y << ", " << sunPosition.z << std::endl;

        // Test Big Ben z AKTUALNĄ macierzą lightSpaceMatrix
        glm::vec3 bigbenPos = glm::vec3(-1.3f, -0.03f, -0.7f);
        glm::vec4 bigbenInLightSpace = lightSpaceMatrix * glm::vec4(bigbenPos, 1.0f);

        std::cout << "Big Ben przed dzieleniem: " << bigbenInLightSpace.x << ", " << bigbenInLightSpace.y << ", " << bigbenInLightSpace.z << ", " << bigbenInLightSpace.w << std::endl;

        if (bigbenInLightSpace.w != 0.0f) {
            bigbenInLightSpace /= bigbenInLightSpace.w;
            std::cout << "Big Ben po dzieleniu przez W: " << bigbenInLightSpace.x << ", " << bigbenInLightSpace.y << ", " << bigbenInLightSpace.z << std::endl;

            bigbenInLightSpace = bigbenInLightSpace * 0.5f + 0.5f;
            std::cout << "Big Ben in Light Space (final): " << bigbenInLightSpace.x << ", " << bigbenInLightSpace.y << ", " << bigbenInLightSpace.z << std::endl;

            if (bigbenInLightSpace.x < 0.0f || bigbenInLightSpace.x > 1.0f ||
                bigbenInLightSpace.y < 0.0f || bigbenInLightSpace.y > 1.0f) {
                std::cout << "*** PROBLEM: Big Ben POZA SHADOW MAP! ***" << std::endl;
            }
            else {
                std::cout << "Big Ben jest W SHADOW MAP - OK" << std::endl;
            }
        }
        else {
            std::cout << "*** ERROR: W component is 0! ***" << std::endl;
        }

        std::cout << "Shadow Distance: " << shadowDistance << std::endl;
        std::cout << "===============================\n" << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pKeyPressed = false;
    }
}

void init(GLFWwindow* window) {
    std::cout << "=== MINIMAL SHADOW TEST INITIALIZATION ===" << std::endl;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);


    // === DODAJ TE LINIE PRZED CreateProgram ===

    // Sprawdź czy pliki istnieją
    std::ifstream vertFile("shaders/shader_5_1_tex.vert");
    std::ifstream fragFile("shaders/shader_5_1_tex.frag");

    std::cout << "\n=== FILE CHECK ===" << std::endl;
    std::cout << "shader_5_1_tex.vert exists: " << (vertFile.good() ? "YES" : "NO") << std::endl;
    std::cout << "shader_5_1_tex.frag exists: " << (fragFile.good() ? "YES" : "NO") << std::endl;

    if (vertFile.good()) {
        std::string line;
        std::getline(vertFile, line);
        std::cout << "First line of vert shader: " << line << std::endl;
    }

    vertFile.close();
    fragFile.close();
    std::cout << "==================\n" << std::endl;
    // Load shaders
    program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");
    programTex = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
    programShadowMap = shaderLoader.CreateProgram("shaders/shadow_map.vert", "shaders/shadow_map.frag");

    // Check shader loading
    std::cout << "Program IDs - Main: " << program << ", Tex: " << programTex << ", Shadow: " << programShadowMap << std::endl;

    // Check uniforms
    // W funkcji init(), ZASTĄP sekcję "Check uniforms" tym:

    // Check uniforms - ROZSZERZONY DEBUG
    std::cout << "\n=== SHADER DEBUG ===" << std::endl;
    std::cout << "Checking program " << programTex << std::endl;

    // Sprawdź czy program jest poprawny
    GLint linkStatus;
    glGetProgramiv(programTex, GL_LINK_STATUS, &linkStatus);
    std::cout << "Program link status: " << (linkStatus ? "SUCCESS" : "FAILED") << std::endl;

    if (!linkStatus) {
        char infoLog[512];
        glGetProgramInfoLog(programTex, 512, NULL, infoLog);
        std::cout << "Program link error: " << infoLog << std::endl;
    }

    // Sprawdź aktywne uniformy
    GLint numUniforms;
    glGetProgramiv(programTex, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::cout << "Number of active uniforms: " << numUniforms << std::endl;

    char uniformName[256];
    for (int i = 0; i < numUniforms; i++) {
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(programTex, i, 256, &length, &size, &type, uniformName);
        GLint location = glGetUniformLocation(programTex, uniformName);
        std::cout << "Uniform " << i << ": " << uniformName << " (location: " << location << ")" << std::endl;
    }

    // Teraz sprawdź konkretne uniformy
    GLint lightSpaceLoc = glGetUniformLocation(programTex, "lightSpaceMatrix");
    GLint shadowMapLoc = glGetUniformLocation(programTex, "shadowMap");
    GLint receiveShadowsLoc = glGetUniformLocation(programTex, "receiveShadows");

    std::cout << "Shader uniforms - lightSpace: " << lightSpaceLoc << ", shadowMap: " << shadowMapLoc << ", receiveShadows: " << receiveShadowsLoc << std::endl;
    std::cout << "===================\n" << std::endl;
    // Load models
    loadModelToContext("./models/eurobj.obj", europeContext);
    loadModelToContext("./models/bigben.obj", bigbenContext);
    loadModelToContext("./models/bigben.obj", lightSphereContext);

    // Load textures
    texture::europe = Core::LoadTexture("textures/europe.png");
    texture::europeNormal = Core::LoadTexture("textures/europeNormal.png");
    texture::bigben = Core::LoadTexture("textures/bigben.jpeg");

    // Setup skybox
    const char* faces[6] = {
        "textures/skybox/_px.jpg", "textures/skybox/_nx.jpg",
        "textures/skybox/_py.jpg", "textures/skybox/_ny.jpg",
        "textures/skybox/_pz.jpg", "textures/skybox/_nz.jpg"
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    skyboxCubemap = Core::LoadCubemap(faces);
    programSun = shaderLoader.CreateProgram("shaders/skybox.vert", "shaders/skybox.frag");

    // Setup shadow mapping
    setupShadowMapping();

    // Input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Inicjalizuj lightSpaceMatrix PRZED końcem
    calculateLightSpaceMatrix();
    std::cout << "Light space matrix initialized" << std::endl;

    std::cout << "=== INITIALIZATION COMPLETE ===" << std::endl;
}

void shutdown(GLFWwindow* window) {
    shaderLoader.DeleteProgram(program);
    shaderLoader.DeleteProgram(programTex);
    shaderLoader.DeleteProgram(programSun);
    shaderLoader.DeleteProgram(programShadowMap);

    glDeleteFramebuffers(1, &shadowMapFBO);
    glDeleteTextures(1, &shadowMap);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void renderLoop(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Simple shadow controls
        ImGui::Begin("Shadow Test");
        ImGui::SliderFloat("Shadow Distance", &shadowDistance, 5.0f, 20.0f);
        ImGui::SliderFloat3("Light Position", &actualLightPosition.x, -20.0f, 20.0f);
        if (ImGui::Button("Reset Light")) {
            actualLightPosition = glm::vec3(5.0f, 10.0f, 5.0f);
        }
        ImGui::End();

        // Render
        renderScene(window);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}