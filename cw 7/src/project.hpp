#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <fstream>  

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

float skyboxVertices[] = {
	// positions
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

namespace texture {
	GLuint earth;
	GLuint clouds;
	GLuint moon;
	GLuint ship;
	GLuint shipNormal;
	GLuint europeNormal;
	GLuint europe;
	GLuint grid;
	GLuint earthNormal;
	GLuint earthRough;
	GLuint earthMetal;
	GLuint earthAO;
	GLuint earthHeight;
	GLuint tower;
	GLuint towerNormal;
	GLuint castle;
	GLuint castleNormal;
	GLuint katedra;
	GLuint katedraNormal;
	GLuint bigben;
	GLuint bigbenNormal;
	GLuint koloseum;
	GLuint watykan;
	GLuint watykanNormal;
	GLuint piza;
	GLuint inowroclaw;
}

GLuint skyboxCubemap;

GLuint program;
GLuint programSun;
GLuint programTex;
GLuint programEarth;
GLuint programProcTex;
Core::Shader_Loader shaderLoader;

Core::RenderContext sphereContext;
Core::RenderContext europeContext;
Core::RenderContext towerContext;
Core::RenderContext castleContext;
Core::RenderContext katedraContext;
Core::RenderContext bigbenContext;
Core::RenderContext watykanContext;
Core::RenderContext pizaContext;
Core::RenderContext inowroclawContext;
Core::RenderContext sunSphereContext;  


glm::vec3 defaultCameraPos = glm::vec3(-2.02047f, 3.41489f, 6.79471f);
glm::vec3 defaultCameraTarget = glm::vec3(-2.10608f, 2.8615f, 5.9662f);
glm::vec3 cameraPos = defaultCameraPos;
glm::vec3 cameraFront = glm::normalize(defaultCameraTarget - defaultCameraPos);
glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

glm::vec3 europeStartPos = glm::vec3(0.0f, 0.0f, 0.0f);
GLuint VAO, VBO;
GLuint skyboxVAO, skyboxVBO;
float cameraSpeed = 0.025f;
float aspectRatio = 1.f;

float roughness = 0.0f;
float roughnessShip = 0.0f;
float metallic = 0.0f;
float metallicShip = 0.0f;
float glowThreshold = 0.3f;

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
bool firstMouse = true;


bool isRecording = false;
float recordingStartTime = 0.0f;
bool animationStarted = false;
bool animationFinished = false;
float animationStartTime = 0.0f;

struct CameraKeyframe {
	glm::vec3 position;
	glm::vec3 target;
	float time;
};

std::vector<CameraKeyframe> recordedKeyframes;
std::vector<std::vector<CameraKeyframe>> queue;
std::vector<CameraKeyframe> currentPath;
float currentAnimationStartTime = 0.0f;
bool isAnimating = false;
std::vector<CameraKeyframe> keyframes = {};


std::vector<CameraKeyframe> benkeyframes = {
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, -1.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 0.0f },
	{ {-2.2424f, 3.02749f, 5.84442f}, {-2.45631f, 2.69697f, 4.92518f}, 0.5f },
	{ {-2.43335f, 2.73267f, 5.01581f}, {-2.63687f, 2.41371f, 4.09015f}, 1.0f },
	{ {-2.67248f, 2.35789f, 3.92817f}, {-2.876f, 2.03893f, 3.00251f}, 1.5f },
	{ {-2.92088f, 1.96916f, 2.79336f}, {-3.12012f, 1.65848f, 1.86396f}, 2.0f },
	{ {-3.1616f, 1.59444f, 1.65223f}, {-3.35062f, 1.30207f, 0.714788f}, 2.5f },
	{ {-3.39817f, 1.23403f, 0.478941f}, {-3.58779f, 0.951689f, -0.461448f}, 3.0f },
	{ {-3.6595f, 0.856687f, -0.817104f}, {-3.85078f, 0.604618f, -1.76572f}, 3.5f },
	{ {-4.75975f, 0.823965f, -1.41527f}, {-4.02448f, 0.592218f, -2.05219f}, 4.5f },
	{ {-4.9474f, 0.823965f, -2.41668f}, {-4.00516f, 0.59052f, -2.1765f}, 5.5f },
	{ {-3.92581f, 0.823965f, -3.1877f}, {-3.89183f, 0.595615f, -2.21472f}, 6.5f },
	{ {-2.98385f, 0.823965f, -2.35678f}, {-3.91696f, 0.583737f, -2.08922f}, 7.5f },
	{ {-3.66722f, 0.823965f, -1.21463f}, {-3.98094f, 0.556727f, -2.12576f}, 8.5f },
	{ {-3.6595f, 0.856687f, -0.817104f}, {-3.85078f, 0.604618f, -1.76572f}, 9.0f },
	{ {-3.39817f, 1.23403f, 0.478941f}, {-3.58779f, 0.951689f, -0.461448f}, 9.5f },
	{ {-3.1616f, 1.59444f, 1.65223f}, {-3.35062f, 1.30207f, 0.714788f}, 10.0f },
	{ {-2.92088f, 1.96916f, 2.79336f}, {-3.12012f, 1.65848f, 1.86396f}, 10.5f },
	{ {-2.67248f, 2.35789f, 3.92817f}, {-2.876f, 2.03893f, 3.00251f}, 11.0f },
	{ {-2.43335f, 2.73267f, 5.01581f}, {-2.63687f, 2.41371f, 4.09015f}, 11.5f },
	{ {-2.2424f, 3.02749f, 5.84442f}, {-2.45631f, 2.69697f, 4.92518f}, 12.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 12.5f },
	{ {-2.03047f, 3.42489f, 6.80471f}, {-2.11608f, 2.8715f, 5.9762f}, 13.0f },
};

