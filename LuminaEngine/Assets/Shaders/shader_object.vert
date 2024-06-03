#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

#define NR_LIGHTS 10

out vec2 TexCoords;
out vec3 TangentDirLightDirection;
out vec3 TangentPointLightPos[NR_LIGHTS];
out vec3 TangentSpotLightPos[NR_LIGHTS];
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform vec3 dirLightDirection;
uniform vec3 pointLightPos[NR_LIGHTS];
uniform vec3 spotLightPos[NR_LIGHTS];

void main()
{
    TexCoords = aTexCoords;

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));
    TangentViewPos = TBN * viewPos;
    TangentFragPos = TBN * vec3(model * vec4(aPos, 1.0));
    TangentDirLightDirection = TBN * dirLightDirection;
    for (int i = 0; i < NR_LIGHTS; i++)
    {
        TangentPointLightPos[i] = TBN * pointLightPos[i];
        TangentSpotLightPos[i] = TBN * spotLightPos[i];
    }

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
