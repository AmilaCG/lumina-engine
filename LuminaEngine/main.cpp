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

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float MOUSE_SENSITIVITY = 0.1f;
const float DURATION_TO_MOUSE_HOLD = 0.1f; // In seconds

const char* OBJ_V_SHADER_PATH = "Assets/Shaders/shader_object.vert";
const char* OBJ_F_SHADER_PATH = "Assets/Shaders/shader_object.frag";
const std::string BACKPACK_MODEL_PATH = "Assets/Models/backpack.obj";

const char* LIGHT_V_SHADER_PATH = "Assets/Shaders/shader_light.vert";
const char* LIGHT_F_SHADER_PATH = "Assets/Shaders/shader_light.frag";

const char* SCR_V_SHADER_PATH = "Assets/Shaders/shader_screen.vert";
const char* SCR_F_SHADER_PATH = "Assets/Shaders/shader_screen.frag";

const char* SKYBOX_V_SHADER_PATH = "Assets/Shaders/shader_skybox.vert";
const char* SKYBOX_F_SHADER_PATH = "Assets/Shaders/shader_skybox.frag";

static float pos[3];
static float rot[3];
static float scale[] = {1.0f, 1.0f, 1.0f};
bool shouldEnableReflections;
bool shouldEnableRefractions;
static bool show_skybox;

const glm::vec3 world_front(0.0f, 0.0f, -1.0f);
const glm::vec3 world_up(0.0f, 1.0f, 0.0f);

glm::vec3 cameraPosition(0.0f, 0.0f, 5.0f);
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

Model* guitarBackpackModel;
LightPreview* lightPreview;
Shader* backpackShader;
Shader* lightShader;
Shader* screenShader;
Shader* skyboxShader;

unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int rbo;
unsigned int quadVAO;
unsigned int quadVBO;

unsigned int texCubemap;
std::string cubemapFaces[] = {
    "Assets/Textures/Skybox/right.jpg",
    "Assets/Textures/Skybox/left.jpg",
    "Assets/Textures/Skybox/top.jpg",
    "Assets/Textures/Skybox/bottom.jpg",
    "Assets/Textures/Skybox/front.jpg",
    "Assets/Textures/Skybox/back.jpg",
};
unsigned int skyboxVAO;
unsigned int skyboxVBO;

glm::vec3 pointLightPositions[] = {
        glm::vec3(1.2f,  0.2f,  2.0f),
        glm::vec3(-1.2f,  0.5f,  2.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
};

glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
};

// Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates
float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

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
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    guitarBackpackModel = new Model(BACKPACK_MODEL_PATH);
    backpackShader = new Shader(OBJ_V_SHADER_PATH, OBJ_F_SHADER_PATH);
    lightPreview = new LightPreview();
    lightShader = new Shader(LIGHT_V_SHADER_PATH, LIGHT_F_SHADER_PATH);
    screenShader = new Shader(SCR_V_SHADER_PATH, SCR_F_SHADER_PATH);
    skyboxShader = new Shader(SKYBOX_V_SHADER_PATH, SKYBOX_F_SHADER_PATH);

    // IMGUI setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Screen quad VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

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
    glBindBuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Load cubemap
    texCubemap = TextureUtils::loadCubemapTexture(std::to_array(cubemapFaces), true);

    // Skybox VAO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void setLightParameters()
{
    glm::vec3 ambient(0.05);
    glm::vec3 diffuse(0.8f);
    glm::vec3 specular(1.0f);

    // Directional light
    backpackShader->setBool("dirLight.isActive", true);
    backpackShader->setVec3("dirLightDirection", glm::vec3(-0.2f, -1.0f, -0.3f));
    backpackShader->setVec3("dirLight.ambient", ambient);
    backpackShader->setVec3("dirLight.diffuse", diffuse);
    backpackShader->setVec3("dirLight.specular", specular);

    // Point light
    int i = 0;
    for (const glm::vec3& position : pointLightPositions)
    {
        std::ostringstream ossLightPos;
        ossLightPos << "pointLightPos[" << i << "]";
        std::string pointLightPos = ossLightPos.str();
        backpackShader->setVec3(pointLightPos, position);

        std::ostringstream ossLightParams;
        ossLightParams << "pointLights[" << i << "]";
        std::string pointLight = ossLightParams.str();
        backpackShader->setBool(pointLight + ".isActive", true);
        backpackShader->setVec3(pointLight + ".ambient", ambient * pointLightColors[i]);
        backpackShader->setVec3(pointLight + ".diffuse", diffuse * pointLightColors[i]);
        backpackShader->setVec3(pointLight + ".specular", specular);
        // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        backpackShader->setFloat(pointLight + ".constant", 1.0f);
        backpackShader->setFloat(pointLight + ".linear", 0.09f);
        backpackShader->setFloat(pointLight + ".quadratic", 0.032f);
        i++;
    }

    // Spot light
    backpackShader->setBool("spotLights[0].isActive", false);
    backpackShader->setVec3("spotLightPos[0]", cameraPosition);
    backpackShader->setVec3("spotLightDir[0]", cameraFront);
    backpackShader->setFloat("spotLights[0].cutOff", glm::cos(glm::radians(12.5f)));
    backpackShader->setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(18.5f)));
    backpackShader->setVec3("spotLights[0].ambient", glm::vec3(0.2f));
    backpackShader->setVec3("spotLights[0].diffuse", diffuse);
    backpackShader->setVec3("spotLights[0].specular", specular);
    backpackShader->setFloat("spotLights[0].constant", 1.0f);
    backpackShader->setFloat("spotLights[0].linear", 0.09f);
    backpackShader->setFloat("spotLights[0].quadratic", 0.032f);
}