std::vector<CameraKeyframe> wroclawkeyframes = {
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, -1.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 0.0f },
	{ {-1.60726f, 3.11959f, 5.99193f}, {-1.1728f, 2.81889f, 5.14292f}, 0.5f },
	{ {-1.07123f, 2.75494f, 4.95258f}, {-0.624744f, 2.46591f, 4.10575f}, 1.0f },
	{ {-0.487298f, 2.38576f, 3.85134f}, {-0.0320343f, 2.10677f, 3.00583f}, 1.5f },
	{ {0.150295f, 2.00345f, 2.6947f}, {0.621945f, 1.72613f, 1.85766f}, 2.0f },
	{ {0.79349f, 1.62656f, 1.56925f}, {1.27843f, 1.34087f, 0.74268f}, 2.5f },
	{ {1.4456f, 1.24481f, 0.480859f}, {1.94152f, 0.955777f, -0.337998f}, 3.0f },
	{ {1.99243f, 0.926875f, -0.41908f}, {2.49521f, 0.636172f, -1.23315f}, 3.5f },
	{ {2.54847f, 0.599657f, -1.30991f}, {3.05442f, 0.255997f, -2.10105f}, 4.0f },
	{ {1.92894f, 0.599657f, -2.08602f}, {2.78038f, 0.188142f, -2.41115f}, 5.0f },
	{ {2.18975f, 0.599657f, -3.16554f}, {2.93249f, 0.159718f, -2.66078f}, 6.0f },
	{ {3.46608f, 0.599657f, -3.82161f}, {3.35552f, 0.128645f, -2.94644f}, 7.0f },
	{ {4.53008f, 0.599657f, -3.14229f}, {3.69883f, 0.141007f, -2.82819f}, 8.0f },
	{ {4.37298f, 0.599657f, -1.89856f}, {3.64544f, 0.124032f, -2.39299f}, 9.0f },
	{ {3.1527f, 0.599657f, -1.44088f}, {3.25239f, 0.0801373f, -2.2895f}, 10.0f },
	{ {2.45099f, 0.599657f, -1.68903f}, {2.95764f, 0.114847f, -2.40196f}, 11.0f },
	{ {1.4456f, 1.24481f, 0.480859f}, {1.94152f, 0.955777f, -0.337998f}, 11.5f },
	{ {0.79349f, 1.62656f, 1.56925f}, {1.27843f, 1.34087f, 0.74268f}, 12.0f },
	{ {0.150295f, 2.00345f, 2.6947f}, {0.621945f, 1.72613f, 1.85766f}, 12.5f },
	{ {-0.487298f, 2.38576f, 3.85134f}, {-0.0320343f, 2.10677f, 3.00583f}, 13.0f },
	{ {-1.07123f, 2.75494f, 4.95258f}, {-0.624744f, 2.46591f, 4.10575f}, 13.5f },
	{ {-1.60726f, 3.11959f, 5.99193f}, {-1.1728f, 2.81889f, 5.14292f}, 14.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 14.5f },
	{ {-2.03047f, 3.42489f, 6.80471f}, {-2.11608f, 2.8715f, 5.9762f}, 15.0f },
};

