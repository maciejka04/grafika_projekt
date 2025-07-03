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


glm::vec3 cameraPos = glm::vec3(-0.714328f, 1.06419f, 2.45808f);
glm::vec3 cameraFront = glm::vec3(-0.718981f, 0.60554f, 1.56947f);
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
{ {-0.714328f, 1.06419f, 2.45808f}, {-0.718981f, 0.60554f, 1.56947f}, 0.0f },
{ {-0.825346f, 0.856438f, 1.90625f}, {-1.01005f, 0.52263f, 0.981878f}, 1.58352f },
{ {-0.963878f, 0.620125f, 1.20809f}, {-1.14718f, 0.314429f, 0.273776f}, 2.3496f },
{ {-1.11773f, 0.363306f, 0.385935f}, {-1.29143f, 0.0910251f, -0.560475f}, 3.14996f },
{ {-1.22928f, 0.192559f, -0.23118f}, {-1.39639f, -0.0425829f, -1.18867f}, 4.96614f },
{ {-1.27193f, 0.125515f, -0.469317f}, {-1.41218f, 0.0296689f, -1.45478f}, 5.63265f },
{ {-1.08985f, 0.125515f, -0.704257f}, {-2.08503f, 0.0314063f, -0.676457f}, 7.36576f },
{ {-1.30747f, 0.125515f, -0.917133f}, {-1.28666f, 0.0123114f, 0.0762215f}, 9.5318f },
{ {-1.5239f, 0.133361f, -0.713076f}, {-0.526847f, 0.0566415f, -0.714816f}, 11.715f },
{ {-1.11773f, 0.363306f, 0.385935f}, {-1.29143f, 0.0910251f, -0.560475f}, 13.54996f },
{ {-0.963878f, 0.620125f, 1.20809f}, {-1.14718f, 0.314429f, 0.273776f}, 15.3496f },
{ {-0.825346f, 0.856438f, 1.90625f}, {-1.01005f, 0.52263f, 0.981878f}, 17.58352f },
{ {-0.714328f, 1.06419f, 2.45808f}, {-0.718981f, 0.60554f, 1.56947f}, 19.366498f },
{ {-0.714327f, 1.06418f, 2.45807f}, {-0.718982f, 0.60553f, 1.56946f}, 20.366498f }
};

std::vector<CameraKeyframe> russiakeyframes = {
	{ {-0.714328f, 1.06419f, 2.45808f}, {-0.10623f, 0.608152f, 1.135f}, 0.0f },
{ {-0.504987f, 0.907194f, 2.00294f}, {-0.10624f, 0.608153f, 1.136f}, 1.41592f },
{ {-0.0276208f, 0.589215f, 1.03542f}, {0.416268f, 0.325343f, 0.179072f}, 2.16638f },
{ {0.961554f, 0.0791763f, -0.655828f}, {1.42541f, -0.264483f, -1.47237f}, 5.31554f },
{ {1.39374f, 0.157408f, -0.661048f}, {0.693888f, -0.324346f, -1.18842f}, 8.0819f },
{ {1.4207f, 0.157408f, -1.03037f}, {0.638043f, -0.345612f, -0.663748f}, 9.4486f },
{ {0.994009f, 0.157408f, -1.19847f}, {1.32535f, -0.336534f, -0.394585f}, 10.8819f },
{ {0.852939f, 0.157408f, -0.630852f}, {1.43483f, -0.290351f, -1.30976f}, 13.348f },
{ {0.961554f, 0.0791763f, -0.655828f}, {1.42541f, -0.264483f, -1.47237f}, 15.31554f },
{ {-0.0276208f, 0.589215f, 1.03542f}, {0.416268f, 0.325343f, 0.179072f}, 17.16638f },
{ {-0.504987f, 0.907194f, 2.00294f}, {-0.10624f, 0.608153f, 1.136f}, 19.41592f },
{ {-0.714328f, 1.06419f, 2.45808f}, {-0.718981f, 0.60554f, 1.56947f}, 20.366498f },
{ {-0.714327f, 1.06418f, 2.45807f}, {-0.718982f, 0.60553f, 1.56946f}, 21.366498f }
};

