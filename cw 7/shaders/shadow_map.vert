#version 430 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 lightSpaceMatrix;  // Macierz transformacji �wiat�a (projection * view)
uniform mat4 model;             // Macierz modelu obiektu

void main()
{
    // Transformuj pozycj� wierzcho�ka do przestrzeni �wiat�a
    // To jest wszystko czego potrzebujemy - OpenGL automatycznie zapisze g��boko��
    gl_Position = lightSpaceMatrix * model * vec4(vertexPosition, 1.0);
}