std::vector<CameraKeyframe> pizakeyframes = {
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, -1.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 0.0f },
	{ {-1.65954f, 2.8355f, 5.93217f}, {-1.3071f, 2.30855f, 5.1588f}, 0.5f },
	{ {-1.20424f, 2.19291f, 4.99395f}, {-0.822876f, 1.66893f, 4.23237f}, 1.0f },
	{ {-0.692717f, 1.52066f, 4.00595f}, {-0.28472f, 1.01613f, 3.24503f}, 1.5f },
	{ {-0.178781f, 0.91859f, 3.07124f}, {0.252633f, 0.441432f, 2.3056f}, 2.0f },
	{ {-0.491943f, 0.91859f, 1.97638f}, {0.342185f, 0.381291f, 2.10104f}, 3.0f },
	{ {0.223909f, 0.91859f, 1.28021f}, {0.3236f, 0.399071f, 2.12884f}, 4.0f },
	{ {1.15335f, 0.91859f, 1.8689f}, {0.365399f, 0.388671f, 2.18246f}, 5.0f },
	{ {0.954136f, 1.0013f, 2.81789f}, {0.374449f, 0.434898f, 2.2321f}, 6.0f },
	{ {0.0709328f, 1.0013f, 3.05883f}, {0.287928f, 0.424872f, 2.27102f}, 7.0f },
	{ {-0.178781f, 0.91859f, 3.07124f}, {0.252633f, 0.441432f, 2.3056f}, 7.5f },
	{ {-0.692717f, 1.52066f, 4.00595f}, {-0.28472f, 1.01613f, 3.24503f}, 8.0f },
	{ {-1.20424f, 2.19291f, 4.99395f}, {-0.822876f, 1.66893f, 4.23237f}, 8.5f },
	{ {-1.65954f, 2.8355f, 5.93217f}, {-1.3071f, 2.30855f, 5.1588f}, 9.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 9.5f },
	{ {-2.03047f, 3.42489f, 6.80471f}, {-2.11608f, 2.8715f, 5.9762f}, 10.0f },
};

std::vector<CameraKeyframe> vaticankeyframes = {
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, -1.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 0.0f },
	{ {-1.60155f, 2.98264f, 6.30656f}, {-1.05701f, 2.42489f, 5.68014f}, 0.5f },
	{ {-0.960463f, 2.32728f, 5.57162f}, {-0.413742f, 1.76954f, 4.9471f}, 1.0f },
	{ {-0.23537f, 1.60635f, 4.7689f}, {0.328217f, 1.05733f, 4.1517f}, 1.5f },
	{ {0.618017f, 0.800497f, 3.83497f}, {1.18861f, 0.26762f, 3.2101f}, 2.0f },
	{ {1.44008f, 0.800497f, 3.88185f}, {1.24561f, 0.205674f, 3.10187f}, 3.0f },
	{ {2.10075f, 0.800497f, 3.43658f}, {1.35436f, 0.177982f, 3.20125f}, 4.0f },
	{ {2.05517f, 0.800497f, 2.56533f}, {1.51944f, 0.177982f, 3.13583f}, 5.0f },
	{ {1.0326f, 0.800497f, 2.35252f}, {1.34869f, 0.143121f, 3.03659f}, 6.0f },
	{ {0.573259f, 0.902368f, 3.49625f}, {1.18915f, 0.260918f, 3.03884f}, 7.0f },
	{ {0.618017f, 0.800497f, 3.83497f}, {1.18861f, 0.26762f, 3.2101f}, 7.5f },
	{ {-0.23537f, 1.60635f, 4.7689f}, {0.328217f, 1.05733f, 4.1517f}, 8.0f },
	{ {-0.960463f, 2.32728f, 5.57162f}, {-0.413742f, 1.76954f, 4.9471f}, 8.5f },
	{ {-1.60155f, 2.98264f, 6.30656f}, {-1.05701f, 2.42489f, 5.68014f}, 9.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 9.5f },
	{ {-2.03047f, 3.42489f, 6.80471f}, {-2.11608f, 2.8715f, 5.9762f}, 10.0f },
};

