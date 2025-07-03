#version 430 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 lightSpaceMatrix;  // Macierz transformacji œwiat³a (projection * view)
uniform mat4 model;             // Macierz modelu obiektu

void main()
{
    // Transformuj pozycjê wierzcho³ka do przestrzeni œwiat³a
    // To jest wszystko czego potrzebujemy - OpenGL automatycznie zapisze g³êbokoœæ
    gl_Position = lightSpaceMatrix * model * vec4(vertexPosition, 1.0);
}