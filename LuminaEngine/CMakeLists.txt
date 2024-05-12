cmake_minimum_required(VERSION 3.28)
project(LuminaEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(LuminaEngine main.cpp)

find_package(glad CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_include_directories(LuminaEngine PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(LuminaEngine PRIVATE
        glad::glad
        assimp::assimp
        glfw
        glm::glm-header-only
        imgui::imgui)
