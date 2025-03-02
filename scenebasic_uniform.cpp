#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
#include <sstream>
using std::cerr;
using std::endl;

#include <GLFW/glfw3.h>
#include "helper/glutils.h"
#include "helper/texture.h"

#include "camControls.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;


SceneBasic_Uniform::SceneBasic_Uniform()
	: 
	platform(500.0f, 500.0f, 1, 1),
	tPrev(0),
	angle(0),
	skybox(100.0f)
{
	car = ObjMesh::load("media/model/car.obj", true);
}

void SceneBasic_Uniform::initScene()
{
	compile();
	glEnable(GL_DEPTH_TEST);
	model = mat4(1.0f);
	view = CamControls::getViewMatrix();
	projection = mat4(1.0f);

	// light settings

	prog.setUniform("Spotlight.L", vec3(1.5f));
	prog.setUniform("Spotlight.La", vec3(0.5f));
	prog.setUniform("Spotlight.Exponent", 20.f);
	prog.setUniform("Spotlight.Cutoff", glm::radians(15.0f));

	// texture

	GLuint cubeTex = Texture::loadCubeMap("media/texture/yokohama/yokohama", ".jpg");


	GLuint orange = Texture::loadTexture("media/texture/diffuse-orange.png");
	GLuint black = Texture::loadTexture("media/texture/diffuse-black.png");
	GLuint normal = Texture::loadTexture("media/texture/normal.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, orange);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, black);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normal);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);

	prog.setUniform("IsToonLighting", toonShading);
}


void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
	}
	catch (GLSLProgramException& e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update(float t)
{
	// update view
	view = CamControls::getViewMatrix();

	// Add a speed control variable
	float rotationSpeed = .25f; // Adjust this value to control rotation speed

	// Light position update
	float deltaT = t - tPrev;
	if (tPrev == 0.0f) deltaT = 0.0f;
	tPrev = t;

	// Update angle using the rotation speed
	angle += rotationSpeed * deltaT; // Use rotationSpeed to control speed
	if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();

	// Light position: Rotate diagonally around the origin
	float radius = 35.0f; // Distance from the origin
	vec4 lightPos = vec4(radius * cos(angle), radius, radius * sin(angle), 1.0f); // Diagonal rotation

	// Transform light position to view space
	prog.setUniform("Spotlight.Position", vec3(view * lightPos));

	// Calculate light direction: Point from light position to origin (0, 0, 0)
	vec3 lightDir = normalize(vec3(0.0f) - vec3(lightPos)); // Direction toward origin
	mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
	prog.setUniform("Spotlight.Direction", normalMatrix * lightDir);

	static bool tKeyPressed = false;

	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_T) == GLFW_PRESS) {
		if (!tKeyPressed) {
			toonShading = !toonShading; // Toggle toonShading
			tKeyPressed = true; // Mark key as pressed
			prog.setUniform("IsToonLighting", toonShading);

		}
	}
	else {
		tKeyPressed = false; // Reset key state when T is released
	}


	SetCarRotation();
}

void SceneBasic_Uniform::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render skybox

	//vec3 cameraPos = vec3(7.0f * cos(angle), 2.0f, 7.0f * sin(angle));
	
	prog.setUniform("IsSkyBox", true);
	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, 11.0f, 0.0f));
	setMatrices();
	skybox.render();


	// render platform

	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 100.f);
	prog.setUniform("IsTextured", false);
	prog.setUniform("IsSkyBox", false);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, 0.0f, 0.0f));
	setMatrices();
	platform.render();

	// render car

	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 250.f);
	prog.setUniform("IsTextured", true);
	prog.setUniform("IsSkyBox", false);

	model = mat4(1.0f);
	model = glm::scale(model, vec3(5.f));
	model = glm::translate(model, vec3(0.0f, 0.8f, 0.0f));
	model = glm::rotate(model, glm::radians(carRoation), vec3(0.0f, 1.0f, 0.0f));

	setMatrices();
	car->render();


}

void SceneBasic_Uniform::resize(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::setMatrices() {
	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix", mv);
	prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::SetCarRotation()
{
	static bool playAnimation = true;
	static bool spaceKeyPressed = false;
	static bool isNegative = false;

	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (!spaceKeyPressed) {
			spaceKeyPressed = true;
			playAnimation = !playAnimation;
		}
	}
	else
		spaceKeyPressed = false;

	if (playAnimation)
		carRoation += isNegative ? -0.005f : 0.005f;

	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
		carRoation += 0.005f; // Increase speed
		isNegative = false;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT) == GLFW_PRESS) {
		carRoation -= 0.005f; // Decrease speed
		isNegative = true;
	}
}