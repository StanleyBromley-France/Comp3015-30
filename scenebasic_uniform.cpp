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
	platform(25.0f, 25.0f, 1, 1),
	tPrev(0),
	angle(0)
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

	GLuint orange = Texture::loadTexture("media/texture/diffuse-orange.png");
	GLuint black = Texture::loadTexture("media/texture/diffuse-black.png");
	GLuint normal = Texture::loadTexture("media/texture/normal.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, orange);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, black);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normal);
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
	float rotationSpeed = 1.0f; // Adjust this value to control rotation speed

	// Light position update
	float deltaT = t - tPrev;
	if (tPrev == 0.0f) deltaT = 0.0f;
	tPrev = t;

	// Update angle using the rotation speed
	angle += rotationSpeed * deltaT; // Use rotationSpeed to control speed
	if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();

	// Light position: Rotate diagonally around the origin
	float radius = 7.0f; // Distance from the origin
	vec4 lightPos = vec4(radius * cos(angle), radius, radius * sin(angle), 1.0f); // Diagonal rotation

	// Transform light position to view space
	prog.setUniform("Spotlight.Position", vec3(view * lightPos));

	// Calculate light direction: Point from light position to origin (0, 0, 0)
	vec3 lightDir = normalize(vec3(0.0f) - vec3(lightPos)); // Direction toward origin
	mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
	prog.setUniform("Spotlight.Direction", normalMatrix * lightDir);

}

void SceneBasic_Uniform::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render platform

	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 100.f);
	prog.setUniform("IsTextured", false);

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

	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, 0.8f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));

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