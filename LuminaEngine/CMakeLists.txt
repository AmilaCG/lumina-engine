cmake_minimum_required(VERSION 3.28)
project(LuminaEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(LuminaEngine
        main.cpp
        Mesh.cpp
        Model.cpp
        Shader.cpp
        LightPreview.cpp)

find_package(glad CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_include_directories(LuminaEngine PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(LuminaEngine PRIVATE
        glfw
        glad::glad
        glm::glm-header-only
        assimp::assimp
        imgui::imgui)

add_custom_target(ClearAssets ALL
        COMMAND ${CMAKE_COMMAND} -E rm -rf
        $<TARGET_FILE_DIR:LuminaEngine>/Assets/)

add_custom_target(CopyAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets/ $<TARGET_FILE_DIR:LuminaEngine>/Assets/)

add_dependencies(LuminaEngine CopyAssets)
add_dependencies(CopyAssets ClearAssets)
