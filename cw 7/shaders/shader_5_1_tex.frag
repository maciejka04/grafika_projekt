#version 430 core
uniform sampler2D colorTexture;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform int receiveShadows;

float AMBIENT = 0.3;
float shininess = 32.0;

in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN;
in vec4 fragPosLightSpace;

out vec4 outColor;

float calculateShadow(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    projCoords = projCoords * 0.5 + 0.5;
    
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 || 
        projCoords.z > 1.0) {
        return 0.0; // No shadow
    }
    
    float shadowDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    float bias = 0.005;
    
    float shadow = currentDepth - bias > shadowDepth ? 1.0 : 0.0;
    
    return shadow;
}

void main() {
    vec3 albedo = texture(colorTexture, texCoord).rgb;
    
    vec3 normalMapVal = texture(normalMap, texCoord).rgb;
    vec3 normal = normalize(TBN * (normalMapVal * 2.0 - 1.0));
    
    vec3 lightDir = normalize(lightPos - worldPos);
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    
    float diff = max(dot(normal, lightDir), 0.0);
    diff *=1.2f;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    spec *=1.2f;
    
    vec3 baseColor = 0.4 * albedo + diff * albedo + 0.3 * spec * vec3(1.0);
    
    if (receiveShadows == 0) {
        outColor = vec4(baseColor, 1.0);
        return;
    }
    
    float shadowFactor = calculateShadow(fragPosLightSpace);
    
    vec3 ambient = 0.4 * albedo;
    vec3 diffuseAndSpecular = diff * albedo + 0.3 * spec * vec3(1.0);
    
 
    vec3 finalColor = ambient + (1.0 - shadowFactor * 0.7) * diffuseAndSpecular;
    
    outColor = vec4(finalColor, 1.0);
}