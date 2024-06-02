#include <iostream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "Shader.h"
#include "LightPreview.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void renderLoop(GLFWwindow* window);
void setLightParameters();
glm::vec3 getCameraDirection(double yaw, double pitch);
void displayUI();
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

static float pos[3];
static float rot[3];
static float scale[] = {1.0f, 1.0f, 1.0f};

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

glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
};

glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
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

void setLightParameters()
{
    glm::vec3 ambient(0.05);
    glm::vec3 diffuse(0.8f);
    glm::vec3 specular(1.0f);

    // Directional light
    backpackShader->setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    backpackShader->setVec3("dirLight.ambient", ambient);
    backpackShader->setVec3("dirLight.diffuse", diffuse);
    backpackShader->setVec3("dirLight.specular", specular);

    // Point light
    int i = 0;
    for (const glm::vec3& position : pointLightPositions)
    {
        std::ostringstream oss;
        oss << "pointLights[" << i << "]";
        std::string pointLight = oss.str();

        backpackShader->setVec3(pointLight + ".position", position);
        backpackShader->setVec3(pointLight + ".ambient", ambient * pointLightColors[i]);
        backpackShader->setVec3(pointLight + ".diffuse", diffuse * pointLightColors[i]);
        backpackShader->setVec3(pointLight + ".specular", specular);
        // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        backpackShader->setFloat(pointLight + ".constant", 1.0f);
        backpackShader->setFloat(pointLight + ".diffuse", 0.09f);
        backpackShader->setFloat(pointLight + ".quadratic", 0.032f);
        i++;
    }

    // Spot light
    backpackShader->setVec3("spotLight.position", glm::vec3(cameraPosition));
    backpackShader->setVec3("spotLight.direction", glm::vec3(cameraFront));
    backpackShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    backpackShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(18.5f)));
    backpackShader->setVec3("spotLight.ambient", glm::vec3(0.2f));
    backpackShader->setVec3("spotLight.diffuse", diffuse);
    backpackShader->setVec3("spotLight.specular", specular);
    backpackShader->setFloat("spotLight.constant", 1.0f);
    backpackShader->setFloat("spotLight.diffuse", 0.09f);
    backpackShader->setFloat("spotLight.quadratic", 0.032f);
}

void renderLoop(GLFWwindow* window)
{
    processInput(window);
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    displayUI();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    backpackShader->use();

    setLightParameters();
    backpackShader->setFloat("material.shininess", 64.0f);

    glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, world_up);
    backpackShader->setMat4("view", view);

    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    backpackShader->setMat4("projection", projection);

    backpackShader->setVec3("viewPos", cameraPosition);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rot[0]), glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, glm::radians(rot[1]), glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, glm::radians(rot[2]), glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(pos[0], pos[1], pos[2]));
    model = glm::scale(model, glm::vec3(scale[0], scale[1], scale[2]));
    backpackShader->setMat4("model", model);

    guitarBackpackModel->Draw(*backpackShader);

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

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void displayUI()
{
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 110), ImGuiCond_Always);

    if (!ImGui::Begin("Transform", nullptr, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Transform");
    ImGui::DragFloat3("Position", pos, 0.001f);
    ImGui::DragFloat3("Rotation", rot, 0.01f);
    ImGui::DragFloat3("Scale", scale, 0.001f);

    ImGui::End();
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
    delete(guitarBackpackModel);
    delete(backpackShader);
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

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    guitarBackpackModel = new Model(BACKPACK_MODEL_PATH);
    backpackShader = new Shader(OBJ_V_SHADER_PATH, OBJ_F_SHADER_PATH);
    lightPreview = new LightPreview();
    lightShader = new Shader(LIGHT_V_SHADER_PATH, LIGHT_F_SHADER_PATH);

    // IMGUI setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
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
