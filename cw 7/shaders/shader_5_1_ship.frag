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
    // Pobierz i przeksztalc normalna z mapy normalnych
    vec3 tangentNormal = texture(normalMap, texCoord).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
    vec3 normal = normalize(TBN * tangentNormal);

    // Kierunki swiatla i widoku
    vec3 lightDir = normalize(lightPos - worldPos);
    vec3 viewDir  = normalize(cameraPos - worldPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    // Tekstura koloru
    vec3 albedo = texture(colorTexture, texCoord).rgb;

    // Oswietlenie
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient = AMBIENT * albedo;
    vec3 diffuse = diff * albedo;
    vec3 specular = spec * vec3(1.0); // biale odbicie

    // Finalny kolor
    vec3 color = ambient + diffuse + specular;
    outColor = vec4(color, 1.0);
}
