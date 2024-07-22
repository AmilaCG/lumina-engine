#include <array>
#include <iostream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "TextureUtils.h"
#include "Model.h"
#include "Shader.h"
#include "LightPreview.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void sceneSetup();
void renderLoop(GLFWwindow* window);
void setLightParameters();
glm::vec3 getCameraDirection(double yaw, double pitch);
void displayUI(const unsigned int& triangleCount);
void deinit();
void renderCube();
void renderQuad();

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;
constexpr float MOUSE_SENSITIVITY = 0.1f;
constexpr float DURATION_TO_MOUSE_HOLD = 0.1f; // In seconds
constexpr int CUBE_FACE_COUNT = 6;

const std::string MODEL_PATH = "Assets/Models/GuitarBackpack/guitar_backpack.obj";
// const std::string MODEL_PATH = "Assets/Models/OldTorch/old_torch.obj";
const char* OBJ_V_SHADER_PATH = "Assets/Shaders/shader_object.vert";
const char* OBJ_F_SHADER_PATH = "Assets/Shaders/shader_object.frag";

const char* LIGHT_V_SHADER_PATH = "Assets/Shaders/shader_light.vert";
const char* LIGHT_F_SHADER_PATH = "Assets/Shaders/shader_light.frag";

const char* SCR_V_SHADER_PATH = "Assets/Shaders/shader_screen.vert";
const char* SCR_F_SHADER_PATH = "Assets/Shaders/shader_screen.frag";

const char* SKYBOX_V_SHADER_PATH = "Assets/Shaders/shader_skybox.vert";
const char* SKYBOX_F_SHADER_PATH = "Assets/Shaders/shader_skybox.frag";

const char* EQR_TO_CUBE_V_SHADER_PATH = "Assets/Shaders/shader_cube_capture.vert";
const char* EQR_TO_CUBE_F_SHADER_PATH = "Assets/Shaders/shader_eqrect_to_cubemap.frag";

const char* IRRADIANCE_V_SHADER_PATH = "Assets/Shaders/shader_cube_capture.vert";
const char* IRRADIANCE_F_SHADER_PATH = "Assets/Shaders/shader_irradiance.frag";

const char* PREFILTER_V_SHADER_PATH = "Assets/Shaders/shader_cube_capture.vert";
const char* PREFILTER_F_SHADER_PATH = "Assets/Shaders/shader_prefilter.frag";

const char* BRDF_V_SHADER_PATH = "Assets/Shaders/shader_brdf.vert";
const char* BRDF_F_SHADER_PATH = "Assets/Shaders/shader_brdf.frag";

const std::string HDR_IMAGE_PATH = "Assets/Textures/Skybox/adams_place_bridge_4k.hdr";
constexpr int SKYBOX_RES = 2048;
constexpr int IRRADIANCE_MAP_RES = 128;
constexpr int PREFILTER_MAP_RES = 128;
constexpr int BRDF_MAP_RES = 512;

// Reserving unit 0 to 4 for PBR/phong material texture maps
constexpr unsigned int skyboxTexUnit = 5;
constexpr unsigned int hdriTexUnit = 6;
constexpr unsigned int screenTexUnit = 7;
constexpr unsigned int irradianceTexUnit = 8;
constexpr unsigned int prefilterTexUnit = 9;
constexpr unsigned int brdfLutTexUnit = 10;

static float pos[3];
static float rot[3];
static float scale[] = {1.0f, 1.0f, 1.0f};
static bool show_skybox = true;
static bool enableIBL = true;
static bool enable_point_lights = true;

const glm::vec3 world_front(0.0f, 0.0f, -1.0f);
const glm::vec3 world_up(0.0f, 1.0f, 0.0f);

