#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float gamma;
uniform float exposure;

void main()
{
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;

    // Exposure tone mapping
    vec3 colorOut = vec3(1.0) - exp(-hdrColor * exposure);
    // Gamma correction
    colorOut = pow(colorOut, vec3(1.0/gamma));

    FragColor = vec4(colorOut, 1.0);
}
