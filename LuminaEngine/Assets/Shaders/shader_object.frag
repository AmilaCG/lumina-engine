#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;

#define NR_POINT_LIGHTS 50
#define NR_SPOT_LIGHTS  50

struct DirLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    bool isActive;
    vec3 position;

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
    vec3 position;
    vec3 direction;
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
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform Material material;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    // Ambient
    vec3 ambient = texture(material.texture_diffuse1, TexCoords).rgb * light.ambient;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(material.texture_diffuse1, TexCoords).rgb * light.diffuse;

    // Specular
    // The reflect function expects the first vector to point
    // from the light source towards the fragment's position. Therefore,
    // we are getting the negative value.
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * texture(material.texture_specular1, TexCoords).rgb * light.specular;

    return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Light direction from fragment to light
    vec3 lightDir = normalize(light.position - fragPos);

    // Ambient
    vec3 ambient = texture(material.texture_diffuse1, TexCoords).rgb * light.ambient;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(material.texture_diffuse1, TexCoords).rgb * light.diffuse;

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * texture(material.texture_specular1, TexCoords).rgb * light.specular;

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                            light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Light direction from fragment to light
    vec3 lightDir = normalize(light.position - fragPos);

    // Ambient
    vec3 ambient = texture(material.texture_diffuse1, TexCoords).rgb * light.ambient;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(material.texture_diffuse1, TexCoords).rgb * light.diffuse;

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * texture(material.texture_specular1, TexCoords).rgb * light.specular;

    // Spotlight with soft edges
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                            light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
    vec3 normal = texture(material.texture_normal1, TexCoords).rgb;
    vec3 norm = normalize(normal * 2.0 - 1.0);

    // Light reflection from fragment to camera/eye
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result;

    // Phase 1: Directional lighting
    result += calcDirLight(dirLight, norm, viewDir);
    // Phase 2: Point lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        if (!pointLights[i].isActive) { continue; }
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    // Phase 3: Spot light
    for (int i = 0; i < NR_SPOT_LIGHTS; i++)
    {
        if (!spotLights[i].isActive) { continue; }
        result += calcSpotLight(spotLights[i], norm, FragPos, viewDir);
    }

    FragColor = vec4(result, 1.0);
}