std::vector<CameraKeyframe> pizakeyframes = {
{ {-0.714328f, 1.06419f, 2.45808f}, {-0.20692f, 0.422054f, 1.21993f}, 0.0f },
{ {-0.304999f, 0.595347f, 1.56813f}, {0.0342332f, 0.139802f, 0.745089f}, 1.09992f },
{ {-0.0974301f, 0.320895f, 1.07663f}, {0.299555f, -0.153194f, 0.290731f}, 2.13353f },
{ {0.0112046f, 0.188336f, 0.861572f}, {0.375992f, -0.293418f, 0.0648013f}, 3.23302f },
{ {0.308994f, 0.188336f, 0.836044f}, {-0.423352f, -0.239022f, 0.305916f}, 4.63299f },
{ {0.240426f, 0.188336f, 0.526414f}, {-0.312238f, -0.293417f, 1.20647f}, 5.79962f },
{ {-0.162737f, 0.188336f, 0.588024f}, {0.72598f, -0.219994f, 0.796468f}, 7.11601f },
{ {-0.000627615f, 0.188336f, 0.833347f}, {0.417335f, -0.304087f, 0.0699162f}, 9.03274f },
{ {0.0112046f, 0.188336f, 0.861572f}, {0.375992f, -0.293418f, 0.0648013f}, 10.23302f },
{ {-0.0974301f, 0.320895f, 1.07663f}, {0.299555f, -0.153194f, 0.290731f}, 11.13353f },
{ {-0.304999f, 0.595347f, 1.56813f}, {0.0342332f, 0.139802f, 0.745089f}, 12.09992f },
{ {-0.501488f, 0.866689f, 2.06582f}, {-0.20692f, 0.422054f, 1.21993f}, 13.0f },
{ {-0.714328f, 1.06419f, 2.45808f}, {-0.718981f, 0.60554f, 1.56947f}, 14.366498f },
{ {-0.714327f, 1.06418f, 2.45807f}, {-0.718982f, 0.60553f, 1.56946f}, 15.366498f }
};

std::vector<CameraKeyframe> vaticankeyframes = {
{ {-0.714328f, 1.06419f, 2.45808f}, {0.0734017f, 0.283220f, 1.41108f}, -1.0f },
{ {-0.714328f, 1.06419f, 2.45808f}, {0.0734017f, 0.283220f, 1.41108f}, 0.0f },
{ {-0.445925f, 0.795264f, 2.09528f}, {0.0734018f, 0.283221f, 1.41109f}, 1.58358f },
{ {-0.0610445f, 0.425984f, 1.60424f}, {0.484163f, -0.0860584f, 0.940496f}, 2.9665f },
{ {0.172143f, 0.208366f, 1.32337f}, {0.720819f, -0.303676f, 0.662484f}, 4.59953f },
{ {0.284342f, 0.11019f, 1.19007f}, {0.845987f, -0.290158f, 0.465999f}, 5.58316f },
{ {0.651384f, 0.11019f, 1.19022f}, {-0.0993853f, -0.220324f, 0.618292f}, 6.61596f },
{ {0.541001f, 0.11019f, 0.829847f}, {0.252161f, -0.314009f, 1.68812f}, 8.48249f },
{ {0.227399f, 0.11019f, 0.86614f}, {1.03164f, -0.26765f, 1.32487f}, 9.63241f },
{ {0.274093f, 0.11019f, 1.15853f}, {0.921194f, -0.296546f, 0.513688f}, 10.94892f },
{ {0.172143f, 0.208366f, 1.32337f}, {0.720819f, -0.303676f, 0.662484f}, 12.59953f },
{ {-0.0610445f, 0.425984f, 1.60424f}, {0.484163f, -0.0860584f, 0.940496f}, 13.9665f },
{ {-0.714328f, 1.06419f, 2.45808f}, {-0.718981f, 0.60554f, 1.56947f}, 15.366498f },
{ {-0.714327f, 1.06418f, 2.45807f}, {-0.718982f, 0.60553f, 1.56946f}, 16.366498f }
};

