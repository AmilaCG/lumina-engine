#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    WorldPos = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    // How to trick the skybox depth value to be always 1.0:
    // Perspective division is performed after the vertex shader has run (gl_Position.xyz / w).
    // z component of the resulting division (z / w) = vertex depth value
    // Hence, making z = w will give us w / w = 1.0, a depth value of 1.0 all the time
    gl_Position = pos.xyww;
}
