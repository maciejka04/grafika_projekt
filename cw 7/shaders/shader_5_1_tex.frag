#version 430 core

uniform sampler2D colorTexture;
uniform sampler2D normalMap;

uniform vec3 lightPos;   // not used anymore, but you can keep it if needed
uniform vec3 cameraPos;

in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN;

out vec4 outColor;

float AMBIENT = 0.4;
float shininess = 32.0;

void main()
{
    vec3 tangentNormal = texture(normalMap, texCoord).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
    vec3 normal = normalize(TBN * tangentNormal);

    vec3 lightDir = normalize(vec3(0.2, -1.0, 0.2));
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    vec3 textureColor = texture(colorTexture, texCoord).rgb;
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient = AMBIENT * textureColor;
    vec3 diffuse = diff * textureColor;
    vec3 specular = spec * vec3(1.0);

    vec3 color = ambient + diffuse + specular;

    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}