void renderLoop(GLFWwindow* window)
{
    constexpr unsigned int skyboxTexUnit = 3; // Reserving unit 0, 1 and 2 for diff, spec and normal maps
    unsigned int indiceCount = 0;
    processInput(window);
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, world_up);
    const glm::mat4 projection = glm::perspective(glm::radians(fov),
        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);

    // Bind to framebuffer and draw scene as we normally would to color texture
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST); // Enable depth testing (disabled for rendering screen-space quad)

    glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    backpackShader->use();
    setLightParameters();
    backpackShader->setFloat("material.shininess", 192.0f);
    backpackShader->setMat4("view", view);
    backpackShader->setMat4("projection", projection);
    backpackShader->setVec3("viewPos", cameraPosition);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rot[0]), glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, glm::radians(rot[1]), glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, glm::radians(rot[2]), glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(pos[0], pos[1], pos[2]));
    model = glm::scale(model, glm::vec3(scale[0], scale[1], scale[2]));
    backpackShader->setMat4("model", model);

    // Activate and bind skybox texture for reflections before drawing the model
    glActiveTexture(GL_TEXTURE0 + skyboxTexUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texCubemap);
    backpackShader->setInt("skybox", skyboxTexUnit);
    backpackShader->setBool("shouldEnableReflections", shouldEnableReflections);
    backpackShader->setBool("shouldEnableRefractions", shouldEnableRefractions);

    indiceCount += guitarBackpackModel->Draw(*backpackShader);

    lightShader->use();
    lightShader->setMat4("view", view);
    lightShader->setMat4("projection", projection);
    unsigned int i = 0;
    for (const glm::vec3& lightPos : pointLightPositions)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightShader->setMat4("model", model);

        lightPreview->Draw(*lightShader, pointLightColors[i++]);
    }

    if (show_skybox)
    {
        // Draw the skybox
        // Depth test passes when values are equal to depth buffer's content. Even though ideally this
        // should be GL_EQUAL, some artifacts will occur on the skybox when panning because sometimes
        // incoming pixel depth value < depth value in depth buffer, so we use GL_LEQUAL to avoid them
        glDepthFunc(GL_LEQUAL);
        skyboxShader->use();
        // Remove translation section of the view transform matrix by taking only the upper-left 3x3 matrix,
        // so the skybox will rotate but not scale or move.
        const auto viewSkybox = glm::mat4(glm::mat3(view));
        skyboxShader->setMat4("view", viewSkybox);
        skyboxShader->setMat4("projection", projection);
        skyboxShader->setInt("skybox", skyboxTexUnit);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS); // Set depth function back to default
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Back to default framebuffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader->use();
    screenShader->setFloat("gamma", 2.0f);
    screenShader->setFloat("exposure", 1.0f);
    screenShader->setInt("screenTexture", 0); // Expected texture unit is 0
    glBindVertexArray(quadVAO);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0); // Just making sure the correct texture unit is activated
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);

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

    // Start Env Reflections section
    const char* items[] = { "OFF", "Reflections ON", "Refractions ON" };
    static int item_current_idx = 0; // Here we store our selection data as an index.
    static int item_prev_idx = 0;

    static ImGuiComboFlags env_ref_combo_flags = 0;
    env_ref_combo_flags |= ImGuiComboFlags_WidthFitPreview;
    const char* combo_preview_value = items[item_current_idx];
    if (ImGui::BeginCombo("Reflections/Refractions", combo_preview_value, env_ref_combo_flags))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(items[n], is_selected))
                item_current_idx = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (item_current_idx != item_prev_idx)
    {
        switch (item_current_idx)
        {
            case 0:
                shouldEnableReflections = false;
                shouldEnableRefractions = false;
                break;
            case 1:
                shouldEnableReflections = true;
                shouldEnableRefractions = false;
                break;
            case 2:
                shouldEnableReflections = false;
                shouldEnableRefractions = true;
                break;
        }
        item_prev_idx = item_current_idx;
    }
    // End Env Reflections section

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
    glDeleteTextures(1, &texCubemap);
    delete(guitarBackpackModel);
    delete(backpackShader);
    delete(lightPreview);
    delete(lightShader);
    delete(screenShader);
    delete(skyboxShader);
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
