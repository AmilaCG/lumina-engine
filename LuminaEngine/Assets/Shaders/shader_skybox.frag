#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube skybox;

void main()
{
//    FragColor = texture(skybox, WorldPos);
    FragColor = vec4(textureLod(skybox, WorldPos, 1.2).rgb, 1.0);
}
