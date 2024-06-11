#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float gamma;

void main()
{
    // Apply gamma correction
    vec3 colorOut = pow(texture(screenTexture, TexCoords).rgb, vec3(1.0/gamma));
    FragColor = vec4(colorOut, 1.0);
}
