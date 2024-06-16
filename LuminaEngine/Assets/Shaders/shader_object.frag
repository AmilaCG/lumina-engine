#version 330 core

#define NR_LIGHTS 5

out vec4 FragColor;
in vec2 TexCoords;
in vec3 TangentDirLightDirection;
in vec3 TangentPointLightPos[NR_LIGHTS];
in vec3 TangentSpotLightPos[NR_LIGHTS];
in vec3 TangentSpotLightDir[NR_LIGHTS];
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in mat3 inversedTBN;

struct DirLight
{
    bool isActive;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    bool isActive;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight
{
    bool isActive;

    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float     shininess;
};

uniform DirLight dirLight;
uniform PointLight pointLights[NR_LIGHTS];
uniform SpotLight spotLights[NR_LIGHTS];
uniform Material material;
uniform samplerCube skybox;
uniform bool shouldEnableReflections;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcDiffuse(vec3 color, vec3 normal, vec3 lightDir);
vec3 calcSpecular(vec3 color, vec3 normal, vec3 lightDir, vec3 viewDir);

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-TangentDirLightDirection);

    vec3 ambient = texture(material.texture_diffuse1, TexCoords).rgb * light.ambient;
    vec3 diffuse = calcDiffuse(light.diffuse, normal, lightDir);
    vec3 specular = calcSpecular(light.specular, normal, lightDir, viewDir);

    return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Light direction from fragment to light
    vec3 lightDir = normalize(lightPos - fragPos);

    vec3 ambient = texture(material.texture_diffuse1, TexCoords).rgb * light.ambient;
    vec3 diffuse = calcDiffuse(light.diffuse, normal, lightDir);
    vec3 specular = calcSpecular(light.specular, normal, lightDir, viewDir);

    // Attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                            light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 lightPos, vec3 spotDir, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Light direction from fragment to light
    vec3 lightDir = normalize(lightPos - fragPos);

    vec3 ambient = texture(material.texture_diffuse1, TexCoords).rgb * light.ambient;
    vec3 diffuse = calcDiffuse(light.diffuse, normal, lightDir);
    vec3 specular = calcSpecular(light.specular, normal, lightDir, viewDir);

    // Spotlight with soft edges
    float theta = dot(lightDir, normalize(-spotDir));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    // Attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                            light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calcDiffuse(vec3 color, vec3 normal, vec3 lightDir)
{
    float diff = max(dot(normal, lightDir), 0.0);
    return (diff * texture(material.texture_diffuse1, TexCoords).rgb * color);
}

vec3 calcSpecular(vec3 color, vec3 normal, vec3 lightDir, vec3 viewDir)
{
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    return (spec * texture(material.texture_specular1, TexCoords).rgb * color);
}

void main()
{
    vec3 normal = texture(material.texture_normal1, TexCoords).rgb;
    vec3 norm = normalize(normal * 2.0 - 1.0);

    // Light reflection from fragment to camera/eye
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);

    vec3 result;

    // Phase 1: Directional lighting
    if (dirLight.isActive)
    {
        result += calcDirLight(dirLight, norm, viewDir);
    }

    for (int i = 0; i < NR_LIGHTS; i++)
    {
        // Phase 2: Point lights
        if (pointLights[i].isActive)
        {
            result += calcPointLight(pointLights[i],
                                     TangentPointLightPos[i],
                                     norm,
                                     TangentFragPos,
                                     viewDir);
        }

        // Phase 3: Spot light
        if (spotLights[i].isActive)
        {
            result += calcSpotLight(spotLights[i],
                                  TangentSpotLightPos[i],
                                  TangentSpotLightDir[i],
                                  norm,
                                  TangentFragPos,
                                  viewDir);
        }
    }

    if (shouldEnableReflections)
    {
        // Calculating environment reflections
        // Note that we are converting the reflection vector from tangent space to model space
        // because vidwDir and norm are in tangent space
        vec3 R = inversedTBN * reflect(-viewDir, norm);
        vec3 envReflection = texture(skybox, R).rgb;
        result *= envReflection * 10;
    }

    FragColor = vec4(result, 1.0);
}
