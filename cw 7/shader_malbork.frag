#version 430 core
uniform sampler2D colorTexture;
in vec2 texCoord;
out vec4 outColor;

void main() {
    vec3 textureColor = texture(colorTexture, texCoord).rgb;
    if (textureColor == vec3(0.0)) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0); // Czerwony, jeœli tekstura nie dzia³a
    } else {
        outColor = vec4(textureColor, 1.0);
    }
}