std::vector<CameraKeyframe> castlekeyframes = {
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, -1.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 0.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.64027f, 2.72023f, 6.42962f}, 0.5f },
	{ {-2.82115f, 2.5063f, 6.32209f}, {-3.42953f, 1.79673f, 5.96659f}, 1.0f },
	{ {-3.50423f, 1.70632f, 5.92329f}, {-4.10785f, 0.990624f, 5.57197f}, 1.5f },
	{ {-4.20944f, 0.856921f, 5.521f}, {-4.78825f, 0.15105f, 5.11269f}, 2.5f },
	{ {-4.97489f, 0.856921f, 5.77901f}, {-5.08211f, 0.168567f, 5.0616f}, 3.5f },
	{ {-5.88497f, 0.856921f, 5.61546f}, {-5.50335f, 0.161009f, 5.00712f}, 4.5f },
	{ {-6.416f, 0.856921f, 4.76485f}, {-5.69488f, 0.16604f, 4.71316f}, 5.5f },
	{ {-5.99672f, 0.856921f, 3.84346f}, {-5.51403f, 0.174924f, 4.39291f}, 6.5f },
	{ {-4.7982f, 0.856921f, 3.55644f}, {-5.0156f, 0.16604f, 4.24594f}, 7.5f },
	{ {-3.90399f, 0.856921f, 4.20845f}, {-4.59334f, 0.216813f, 4.54765f}, 8.5f },
	{ {-3.50423f, 1.70632f, 5.92329f}, {-4.10785f, 0.990624f, 5.57197f}, 9.5f },
	{ {-2.82115f, 2.5063f, 6.32209f}, {-3.42953f, 1.79673f, 5.96659f}, 10.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.64027f, 2.72023f, 6.42962f}, 10.5f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 11.0f },
	{ {-2.03047f, 3.42489f, 6.80471f}, {-2.11608f, 2.8715f, 5.9762f}, 11.5f },

};
std::vector<CameraKeyframe> eiffelkeyframes = {
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, -1.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 0.0f },
	{ {-1.84075f, 2.95599f, 5.8113f}, {-1.67596f, 2.54766f, 4.91347f}, 0.5f },
	{ {-1.60457f, 2.42327f, 4.59361f}, {-1.42828f, 2.04058f, 3.6867f}, 1.0f },
	{ {-1.36548f, 1.93443f, 3.41303f}, {-1.17973f, 1.57118f, 2.50004f}, 1.5f },
	{ {-1.09249f, 1.46125f, 2.12437f}, {-0.891283f, 1.15556f, 1.19375f}, 2.0f },
	{ {-0.783122f, 1.04367f, 0.717543f}, {-0.574354f, 0.779801f, -0.22415f}, 2.5f },
	{ {-1.54337f, 1.04367f, -0.0555946f}, {-0.735958f, 0.686936f, -0.525518f}, 3.5f },
	{ {-1.59096f, 1.04367f, -1.08652f}, {-0.741968f, 0.641726f, -0.743506f}, 4.5f },
	{ {-0.555683f, 1.04367f, -1.76243f}, {-0.488469f, 0.646526f, -0.84714f}, 5.5f },
	{ {0.490931f, 1.04367f, -1.11474f}, {-0.305064f, 0.649732f, -0.655168f}, 6.5f },
	{ {0.438523f, 1.04367f, 0.0221303f}, {-0.362334f, 0.641726f, -0.421793f}, 7.5f },
	{ {-0.735741f, 1.04367f, 0.608239f}, {-0.589305f, 0.691832f, -0.316295f}, 8.5f },
	{ {-0.783122f, 1.04367f, 0.717543f}, {-0.574354f, 0.779801f, -0.22415f}, 9.0f },
	{ {-1.09249f, 1.46125f, 2.12437f}, {-0.891283f, 1.15556f, 1.19375f}, 9.5f },
	{ {-1.36548f, 1.93443f, 3.41303f}, {-1.17973f, 1.57118f, 2.50004f}, 10.0f },
	{ {-1.60457f, 2.42327f, 4.59361f}, {-1.42828f, 2.04058f, 3.6867f}, 10.5f },
	{ {-1.84075f, 2.95599f, 5.8113f}, {-1.67596f, 2.54766f, 4.91347f}, 11.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 11.5f },
	{ {-2.03047f, 3.42489f, 6.80471f}, {-2.11608f, 2.8715f, 5.9762f}, 12.0f },
};
std::vector<CameraKeyframe> cathedralkeyframes = {
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, -1.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 0.0f },
	{ {-2.1073f, 2.88012f, 5.80876f}, {-2.17657f, 2.41065f, 4.92854f}, 0.5f },
	{ {-2.17421f, 2.36136f, 4.81279f}, {-2.2271f, 1.90893f, 3.92256f}, 1.0f },
	{ {-2.24133f, 1.78548f, 3.67723f}, {-2.2928f, 1.33773f, 2.78456f}, 1.5f },
	{ {-2.3112f, 1.1742f, 2.4476f}, {-2.35522f, 0.740538f, 1.5476f}, 2.0f },
	{ {-3.24786f, 0.908678f, 1.58439f}, {-2.74659f, 0.475019f, 0.835608f}, 3.0f },
	{ {-3.56689f, 0.908678f, 0.612635f}, {-2.76292f, 0.437667f, 0.249627f}, 4.0f },
	{ {-3.66747f, 0.908678f, -0.56052f}, {-2.80991f, 0.43459f, -0.360959f}, 5.0f },
	{ {-2.79196f, 0.908678f, -1.50048f}, {-2.45707f, 0.43152f, -0.687971f}, 6.0f },
	{ {-1.56106f, 0.908678f, -1.16447f}, {-2.16391f, 0.40114f, -0.54886f}, 7.0f },
	{ {-1.22579f, 0.908678f, 0.0836525f}, {-2.09327f, 0.414736f, 0.0245129f}, 8.0f },
	{ {-1.73125f, 0.908678f, 1.29786f}, {-2.19362f, 0.387669f, 0.580395f}, 9.0f },
	{ {-2.3112f, 1.1742f, 2.4476f}, {-2.35522f, 0.740538f, 1.5476f}, 9.5f },
	{ {-2.24133f, 1.78548f, 3.67723f}, {-2.2928f, 1.33773f, 2.78456f}, 10.0f },
	{ {-2.17421f, 2.36136f, 4.81279f}, {-2.2271f, 1.90893f, 3.92256f}, 10.5f },
	{ {-2.1073f, 2.88012f, 5.80876f}, {-2.17657f, 2.41065f, 4.92854f}, 11.0f },
	{ {-2.02047f, 3.41489f, 6.79471f}, {-2.10608f, 2.8615f, 5.9662f}, 11.5f },
	{ {-2.03047f, 3.42489f, 6.80471f}, {-2.11608f, 2.8715f, 5.9762f}, 12.0f },
};


