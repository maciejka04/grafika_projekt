#version 430 core

uniform sampler2D colorTexture;
uniform sampler2D normalMap;

uniform vec3 lightPos;
uniform vec3 cameraPos;

float AMBIENT = 0.3;
float shininess = 32.0; // standard Phong shininess

in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN;

out vec4 outColor;

void main()
{
    // Normalna z mapy normalnych
    vec3 normalMapVal = texture(normalMap, texCoord).rgb;
    vec3 normal = normalize(TBN * (normalMapVal * 2.0 - 1.0));

    // Oœwietlenie
    vec3 lightDir = normalize(lightPos - worldPos);
    vec3 viewDir  = normalize(cameraPos - worldPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    vec3 albedo = texture(colorTexture, texCoord).rgb;

    // Oswietlenie
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0); // uproszczone shininess

    //vec3 color = 0.2 * albedo + diff * albedo + spec * vec3(1.0); // ambient + diffuse + specular
    vec3 color = 0.4 * albedo + diff * albedo + 0.3 * spec * vec3(1.0);
    //vec3 color = 0.2 * albedo + diff * albedo + spec * albedo;
    outColor = vec4(color, 1.0);
}
