#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

const float pi = 3.14159265359;
const vec3 worldUp = vec3(0.0, 1.0, 0.0);

void main()
{
    // The world vector acts as the normal of a tangent surface
    // from the origin, aligned to WorldPos
    vec3 normal = normalize(WorldPos);
    vec3 right = normalize(cross(worldUp, normal));
    vec3 up = normalize(cross(normal, right));

    vec3 irradiance = vec3(0.0);

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * pi; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * pi; theta += sampleDelta)
        {
            // Spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),
                                      sin(theta) * sin(phi),
                                      cos(theta));
            // Tangent space to world space
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = pi * irradiance * (1.0 / float(nrSamples));

    FragColor = vec4(irradiance, 1.0);
}
