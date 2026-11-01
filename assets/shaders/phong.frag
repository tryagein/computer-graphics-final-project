#version 330 core
struct PointLight{vec3 position;vec3 color;float intensity;};
struct DirectionalLight{vec3 direction;vec3 color;float intensity;};
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
in vec3 worldPos;
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
vec3 albedo;
vec3 shade(vec3 lightDir,vec3 color,float intensity,vec3 n,vec3 viewDir){float diffuse=max(dot(n,lightDir),0.0);vec3 halfVec=normalize(lightDir+viewDir);float specular=pow(max(dot(n,halfVec),0.0),shininess);return intensity*color*(albedo*diffuseColor*diffuse+specularColor*specular);}
void main(){albedo=baseColor*(hasTexture?texture(diffuseTexture,uv).rgb:vec3(1.0));vec3 n=normalize(normal);vec3 v=normalize(viewPos-worldPos);vec3 result=ambientColor*albedo;for(int i=0;i<pointCount;++i){vec3 d=pointLights[i].position-worldPos;float r=length(d);result+=shade(normalize(d),pointLights[i].color,pointLights[i].intensity/(1.0+0.09*r+0.032*r*r),n,v);}for(int i=0;i<directionalCount;++i){result+=shade(normalize(-directionalLights[i].direction),directionalLights[i].color,directionalLights[i].intensity,n,v);}result+=emissionColor*emissionStrength;FragColor=vec4(result,1.0);}