GLuint shadowMapFBO;
GLuint shadowMap;
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
GLuint programShadowMap;
glm::mat4 lightProjection, lightView, lightSpaceMatrix;


glm::vec3 actualLightPosition = glm::vec3(7.5f, 8.0f, 3.0f);
glm::vec3 sunDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 sunPosition = glm::vec3(7.5f, 8.0f, 3.0f);


float shadowDistance = 15.0f;
float shadowNearPlane = 0.1f;
float shadowFarPlane = 25.0f;



glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
	return 0.5f * (
		(2.0f * p1) +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t
		);
}

void updateCameraFromKeyframes(float currentTime) {
	if (keyframes.size() < 4) return;

	int segment = -1;
	for (int i = 0; i < keyframes.size() - 3; ++i) {
		if (currentTime < keyframes[i + 2].time) {
			segment = i;
			break;
		}
	}
	if (segment == -1) return;

	float t0 = keyframes[segment + 1].time;
	float t1 = keyframes[segment + 2].time;
	float t = (currentTime - t0) / (t1 - t0);

	glm::vec3 p0 = keyframes[segment].position;
	glm::vec3 p1 = keyframes[segment + 1].position;
	glm::vec3 p2 = keyframes[segment + 2].position;
	glm::vec3 p3 = keyframes[segment + 3].position;

	glm::vec3 camPos = catmullRom(p0, p1, p2, p3, t);

	glm::vec3 t0t = keyframes[segment].target;
	glm::vec3 t1t = keyframes[segment + 1].target;
	glm::vec3 t2t = keyframes[segment + 2].target;
	glm::vec3 t3t = keyframes[segment + 3].target;
	glm::vec3 camTarget = catmullRom(t0t, t1t, t2t, t3t, t);

	cameraPos = camPos;
	cameraFront = glm::normalize(camTarget - camPos);
}



glm::mat4 createCameraMatrix()
{
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 createPerspectiveMatrix()
{
	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 80.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
		0.,0.,-1.,0.,
		});

	perspectiveMatrix = glm::transpose(perspectiveMatrix);
	return perspectiveMatrix;
}


void setupShadowMapping() {
	//std::cout << "Setting up shadow mapping..." << std::endl;

	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//std::cout << "Shadow mapping setup complete. Shadow map ID: " << shadowMap << std::endl;
}

void calculateLightSpaceMatrix() {
	glm::vec3 lightPosition = actualLightPosition;
	glm::vec3 lookAtTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

	float orthoSize = 20.0f;
	lightProjection = glm::ortho(-orthoSize, orthoSize,
		-orthoSize, orthoSize,
		0.1f, 30.0f);

	lightView = glm::lookAt(lightPosition, lookAtTarget, upVector);
	lightSpaceMatrix = lightProjection * lightView;

	sunPosition = lightPosition;
}

