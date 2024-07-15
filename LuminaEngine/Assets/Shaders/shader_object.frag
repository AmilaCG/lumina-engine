#version 330 core

#define NR_LIGHTS 5

const float PI = 3.14159265359;

out vec4 FragColor;
in vec2 TexCoords;
in vec3 TangentDirLightDirection;
in vec3 TangentPointLightPos[NR_LIGHTS];
in vec3 TangentSpotLightPos[NR_LIGHTS];
in vec3 TangentSpotLightDir[NR_LIGHTS];
in vec3 TangentCamPos;
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

struct MaterialPbr
{
    sampler2D texture_albedo1;
    sampler2D texture_metallic1;
    sampler2D texture_roughness1;
    sampler2D texture_normal1;
    sampler2D texture_ao1;
};

uniform bool isPbr;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_LIGHTS];
uniform SpotLight spotLights[NR_LIGHTS];
uniform Material material;
uniform MaterialPbr materialPbr;
uniform samplerCube skybox;
uniform bool shouldEnableReflections;
uniform bool shouldEnableRefractions;

vec3 Lo = vec3(0.0);

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcPbrPointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcDiffuse(vec3 color, vec3 normal, vec3 lightDir);
vec3 calcSpecular(vec3 color, vec3 normal, vec3 lightDir, vec3 viewDir);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

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

vec3 calcPbrPointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 albedo = texture(materialPbr.texture_albedo1, TexCoords).rgb;
    float metallic = texture(materialPbr.texture_metallic1, TexCoords).r;
    float roughness = texture(materialPbr.texture_roughness1, TexCoords).r;
    float ao = texture(materialPbr.texture_ao1, TexCoords).r;

    vec3 N = normal;
    vec3 V = viewDir;
    vec3 L = normalize(lightPos - fragPos); // Light direction
    vec3 H = normalize(V + L); // Halfway direction

    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.diffuse * attenuation;

    vec3 F0 = vec3(0.04); // Non-metallic / dielectric
    F0 = mix(F0, albedo, metallic);
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);

    // Calculating Cook-Torrance BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F; // Reflected light energy
    vec3 kD = vec3(1.0) - kS; // Refracted light energy

    kD *= 1.0 - metallic; // Since metals doesn't refract, nullifying kD with metallic value

    float NdotL = max(dot(N, L), 0.0); // Lambert (wi dot n)
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    return color;
}

// Trowbridge-Reitz GGX
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness; // alfa
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
    vec3 normal;
    if (isPbr)
    {
        normal = texture(materialPbr.texture_normal1, TexCoords).rgb;
    }
    else
    {
        normal = texture(material.texture_normal1, TexCoords).rgb;
    }
    normal = normalize(normal * 2.0 - 1.0);

    // Light reflection from fragment to camera/eye
    vec3 viewDir = normalize(TangentCamPos - TangentFragPos);

    vec3 result;

    // Phase 1: Directional lighting
    if (dirLight.isActive)
    {
        result += calcDirLight(dirLight, normal, viewDir);
    }

    for (int i = 0; i < NR_LIGHTS; i++)
    {
        // Phase 2: Point lights
        if (pointLights[i].isActive)
        {
            if (isPbr)
            {
                result += calcPbrPointLight(pointLights[i],
                                            TangentPointLightPos[i],
                                            normal,
                                            TangentFragPos,
                                            viewDir);
            }
            else
            {
                result += calcPointLight(pointLights[i],
                                         TangentPointLightPos[i],
                                         normal,
                                         TangentFragPos,
                                         viewDir);
            }
        }

        // Phase 3: Spot light
        if (spotLights[i].isActive)
        {
            result += calcSpotLight(spotLights[i],
                                  TangentSpotLightPos[i],
                                  TangentSpotLightDir[i],
                                  normal,
                                  TangentFragPos,
                                  viewDir);
        }
    }

    if (shouldEnableReflections)
    {
        // Calculating environment reflections
        vec3 I = -viewDir;
        vec3 R = reflect(I, normal);
        // Note that we are converting the reflection vector from tangent space to model space
        // because vidwDir and norm are in tangent space
        vec3 envReflection = texture(skybox, inversedTBN * R).rgb;
        result = envReflection;
    }

    if (shouldEnableRefractions)
    {
        const float airRefractiveIndex = 1.00;
        const float glassRefractiveIndex = 1.52;
        float ratio = airRefractiveIndex / glassRefractiveIndex;
        vec3 I = -viewDir;
        vec3 R = refract(I, normal, ratio);
        vec3 envRefraction = texture(skybox, inversedTBN * R).rgb;
        result = envRefraction;
    }

    FragColor = vec4(result, 1.0);
}