const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
const glm::mat4 captureViews[] =
{
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

glm::vec3 cameraPosition(0.0f, 0.0f, 7.0f);
glm::vec3 cameraFront = world_front;
glm::vec3 cameraUp = world_up;
double camYaw = -90.0f; // Rotation around Y axis
double camPitch = 0.0f; // Rotation around X axis
float fov = 45.0f;

bool shouldPanCamera = false;
bool isFirstMouse = true;
double lastMouseX = SCR_WIDTH / 2.0f;
double lastMouseY = SCR_HEIGHT / 2.0f;

double deltaTime = 0.0f;
double lastFrameTime = 0.0f;

bool isLeftMouseHolding = false;
double mouseHoldStartTime = 0.0f;
double mouseHoldDuration = 0.0f;

Model* modelAsset = nullptr;
LightPreview* lightPreview = nullptr;
Shader* objectShader = nullptr;
Shader* lightShader = nullptr;
Shader* screenShader = nullptr;
Shader* skyboxShader = nullptr;
Shader* equirectToCubemapShader = nullptr;
Shader* irradianceShader = nullptr;
Shader* prefilterShader = nullptr;
Shader* brdfShader = nullptr;

unsigned int framebuffer = 0;
unsigned int textureColorbuffer = 0;
unsigned int rbo = 0;
unsigned int quadVAO = 0;
unsigned int quadVBO = 0;
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

unsigned int hdriTexture = 0;
unsigned int captureFBO = 0;
unsigned int captureRBO = 0;
unsigned int skyboxTex = 0;
unsigned int irradianceMapTex = 0;
unsigned int prefiltetMapTex = 0;
unsigned int brdfLutTex = 0;

glm::vec3 pointLightPositions[] = {
        glm::vec3(1.2f,  0.2f,  2.0f),
        glm::vec3(-1.2f,  0.5f,  2.0f),
        glm::vec3(-1.0f, 0.3f, -4.0f),
        glm::vec3(1.0f,  0.0f, -3.7f)
};

glm::vec3 pointLightColors[] = {
        glm::vec3(8.0f, 8.0f, 8.0f),
        glm::vec3(5.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 5.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 5.0f),
};

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    const float camera_speed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPosition += camera_speed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPosition -= camera_speed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * camera_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * camera_speed;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void sceneSetup(GLFWwindow* window)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    constexpr bool isPbr = true;
    modelAsset = new Model(MODEL_PATH, isPbr);
    objectShader = new Shader(OBJ_V_SHADER_PATH, OBJ_F_SHADER_PATH);
    lightPreview = new LightPreview();
    lightShader = new Shader(LIGHT_V_SHADER_PATH, LIGHT_F_SHADER_PATH);
    screenShader = new Shader(SCR_V_SHADER_PATH, SCR_F_SHADER_PATH);
    skyboxShader = new Shader(SKYBOX_V_SHADER_PATH, SKYBOX_F_SHADER_PATH);
    equirectToCubemapShader = new Shader(EQR_TO_CUBE_V_SHADER_PATH, EQR_TO_CUBE_F_SHADER_PATH);
    irradianceShader = new Shader(IRRADIANCE_V_SHADER_PATH, IRRADIANCE_F_SHADER_PATH);
    prefilterShader = new Shader(PREFILTER_V_SHADER_PATH, PREFILTER_F_SHADER_PATH);
    brdfShader = new Shader(BRDF_V_SHADER_PATH, BRDF_F_SHADER_PATH);

    // IMGUI setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Framebuffer config
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // Color attachment to the framebuffer
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // Renderbuffer object for depth and stencil attachment
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Framebuffer and renderbuffer setup for generating cubemaps by capturing the given environment
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SKYBOX_RES, SKYBOX_RES);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // Skybox texture setup
    glGenTextures(1, &skyboxTex);
    glActiveTexture(GL_TEXTURE0 + skyboxTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    for (unsigned int i = 0; i < CUBE_FACE_COUNT; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB16F, SKYBOX_RES, SKYBOX_RES, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Irradiance map texture setup
    glGenTextures(1, &irradianceMapTex);
    glActiveTexture(GL_TEXTURE0 + irradianceTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapTex);
    for (unsigned int i = 0; i < CUBE_FACE_COUNT; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB16F, IRRADIANCE_MAP_RES, IRRADIANCE_MAP_RES, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Prefilter map texture setup
    glGenTextures(1, &prefiltetMapTex);
    glActiveTexture(GL_TEXTURE0 + prefilterTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltetMapTex);
    for (unsigned int i = 0; i < CUBE_FACE_COUNT; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB16F, PREFILTER_MAP_RES, PREFILTER_MAP_RES, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Generate mipmaps for the cubemap so OpenGL automatically allocates the required memory
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // BRDF LUT map texture setup
    glGenTextures(1, &brdfLutTex);
    glActiveTexture(GL_TEXTURE0 + brdfLutTexUnit);
    glBindTexture(GL_TEXTURE_2D, brdfLutTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, BRDF_MAP_RES, BRDF_MAP_RES, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load cubemap
    glActiveTexture(GL_TEXTURE0 + hdriTexUnit);
    hdriTexture = TextureUtils::loadHdrImage(HDR_IMAGE_PATH);
    glBindTexture(GL_TEXTURE_2D, hdriTexture);

    // Convert HDR equirectangular environment map to cubemap equivalent
    equirectToCubemapShader->use();
    equirectToCubemapShader->setMat4("projection", captureProjection);
    equirectToCubemapShader->setInt("equirectangularMap", hdriTexUnit);

    glViewport(0, 0, SKYBOX_RES, SKYBOX_RES); // Configure the viewport to the capture dimensions
    for (int i = 0; i < CUBE_FACE_COUNT; i++)
    {
        equirectToCubemapShader->setMat4("view", captureViews[i]);
        // Already bound to captureFBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyboxTex, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }

    // Generate mipmaps of skybox
    glActiveTexture(GL_TEXTURE0 + skyboxTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    irradianceShader->use();
    irradianceShader->setMat4("projection", captureProjection);
    irradianceShader->setInt("environmentMap", skyboxTexUnit);
    glActiveTexture(GL_TEXTURE0 + skyboxTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    glViewport(0, 0, IRRADIANCE_MAP_RES, IRRADIANCE_MAP_RES);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, IRRADIANCE_MAP_RES, IRRADIANCE_MAP_RES);
    for (int i = 0; i < CUBE_FACE_COUNT; i++)
    {
        irradianceShader->setMat4("view", captureViews[i]);
        // Already bound to captureFBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMapTex, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }

    prefilterShader->use();
    prefilterShader->setMat4("projection", captureProjection);
    prefilterShader->setInt("environmentMap", skyboxTexUnit);
    prefilterShader->setInt("skyboxResolution", SKYBOX_RES);
    glActiveTexture(GL_TEXTURE0 + skyboxTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    constexpr unsigned int MAX_MIP_LEVELS = 8;
    for (int mip = 0; mip < MAX_MIP_LEVELS; mip++)
    {
        // Reisze framebuffer according to mip-level size
        const auto mipWidth = static_cast<unsigned int>(PREFILTER_MAP_RES * std::pow(0.5, mip));
        const auto mipHeight = static_cast<unsigned int>(PREFILTER_MAP_RES * std::pow(0.5, mip));
        // Already bound to captureRBO
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = static_cast<float>(mip) / static_cast<float>(MAX_MIP_LEVELS - 1);
        prefilterShader->setFloat("roughness", roughness);

        for (int i = 0; i < CUBE_FACE_COUNT; i++)
        {
            prefilterShader->setMat4("view", captureViews[i]);
            // Already bound to captureFBO
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefiltetMapTex, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }

    // Already bound to captureFBO and captureRBO
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, BRDF_MAP_RES, BRDF_MAP_RES);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLutTex, 0);
    glViewport(0, 0, BRDF_MAP_RES, BRDF_MAP_RES);
    brdfShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Before rendering, config the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    const glm::mat4 projection = glm::perspective(glm::radians(fov),
        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);

    // Setting constant uniforms
    objectShader->use();
    objectShader->setMat4("projection", projection);
    objectShader->setInt("skybox", skyboxTexUnit);
    objectShader->setInt("irradianceMap", irradianceTexUnit);
    objectShader->setInt("prefilterMap", prefilterTexUnit);
    objectShader->setInt("brdfLut", brdfLutTexUnit);

    lightShader->use();
    lightShader->setMat4("projection", projection);

    skyboxShader->use();
    skyboxShader->setMat4("projection", projection);
    skyboxShader->setInt("skybox", skyboxTexUnit);

    screenShader->use();
    screenShader->setInt("screenTexture", screenTexUnit);
}

void setLightParameters()
{
    glm::vec3 ambient(0.05);
    glm::vec3 diffuse(0.8f);
    glm::vec3 specular(1.0f);

    // Directional light
    objectShader->setBool("dirLight.isActive", false);
    objectShader->setVec3("dirLightDirection", glm::vec3(-0.2f, -1.0f, -0.3f));
    objectShader->setVec3("dirLight.ambient", ambient);
    objectShader->setVec3("dirLight.diffuse", diffuse);
    objectShader->setVec3("dirLight.specular", specular);

    // Point light
    int i = 0;
    for (const glm::vec3& position : pointLightPositions)
    {
        std::ostringstream ossLightPos;
        ossLightPos << "pointLightPos[" << i << "]";
        std::string pointLightPos = ossLightPos.str();
        objectShader->setVec3(pointLightPos, position);

        std::ostringstream ossLightParams;
        ossLightParams << "pointLights[" << i << "]";
        std::string pointLight = ossLightParams.str();
        objectShader->setBool(pointLight + ".isActive", enable_point_lights);
        objectShader->setVec3(pointLight + ".ambient", ambient * pointLightColors[i]);
        objectShader->setVec3(pointLight + ".diffuse", diffuse * pointLightColors[i]);
        objectShader->setVec3(pointLight + ".specular", specular);
        // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        objectShader->setFloat(pointLight + ".constant", 1.0f);
        objectShader->setFloat(pointLight + ".linear", 0.09f);
        objectShader->setFloat(pointLight + ".quadratic", 0.032f);
        i++;
    }

    // Spot light
    objectShader->setBool("spotLights[0].isActive", false);
    objectShader->setVec3("spotLightPos[0]", cameraPosition);
    objectShader->setVec3("spotLightDir[0]", cameraFront);
    objectShader->setFloat("spotLights[0].cutOff", glm::cos(glm::radians(12.5f)));
    objectShader->setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(18.5f)));
    objectShader->setVec3("spotLights[0].ambient", glm::vec3(0.2f));
    objectShader->setVec3("spotLights[0].diffuse", diffuse);
    objectShader->setVec3("spotLights[0].specular", specular);
    objectShader->setFloat("spotLights[0].constant", 1.0f);
    objectShader->setFloat("spotLights[0].linear", 0.09f);
    objectShader->setFloat("spotLights[0].quadratic", 0.032f);
}

void renderLoop(GLFWwindow* window)
{
    unsigned int indiceCount = 0;
    processInput(window);
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, world_up);

    // Bind to framebuffer and draw scene as we normally would to color texture
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST); // Enable depth testing (disabled for rendering screen-space quad)

    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    objectShader->use();
    setLightParameters();
    objectShader->setFloat("material.shininess", 192.0f);
    objectShader->setMat4("view", view);
    objectShader->setVec3("camPos", cameraPosition);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rot[0]), glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, glm::radians(rot[1]), glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, glm::radians(rot[2]), glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(pos[0], pos[1], pos[2]));
    model = glm::scale(model, glm::vec3(scale[0], scale[1], scale[2]));
    objectShader->setMat4("model", model);

    // Activate and bind skybox texture for reflections before drawing the model
    glActiveTexture(GL_TEXTURE0 + skyboxTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    objectShader->setBool("enableIBL", enableIBL);

    indiceCount += modelAsset->Draw(*objectShader);

    if (enable_point_lights)
    {
        lightShader->use();
        lightShader->setMat4("view", view);
        for (int i = 0; i < std::size(pointLightPositions); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lightShader->setMat4("model", model);

            lightPreview->Draw(*lightShader, pointLightColors[i]);
        }
    }

    if (show_skybox)
    {
        // Draw the skybox
        // Depth test passes when values are equal to depth buffer's content. Even though ideally this
        // should be GL_EQUAL, some artifacts will occur on the skybox when panning because sometimes
        // incoming pixel depth value < depth value in depth buffer, so we use GL_LEQUAL to avoid them
        glDepthFunc(GL_LEQUAL);
        // Remove translation section of the view transform matrix by taking only the upper-left 3x3 matrix,
        // so the skybox will rotate but not scale or move.
        const auto viewSkybox = glm::mat4(glm::mat3(view));
        skyboxShader->use();
        skyboxShader->setMat4("view", viewSkybox);
        renderCube();
        glDepthFunc(GL_LESS); // Set depth function back to default
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Back to default framebuffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader->use();
    screenShader->setFloat("gamma", 2.0f);
    screenShader->setFloat("exposure", 1.0f);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0 + screenTexUnit);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    renderQuad();

    // Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    const unsigned int triCount = indiceCount / 3;
    displayUI(triCount);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void displayUI(const unsigned int& triangleCount)
{
    const ImGuiIO& io = ImGui::GetIO();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float offset = 10;

    // Start Settings window
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + offset, viewport->WorkPos.y + offset),
        ImGuiCond_FirstUseEver);
    ImGui::Begin("Settings");

    // Start Transform section
    ImGui::SeparatorText("Transform");
    ImGui::DragFloat3("Position", pos, 0.001f);
    ImGui::DragFloat3("Rotation", rot, 0.01f);
    ImGui::DragFloat3("Scale", scale, 0.001f);
    // End Transform section

    ImGui::Spacing();

    ImGui::SeparatorText("Skybox");
    ImGui::Checkbox("Show skybox", &show_skybox);

    ImGui::Spacing();

    ImGui::SeparatorText("Lights");
    ImGui::Checkbox("Enable Point Lights", &enable_point_lights);

    ImGui::Spacing();

    ImGui::SeparatorText("IBL (Image Based Lighting)");
    ImGui::Checkbox("Enable IBL", &enableIBL);

    ImGui::End();
    // End Settings window

    // Start stats window
    static ImGuiWindowFlags stats_window_flags = 0;
    stats_window_flags |= ImGuiWindowFlags_NoMove;
    stats_window_flags |= ImGuiWindowFlags_NoCollapse;
    stats_window_flags |= ImGuiWindowFlags_NoTitleBar;
    stats_window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin("Stats", nullptr, stats_window_flags);
    ImGui::SetWindowPos(ImVec2(viewport->WorkSize.x - ImGui::GetWindowWidth() - offset,
                               viewport->WorkPos.y + offset), ImGuiCond_Always);
    ImGui::SeparatorText("Stats");
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Avg: %.3f ms", 1000.0f / io.Framerate);
    ImGui::Text("Triangles: %d", triangleCount);
    ImGui::End();
    // End stats window
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        shouldPanCamera = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        shouldPanCamera = false;
        isFirstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        isLeftMouseHolding = true;
        mouseHoldStartTime = glfwGetTime();
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        isLeftMouseHolding = false;
        mouseHoldDuration = 0.0f;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (!shouldPanCamera)
    {
        return;
    }

    if (isFirstMouse)
    {
        lastMouseX = xPos;
        lastMouseY = yPos;
        isFirstMouse = false;
    }

    double xOffset = xPos - lastMouseX;
    double yOffset = lastMouseY - yPos;
    lastMouseX = xPos;
    lastMouseY = yPos;

    xOffset *= MOUSE_SENSITIVITY;
    yOffset *= MOUSE_SENSITIVITY;

    camYaw += xOffset;
    camPitch += yOffset;

    if (camPitch > 89.0f) { camPitch = 89.0f; }
    if (camPitch < -89.0f) { camPitch = -89.0f; }

    glm::vec3 cameraDirection = getCameraDirection(camYaw, camPitch);
    cameraFront = glm::normalize(cameraDirection);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    fov -= (float)yOffset;
    if (fov < 1.0f) { fov = 1.0f; }
    if (fov > 45.0f) { fov = 45.0f; }
}

glm::vec3 getCameraDirection(const double yaw, const double pitch)
{
    glm::vec3 cameraDirection;
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    return cameraDirection;
}

void deinit()
{
    glDeleteTextures(1, &hdriTexture);
    delete(modelAsset);
    delete(objectShader);
    delete(lightPreview);
    delete(lightShader);
    delete(screenShader);
    delete(skyboxShader);
    delete(equirectToCubemapShader);
    delete(irradianceShader);
    delete(prefilterShader);
    delete(brdfShader);
}

// Renders a 1x1 3D cube in NDC
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        constexpr float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderQuad()
{
    if (quadVAO == 0)
    {
        constexpr float QUAD_VERTICIES[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICIES), &QUAD_VERTICIES, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lumina Engine", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    sceneSetup(window);

    while (!glfwWindowShouldClose(window))
    {
        const double currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrameTime;
        if (isLeftMouseHolding)
        {
            mouseHoldDuration = currentTime - mouseHoldStartTime;
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED &&
                mouseHoldDuration >= DURATION_TO_MOUSE_HOLD)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
        lastFrameTime = currentTime;

        renderLoop(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    deinit();

    return 0;
}