void renderShadowMap() {
	calculateLightSpaceMatrix();

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(programShadowMap);

	GLint lightSpaceLocation = glGetUniformLocation(programShadowMap, "lightSpaceMatrix");
	GLint modelLocation = glGetUniformLocation(programShadowMap, "model");

	glUniformMatrix4fv(lightSpaceLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));


	glm::mat4 towerModel = glm::translate(glm::vec3(-0.5f, -0.45f, -0.6f)) * glm::scale(glm::vec3(0.0045f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(towerModel));
	Core::DrawContext(towerContext);


	glm::mat4 castleModel = glm::translate(glm::vec3(-5.2f, -0.015f, 4.65f)) * glm::scale(glm::vec3(0.003f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(castleModel));
	Core::DrawContext(castleContext);


	glm::mat4 katedraModel = glm::translate(glm::vec3(-2.4f, -0.15f, 0.0f)) * glm::scale(glm::vec3(0.1f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(katedraModel));
	Core::DrawContext(katedraContext);


	glm::mat4 bigbenModel = glm::translate(glm::vec3(-3.9f, -0.15f, -2.1f)) *
		glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::scale(glm::vec3(0.15f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(bigbenModel));
	Core::DrawContext(bigbenContext);


	glm::mat4 watykanModel = glm::translate(glm::vec3(1.38f, -0.53f, 3.0f)) * glm::scale(glm::vec3(0.05f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(watykanModel));
	Core::DrawContext(watykanContext);


	glm::mat4 pizaModel = glm::translate(glm::vec3(0.30f, 0.28f, 1.8f)) * glm::scale(glm::vec3(0.075f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(pizaModel));
	Core::DrawContext(pizaContext);


	glm::mat4 inowroclawModel = glm::translate(glm::vec3(3.3f, -0.141f, -2.7f)) * glm::scale(glm::vec3(0.0225f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(inowroclawModel));
	Core::DrawContext(inowroclawContext);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	int width, height;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
	glViewport(0, 0, width, height);
}



void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {
	GLuint prog = program;
	glUseProgram(prog);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(prog, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(prog, "lightPos"), actualLightPosition.x, actualLightPosition.y, actualLightPosition.z);
	Core::DrawContext(context);
	glUseProgram(0);
}


void drawObjectTextureWithShadows(Core::RenderContext& context, glm::mat4 modelMatrix,
	GLuint textureID, GLuint textureID2, float a, float b, bool receiveShadows = true) {
	GLuint prog = programTex;
	glUseProgram(prog);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;


	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(prog, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(prog, "lightPos"), sunPosition.x, sunPosition.y, sunPosition.z);
	glUniform1i(glGetUniformLocation(prog, "receiveShadows"), receiveShadows ? 1 : 0);


	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);
	Core::SetActiveTexture(textureID2, "normalMap", prog, 1);
	Core::SetActiveTexture(shadowMap, "shadowMap", prog, 2);

	glUniform1f(glGetUniformLocation(prog, "metallic"), a);
	glUniform1f(glGetUniformLocation(prog, "roughness"), b);

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint textureID2, float a, float b) {
	drawObjectTextureWithShadows(context, modelMatrix, textureID, textureID2, a, b, false);
}

void drawSkybox() {
	glDepthFunc(GL_LEQUAL);
	glUseProgram(programSun);

	glm::mat4 view = glm::mat4(glm::mat3(createCameraMatrix()));
	glm::mat4 projection = createPerspectiveMatrix();

	glUniformMatrix4fv(glGetUniformLocation(programSun, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
}


void drawSun() {
	glm::vec3 sunPos = actualLightPosition;

	glm::mat4 sunModel =
		glm::translate(sunPos) *
		glm::scale(glm::vec3(0.5f));

	GLuint prog = program;
	glUseProgram(prog);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * sunModel;

	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&sunModel);
	glUniform3f(glGetUniformLocation(prog, "color"), 1.0f, 1.0f, 0.0f);  // ŻÓŁTY KOLOR
	glUniform3f(glGetUniformLocation(prog, "lightPos"), sunPos.x, sunPos.y, sunPos.z);

	Core::DrawContext(sunSphereContext);
	glUseProgram(0);
}


void renderScene(GLFWwindow* window)
{

	float time = glfwGetTime();
	if (animationStarted && !animationFinished) {
		float currentTime = glfwGetTime() - animationStartTime;

		float totalDuration = keyframes.back().time;

		if (currentTime >= totalDuration) {
			currentTime = totalDuration;
			animationStarted = false;
			animationFinished = false;
			animationStartTime = 0.0f;
		}
		updateCameraFromKeyframes(currentTime);
	}


	renderShadowMap();


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSkybox();


	glm::mat4 europeModel = glm::translate(europeStartPos) * glm::scale(glm::vec3(2.0f) * 3.30f);
	drawObjectTextureWithShadows(europeContext, europeModel, texture::europe, texture::europeNormal,
		roughness, metallic, true);  


	glm::mat4 towerModel = glm::translate(glm::vec3(-0.5f, -0.45f, -0.6f)) * glm::scale(glm::vec3(0.0045f));
	drawObjectTextureWithShadows(towerContext, towerModel, texture::tower, texture::towerNormal,
		1.0f, 0.0f, true);


	glm::mat4 castleModel = glm::translate(glm::vec3(-5.2f, -0.015f, 4.65f)) * glm::scale(glm::vec3(0.003f));
	drawObjectTextureWithShadows(castleContext, castleModel, texture::castle, texture::castleNormal,
		1.0f, 0.0f, true);


	glm::mat4 katedraModel = glm::translate(glm::vec3(-2.4f, -0.15f, 0.0f)) * glm::scale(glm::vec3(0.1f));
	drawObjectTextureWithShadows(katedraContext, katedraModel, texture::katedra, texture::katedraNormal,
		1.0f, 0.0f, true);


	glm::mat4 bigbenModel = glm::translate(glm::vec3(-3.9f, -0.15f, -2.1f)) *
		glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::scale(glm::vec3(0.15f));
	drawObjectTextureWithShadows(bigbenContext, bigbenModel, texture::bigben, texture::bigben,
		1.0f, 0.0f, true);


	glm::mat4 watykanModel = glm::translate(glm::vec3(1.38f, -0.53f, 3.0f)) * glm::scale(glm::vec3(0.05f));
	drawObjectTextureWithShadows(watykanContext, watykanModel, texture::watykan, texture::watykanNormal,
		1.0f, 0.0f, true);


	glm::mat4 pizaModel = glm::translate(glm::vec3(0.30f, 0.28f, 1.8f)) * glm::scale(glm::vec3(0.075f));
	drawObjectTextureWithShadows(pizaContext, pizaModel, texture::piza, texture::piza,
		1.0f, 0.0f, true);


	glm::mat4 inowroclawModel = glm::translate(glm::vec3(3.3f, -0.141f, -2.7f)) * glm::scale(glm::vec3(0.0225f));
	drawObjectTextureWithShadows(inowroclawContext, inowroclawModel, texture::inowroclaw, texture::inowroclaw,
		1.0f, 0.0f, true);


	glUseProgram(0);
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}

void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float sensitivity = 0.1f;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}



void init(GLFWwindow* window)
{
	keyframes = benkeyframes;
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);


	program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
	programShadowMap = shaderLoader.CreateProgram("shaders/shadow_map.vert", "shaders/shadow_map.frag");


	GLint lightSpaceLoc = glGetUniformLocation(programTex, "lightSpaceMatrix");
	GLint shadowMapLoc = glGetUniformLocation(programTex, "shadowMap");
	GLint receiveShadowsLoc = glGetUniformLocation(programTex, "receiveShadows");


	loadModelToContext("./models/eurobj.obj", europeContext);
	loadModelToContext("./models/tower.obj", towerContext);
	loadModelToContext("./models/castle.obj", castleContext);
	loadModelToContext("./models/katedra2.obj", katedraContext);
	loadModelToContext("./models/bigben.obj", bigbenContext);
	loadModelToContext("./models/watykan.obj", watykanContext);
	loadModelToContext("./models/piza.obj", pizaContext);
	loadModelToContext("./models/inowroclaw2.obj", inowroclawContext);
	loadModelToContext("./models/sphere.obj", sunSphereContext);  


	texture::tower = Core::LoadTexture("textures/tower.png");
	texture::towerNormal = Core::LoadTexture("textures/towerNormal.png");
	texture::castle = Core::LoadTexture("textures/castle.png");
	texture::castleNormal = Core::LoadTexture("textures/castleNormal.png");
	texture::europe = Core::LoadTexture("textures/europe.png");
	texture::europeNormal = Core::LoadTexture("textures/europeNormal.png");
	texture::katedra = Core::LoadTexture("textures/katedra.jpeg");
	texture::katedraNormal = Core::LoadTexture("textures/katedraNormal2.png");
	texture::bigben = Core::LoadTexture("textures/bigben.jpeg");
	texture::bigbenNormal = texture::bigben;
	texture::watykan = Core::LoadTexture("textures/watykan.jpeg");
	texture::watykanNormal = texture::watykan;
	texture::piza = Core::LoadTexture("textures/piza.jpeg");
	texture::inowroclaw = Core::LoadTexture("textures/inowroclaw.jpeg");


	const char* faces[6] = {
	"textures/skybox/_px.jpg",
	"textures/skybox/_nx.jpg",
	"textures/skybox/_py.jpg",
	"textures/skybox/_ny.jpg",
	"textures/skybox/_pz.jpg",
	"textures/skybox/_nz.jpg"
	};

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);

	skyboxCubemap = Core::LoadCubemap(faces);
	programSun = shaderLoader.CreateProgram("shaders/skybox.vert", "shaders/skybox.frag");


	setupShadowMapping();
	calculateLightSpaceMatrix();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programShadowMap);


	glDeleteFramebuffers(1, &shadowMapFBO);
	glDeleteTextures(1, &shadowMap);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !animationStarted && !animationFinished) {
		animationStarted = true;
		animationStartTime = glfwGetTime();
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
		animationStarted = false;
		animationFinished = false;
		animationStartTime = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		cameraPos = defaultCameraPos;
		cameraFront = glm::normalize(defaultCameraTarget - defaultCameraPos);
		yaw = -90.0f;
		pitch = 0.0f;
		firstMouse = true;
	}


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !isRecording) {
		recordedKeyframes.clear();
		recordingStartTime = glfwGetTime();
		isRecording = true;
	}
	if (isRecording && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		float currentTime = glfwGetTime() - recordingStartTime;
		glm::vec3 target = cameraPos + cameraFront;
		recordedKeyframes.push_back({ cameraPos, target, currentTime });
		std::cout << "Keyframe added at t = " << currentTime << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !recordedKeyframes.empty()) {
		std::cout << "Recorded Keyframes:\n";
		for (const auto& kf : recordedKeyframes) {
			std::cout << "{ {"
				<< kf.position.x << "f, " << kf.position.y << "f, " << kf.position.z << "f}, {"
				<< kf.target.x << "f, " << kf.target.y << "f, " << kf.target.z << "f}, "
				<< kf.time << "f },\n";
		}
	}


	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		actualLightPosition.x -= 0.2f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		actualLightPosition.x += 0.2f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		actualLightPosition.y += 0.2f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		actualLightPosition.y -= 0.2f;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		keyframes = castlekeyframes;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		keyframes = benkeyframes;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		keyframes = cathedralkeyframes;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		keyframes = eiffelkeyframes;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		keyframes = pizakeyframes;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		keyframes = wroclawkeyframes;
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
		keyframes = vaticankeyframes;
}