std::vector<CameraKeyframe> castlekeyframes = {
{ {-0.897305f, 0.824506f, 2.28041f}, {-1.41461f, 0.138688f, 1.76849f}, -1.0f },
{ {-0.897305f, 0.824506f, 2.28041f}, {-1.41461f, 0.138688f, 1.76849f}, 0.0f },
{ {-1.13009f, 0.515887f, 2.05004f}, {-1.64739f, -0.16993f, 1.53813f}, 1.68364f },
{ {-1.31115f, 0.275851f, 1.87087f}, {-1.83232f, -0.397161f, 1.34605f}, 2.56633f },
{ {-1.73595f, 0.0840833f, 1.82388f}, {-1.32267f, -0.267758f, 0.983998f}, 4.63327f },
{ {-1.96197f, 0.0840833f, 1.57459f}, {-1.01807f, -0.239834f, 1.51024f}, 5.7011f },
{ {-1.62327f, 0.0458715f, 1.27449f}, {-1.5172f, -0.261485f, 2.22016f}, 7.49926f },
{ {-1.32757f, 0.0760668f, 1.36282f}, {-2.05948f, -0.227966f, 1.97262f}, 9.3661f },
{ {-1.31115f, 0.275851f, 1.87087f}, {-1.83232f, -0.397161f, 1.34605f}, 10.56633f },
{ {-1.13009f, 0.515887f, 2.05004f}, {-1.64739f, -0.16993f, 1.53813f}, 11.68364f },
{ {-0.714328f, 1.06419f, 2.45808f}, {-0.718981f, 0.60554f, 1.56947f}, 12.866498f },
{ {-0.714327f, 1.06418f, 2.45807f}, {-0.718982f, 0.60553f, 1.56946f}, 13.866498f }
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

CameraKeyframe createBridgeKeyframe(CameraKeyframe a, CameraKeyframe b, float offset)
{
	CameraKeyframe bridge;
	bridge.position = 0.5f * (a.position + b.position);
	bridge.time = 0.5f * (a.time + offset);
	bridge.target = b.target;
	return bridge;
}

std::vector<CameraKeyframe> concatenatePaths(const std::vector<CameraKeyframe>& first, const std::vector<CameraKeyframe>& second)
{
	const float offset = first.back().time + 6.0f;
	std::vector<CameraKeyframe> out;
	if (out.empty())
	{
		CameraKeyframe temp;
		temp.position = first.front().position;
		temp.target = first.front().target;
		temp.time = 0.0f;
		temp.position.x += 0.001f;
		temp.position.y += 0.001f;
		temp.position.z += 0.001f;
		temp.target.x += 0.001f;
		temp.target.y += 0.001f;
		temp.target.z += 0.001f;
		out.push_back(temp);
	}
	if (first.empty() || second.empty()) return out;
	out.insert(out.end(), first.begin(), first.end());
	CameraKeyframe bridgeframe = createBridgeKeyframe(first.back(), second.front(), offset);
	out.push_back(bridgeframe);
	const float startSecond = second.front().time;
	for (auto k : second)
	{
		k.time = offset + (k.time - startSecond);
		out.push_back(k);
	}
	return out;
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
			animationFinished = true;
			std::cout << "Animation finished!\n";
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
	keyframes = castlekeyframes;
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);

	//std::cout << "=== CAMERA ANIMATION + SHADOW MAPPING PROJECT ===" << std::endl;


	program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
	programShadowMap = shaderLoader.CreateProgram("shaders/shadow_map.vert", "shaders/shadow_map.frag");


	GLint lightSpaceLoc = glGetUniformLocation(programTex, "lightSpaceMatrix");
	GLint shadowMapLoc = glGetUniformLocation(programTex, "shadowMap");
	GLint receiveShadowsLoc = glGetUniformLocation(programTex, "receiveShadows");

	//std::cout << "Shadow uniforms - lightSpace: " << lightSpaceLoc << ", shadowMap: " << shadowMapLoc << ", receiveShadows: " << receiveShadowsLoc << std::endl;


	loadModelToContext("./models/eurobj.obj", europeContext);
	loadModelToContext("./models/tower.obj", towerContext);
	loadModelToContext("./models/castle.obj", castleContext);
	loadModelToContext("./models/katedra2.obj", katedraContext);
	loadModelToContext("./models/bigben.obj", bigbenContext);
	loadModelToContext("./models/watykan.obj", watykanContext);
	loadModelToContext("./models/piza.obj", pizaContext);
	loadModelToContext("./models/inowroclaw2.obj", inowroclawContext);
	loadModelToContext("./models/sphere.obj", sunSphereContext);  // DODANE dla słońca


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
	//std::cout << "Shadow mapping initialized" << std::endl;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	//std::cout << "=== INITIALIZATION COMPLETE ===" << std::endl;
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
}



void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_Always);
		ImGui::Begin("Camera & Shadows");
		ImGui::PushItemWidth(-1);


		ImGui::Text("Camera Animation:");
		if (ImGui::Button("Start Animation (SPACE)")) {
			if (!animationStarted && !animationFinished) {
				animationStarted = true;
				animationStartTime = glfwGetTime();
			}
		}
		if (ImGui::Button("Reset Animation (N)")) {
			animationStarted = false;
			animationFinished = false;
			animationStartTime = 0.0f;
		}

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
		ImGui::Text("N - Reset animation");
		ImGui::Text("R - Start recording");
		ImGui::Text("K - Add keyframe");
		ImGui::Text("L - Print keyframes");
		ImGui::Text("Arrows - Move light");

		ImGui::PopItemWidth();
		ImGui::End();

		renderScene(window);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}