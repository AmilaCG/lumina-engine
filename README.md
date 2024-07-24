# Lumina Engine
A simple, real-time PBR ([Physically Based Rendering](https://en.wikipedia.org/wiki/Physically_based_rendering)) engine built while learning from [learnopengl.com](https://learnopengl.com/).

## Current Progress
![Demo](Docs/demo.png)
*Snapshot of the current progress. [3D model](https://skfb.ly/6QZxW) by Berk Gedik is licensed under [Creative Commons Attribution](http://creativecommons.org/licenses/by/4.0/)*.

<br>

| Feature | Status |
| ---- | ---- |
| Model loading using [assimp](https://github.com/assimp/assimp) | Done |
| Basic GUI setup using [Dear ImGui](https://github.com/ocornut/imgui) | Done |
| Diffuse, Specular and Normal map support | Done |
| Blinn-Phong lighting | Done |
| Gamma correction | Done |
| Skybox support | Done |
| Environment reflections & refractions | Done |
| Anti-aliasing | To do |
| Shadows | To do |
| HDR (High Dynamic Range) | Done |
| Bloom | Done |
| SSAO (Screen-Space Ambient Occlusion) | To do |
| PBR lighting | Done |

## Prerequisites
1. OpenGL 3.3 or above supported graphics hardware
2. [Git](https://git-scm.com/download/win)
3. [CMake](https://cmake.org/download/)
4. [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-cmd) (Make sure to [set the VCPKG_ROOT environment variable](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-cmd#2---set-up-the-project))

## How to Build and Run (Windows)
1. Clone the repository
```
git clone https://github.com/AmilaCG/lumina-engine.git
```
2. Go into LuminaEngine directory
```
cd .\lumina-engine\LuminaEngine\
```
3. Make a build directory and navigate into it
```
mkdir build; cd build
```
4. Setup CMake
```
cmake ..
```
5. Build
```
cmake --build .
```
6. Run
```
cd .\Debug\; .\LuminaEngine.exe
```

Similar steps can be followed for Linux platforms (haven't tested on Linux)