void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		ImGui::SetNextWindowSize(ImVec2(200, 350), ImGuiCond_Always);
		ImGui::Begin("Camera & Shadows");
		ImGui::PushItemWidth(-1);

		ImGui::Separator();


		ImGui::Text("Shadow Controls:");
		ImGui::SliderFloat("Light X", &actualLightPosition.x, -15.0f, 15.0f);
		ImGui::SliderFloat("Light Y", &actualLightPosition.y, 3.0f, 20.0f);
		ImGui::SliderFloat("Light Z", &actualLightPosition.z, -15.0f, 15.0f);

		if (ImGui::Button("Reset Light")) {
			actualLightPosition = glm::vec3(7.5f, 8.0f, 3.0f);
		}

		ImGui::Separator();
		ImGui::Text("Controls:");
		ImGui::Text("SPACE - Start animation");
		ImGui::Text("Arrows - Move light");
		ImGui::Separator();
		ImGui::Text("Budynki (klawisze 1-7):");
		ImGui::BulletText("1 - Zamek Szynszyli");
		ImGui::BulletText("2 - Big Ben");
		ImGui::BulletText("3 - Katedra Notre Dame");
		ImGui::BulletText("4 - Wieza Eiffel'a");
		ImGui::BulletText("5 - Krzywa wieza w Pizie");
		ImGui::BulletText("6 - Zamek w Inowroclawiu");
		ImGui::BulletText("7 - Plac sw. Piotra");

		ImGui::PopItemWidth();
		ImGui::End();

		renderScene(window);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}