#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

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




glm::vec3 cameraPos = glm::vec3(0.f, 1.f, 5.f);
glm::vec3 cameraFront = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

glm::vec3 europeStartPos = glm::vec3(0.0f, 0.0f, 0.0f);
GLuint VAO, VBO;
GLuint skyboxVAO, skyboxVBO;

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
void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {
	GLuint prog = program;
	glUseProgram(prog);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(prog, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(prog, "lightPos"), -5, 3, 3);
	Core::DrawContext(context);
	glUseProgram(0);
}
void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint textureID2, float a, float b) {
	GLuint prog = programTex;
	glUseProgram(prog);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(prog, "lightPos"), 0.0f, 5.0f, 0.0f);
	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);


	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);
	Core::SetActiveTexture(textureID2, "normalMap", prog, 1);

	glUniform1f(glGetUniformLocation(prog, "metallic"), metallic);
	glUniform1f(glGetUniformLocation(prog, "roughness"), roughness);

	Core::DrawContext(context);
	glUseProgram(0);
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



void renderScene(GLFWwindow* window)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();

	drawSkybox();

	glm::mat4 europeModel =
		glm::translate(europeStartPos) *
		glm::scale(glm::vec3(2.0f) * 1.10f);
	drawObjectTexture(
		europeContext,
		europeModel,
		texture::europe,
		texture::europeNormal,
		roughness,
		metallic
	);

	glm::mat4 towerModel =
		glm::translate(glm::vec3(-0.5f, -0.105f, -0.2f)) *//zachód, dół, północ
		glm::scale(glm::vec3(0.0009f));

	drawObjectTexture(
		towerContext,
		towerModel,
		texture::tower,
		texture::towerNormal,
		1.0f,
		0.0f
	);

	glm::mat4 castleModel =
		glm::translate(glm::vec3(-1.6f, -0.005f, 1.55f)) * //zachód, dół, południe
		glm::scale(glm::vec3(0.0006f)); // skalowanie

	drawObjectTexture(
		castleContext,
		castleModel,
		texture::castle,
		texture::castleNormal,
		1.0f,
		0.0f
	);

	glm::mat4 katedraModel =
		glm::translate(glm::vec3(-0.8f, -0.05f, 0.0f)) * //zachód, dół, północ
		glm::scale(glm::vec3(0.02f));                   // skalowanie podobne do zamku

	drawObjectTexture(
		katedraContext,
		katedraModel,
		texture::katedra,
		texture::katedraNormal,
		1.0f,   // roughness, możesz zmienić
		0.0f    // metallic, możesz zmienić
	);

	glm::mat4 bigbenModel =
		glm::translate(glm::vec3(-1.3f, -0.03f, -0.7f)) * //zachód, dół, północ
		glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * // nowa rotacja o 90 stopni wokół Y
		glm::scale(glm::vec3(0.03f)); // skalowanie podobne do katedry

	drawObjectTexture(
		bigbenContext,
		bigbenModel,
		texture::bigben,
		texture::bigben,
		1.0f,   // roughness, możesz zmienić
		0.0f    // metallic, możesz zmienić
	);



	glm::mat4 watykanModel =
		glm::translate(glm::vec3(0.46f, -0.11f, 1.0f)) *  //wschod, dół, południe
		glm::scale(glm::vec3(0.01f));                    // przykład skali, dopasuj do modelu

	drawObjectTexture(
		watykanContext,
		watykanModel,
		texture::watykan,
		texture::watykanNormal,
		1.0f,  // roughness
		0.0f   // metallic
	);

	glm::mat4 pizaModel =
		glm::translate(glm::vec3(0.10f, 0.045f, 0.6f)) *//wschod, gora, południe
		glm::scale(glm::vec3(0.015f));
	drawObjectTexture(
		pizaContext,
		pizaModel,
		texture::piza,
		texture::piza,   // brak normal map, więc dajemy tę samą teksturę lub możesz dodać 0
		1.0f,
		0.0f
	);



	glm::mat4 inowroclawModel =
		glm::translate(glm::vec3(1.1f, -0.047f, -0.9f)) *//wschod, dół, północ
		glm::scale(glm::vec3(0.0045f));

	drawObjectTexture(
		inowroclawContext,
		inowroclawModel,
		texture::inowroclaw,
		texture::inowroclaw, // brak normal mapy
		1.0f,
		0.0f
	);


	glUseProgram(0);
	//glfwSwapBuffers(window);
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
	float yoffset = lastY - ypos; // odwrotnie, bo góra to mniejsze Y

	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// ograniczenie kąta pitch
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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
	loadModelToContext("./models/eurobj.obj", europeContext);
	loadModelToContext("./models/tower.obj", towerContext);
	texture::tower = Core::LoadTexture("textures/tower.png");
	texture::towerNormal = Core::LoadTexture("textures/towerNormal.png");

	loadModelToContext("./models/castle.obj", castleContext);
	texture::castle = Core::LoadTexture("textures/castle.png");
	texture::castleNormal = Core::LoadTexture("textures/castleNormal.png");


	texture::europe = Core::LoadTexture("textures/europe.png");
	texture::europeNormal = Core::LoadTexture("textures/europeNormal.png");

	loadModelToContext("./models/katedra2.obj", katedraContext);
	texture::katedra = Core::LoadTexture("textures/katedra.jpeg");
	texture::katedraNormal = Core::LoadTexture("textures/katedraNormal2.png");

	loadModelToContext("./models/bigben.obj", bigbenContext);
	texture::bigben = Core::LoadTexture("textures/bigben.jpeg");
	texture::bigbenNormal = texture::bigben;




	loadModelToContext("./models/watykan.obj", watykanContext);
	texture::watykan = Core::LoadTexture("textures/watykan.jpeg");
	texture::watykanNormal = texture::watykan;

	loadModelToContext("./models/piza.obj", pizaContext);
	texture::piza = Core::LoadTexture("textures/piza.jpeg");


	loadModelToContext("./models/inowroclaw2.obj", inowroclawContext);
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



	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	float cameraSpeed = 0.025f;

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
}


// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// UI Panel

		ImGui::SetNextWindowSize(ImVec2(120, 200), ImGuiCond_Always);
		ImGui::Begin("Asteroid Controls");
		ImGui::PushItemWidth(-1);
		ImGui::PopItemWidth();
		ImGui::End();
		// Render
		renderScene(window);

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}