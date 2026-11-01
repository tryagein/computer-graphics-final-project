#version 330 core
struct PointLight { vec3 position; vec3 color; float intensity; };
struct DirectionalLight { vec3 direction; vec3 color; float intensity; };
uniform PointLight pointLights[4];
uniform DirectionalLight directionalLights[2];
uniform int pointCount;
uniform int directionalCount;
uniform vec3 baseColor;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 emissionColor;
uniform float emissionStrength;
uniform vec3 viewPos;
uniform float shininess;
uniform bool hasTexture;
uniform sampler2D diffuseTexture;
uniform bool shadowEnabled;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform float shadowBias;
uniform int shadowPcfRadius;
in vec3 worldPos;
in vec3 normal;
in vec2 uv;
out vec4 FragColor;

vec3 albedo;
vec3 shade(vec3 lightDir, vec3 color, float intensity, vec3 n, vec3 viewDir) {
    float diffuse = max(dot(n, lightDir), 0.0);
    vec3 halfVec = normalize(lightDir + viewDir);
    float specular = pow(max(dot(n, halfVec), 0.0), shininess);
    return intensity * color * (albedo * diffuseColor * diffuse + specularColor * specular);
}

float shadowVisibility(vec3 n, vec3 lightDir) {
    vec4 lightPosition = lightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projected = lightPosition.xyz / lightPosition.w * 0.5 + 0.5;
    if (projected.x < 0.0 || projected.x > 1.0 || projected.y < 0.0 ||
        projected.y > 1.0 || projected.z < 0.0 || projected.z > 1.0) return 1.0;
    float bias = max(shadowBias * (1.0 - max(dot(n, lightDir), 0.0)), shadowBias * 0.25);
    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    float visible = 0.0;
    float samples = 0.0;
    for (int x = -2; x <= 2; ++x)
        for (int y = -2; y <= 2; ++y) {
            if (abs(x) > shadowPcfRadius || abs(y) > shadowPcfRadius) continue;
            visible += projected.z - bias <= texture(shadowMap, projected.xy + vec2(x, y) * texel).r
                ? 1.0 : 0.0;
            samples += 1.0;
        }
    return visible / samples;
}

void main() {
    albedo = baseColor * (hasTexture ? texture(diffuseTexture, uv).rgb : vec3(1.0));
    vec3 n = normalize(normal);
    vec3 v = normalize(viewPos - worldPos);
    vec3 result = ambientColor * albedo;
    for (int i = 0; i < pointCount; ++i) {
        vec3 offset = pointLights[i].position - worldPos;
        float distanceToLight = length(offset);
        result += shade(normalize(offset), pointLights[i].color,
            pointLights[i].intensity / (1.0 + 0.09 * distanceToLight +
            0.032 * distanceToLight * distanceToLight), n, v);
    }
    for (int i = 0; i < directionalCount; ++i) {
        vec3 direction = normalize(-directionalLights[i].direction);
        float visibility = (shadowEnabled && i == 0) ? shadowVisibility(n, direction) : 1.0;
        result += visibility * shade(direction, directionalLights[i].color,
                                     directionalLights[i].intensity, n, v);
    }
    FragColor = vec4(result + emissionColor * emissionStrength, 1.0);
}
