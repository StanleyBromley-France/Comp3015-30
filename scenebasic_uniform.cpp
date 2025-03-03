#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
#include <array>
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

	// colours
	orange = Texture::loadTexture("media/texture/diffuse-orange.png");
	black = Texture::loadTexture("media/texture/diffuse-black.png");
	blue = Texture::loadTexture("media/texture/diffuse-blue.png");
	darkblue = Texture::loadTexture("media/texture/diffuse-darkblue.png");
	darkgrey = Texture::loadTexture("media/texture/diffuse-darkgrey.png");
	grey = Texture::loadTexture("media/texture/diffuse-grey.png");
	red = Texture::loadTexture("media/texture/diffuse-red.png");
}

void SceneBasic_Uniform::initScene()
{
	compile();
	glEnable(GL_DEPTH_TEST);
	model = mat4(1.0f);
	view = CamControls::getViewMatrix();
	projection = mat4(1.0f);

	// light settings

	prog.setUniform("Spotlight.L", vec3(.6f));
	prog.setUniform("Spotlight.La", vec3(.2f));
	prog.setUniform("Spotlight.Exponent", 20.f);
	prog.setUniform("Spotlight.Cutoff", glm::radians(15.0f));

	// texture

	GLuint cubeTex = Texture::loadCubeMap("media/texture/yokohama/yokohama", ".jpg");

	GLuint normal = Texture::loadTexture("media/texture/normal.png");

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, orange);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, orange);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, normal);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);

	prog.setUniform("IsToonLighting", toonShading);

	// Setup FBO
	setupFBO();

	// Array for full-screen quad
	GLfloat verts[] = {
	-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
	};
	GLfloat tc[] = {
	0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};
	// Set up the buffers
	unsigned int handle[2];
	glGenBuffers(2, handle);
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
	// Set up the vertex array object
	glGenVertexArrays(1, &fsQuad);
	glBindVertexArray(fsQuad);
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0); // Vertex position
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2); // Texture coordinates
	glBindVertexArray(0);
	prog.setUniform("LumThresh", 1.7f);
	float weights[10], sum, sigma2 = 25.0f;
	// Compute and sum the weights
	weights[0] = gauss(0, sigma2);
	sum = weights[0];
	for (int i = 1; i < 10; i++) {
		weights[i] = gauss(float(i), sigma2);
		sum += 2 * weights[i];
	}
	// Normalize the weights and set the uniform
	for (int i = 0; i < 10; i++) {
		std::stringstream uniName;
		uniName << "Weight[" << i << "]";
		float val = weights[i] / sum;
		prog.setUniform(uniName.str().c_str(), val);
	}
	// Set up two sampler objects for linear and nearest filtering
	GLuint samplers[2];
	glGenSamplers(2, samplers);
	linearSampler = samplers[0];
	nearestSampler = samplers[1];
	GLfloat border[] = { 0.0f,0.0f,0.0f,0.0f };
	// Set up the nearest sampler
	glSamplerParameteri(nearestSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(nearestSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glSamplerParameterfv(nearestSampler, GL_TEXTURE_BORDER_COLOR, border);
	// Set up the linear sampler
	glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glSamplerParameterfv(linearSampler, GL_TEXTURE_BORDER_COLOR, border);
	// We want nearest sampling except for the last pass.
	glBindSampler(0, nearestSampler);
	glBindSampler(1, nearestSampler);
	glBindSampler(2, nearestSampler);
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
	// Add a speed control variable
	float rotationSpeed = .25f; // Adjust this value to control rotation speed

	// Light position update
	float deltaT = t - tPrev;
	if (tPrev == 0.0f) deltaT = 0.0f;
	tPrev = t;

	// Update angle using the rotation speed
	angle += rotationSpeed * deltaT; // Use rotationSpeed to control speed
	if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
	
	// toon shading toggle

	static bool tKeyPressed = false;
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_T) == GLFW_PRESS) {
		if (!tKeyPressed) {
			toonShading = !toonShading; // Toggle toonShading
			tKeyPressed = true; // Mark key as pressed
			prog.setUniform("IsToonLighting", toonShading);

		}
	}
	else {
		tKeyPressed = false;
	}


	SetCarRotation();
	handleTextureSelection();
	
	// normal map toggle

	static bool nKeyPressed = false;
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_N) == GLFW_PRESS) {
		if (!nKeyPressed) {
			nKeyPressed = true;
			normalMode = !normalMode;
			prog.setUniform("DoNormalMap", normalMode);
		}
	}
	else {
		nKeyPressed = false;
	}

}

void SceneBasic_Uniform::render()
{
	pass1();
	computeLogAveLuminance();
	pass2();
	pass3();
	pass4();
	pass5();
}

void SceneBasic_Uniform::pass1() {

	prog.setUniform("Pass", 1);

	glClearColor(1.0f, .0f, .0f, 1.0f);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// update view
	view = CamControls::getViewMatrix();
	projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 200.0f);

	drawScene();
}

void SceneBasic_Uniform::pass2()
{
	prog.setUniform("Pass", 2);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);
	glViewport(0, 0, bloomBufWidth, bloomBufHeight);
	glDisable(GL_DEPTH_TEST);

	glClearColor(0, 0, 0, 0);

	glClear(GL_COLOR_BUFFER_BIT);

	model = mat4(1.0f);
	view = mat4(1.0f);
	projection = mat4(1.0f);

	setMatrices();

	glBindVertexArray(fsQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);}

void SceneBasic_Uniform::pass3()
{
	prog.setUniform("Pass", 3);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);

	glBindVertexArray(fsQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::pass4()
{
	prog.setUniform("Pass", 4);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);

	glBindVertexArray(fsQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::pass5()
{
	prog.setUniform("Pass", 5);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);

	glBindSampler(1, linearSampler);
	glBindVertexArray(fsQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindSampler(1, nearestSampler);
}

void SceneBasic_Uniform::drawScene() {

	prog.use();

	// render light
	
	// Light position: Rotate diagonally around the origin
	float radius = 35.0f; // Distance from the origin
	vec4 lightPos = vec4(radius * cos(angle), radius, radius * sin(angle), 1.0f); // Diagonal rotation

	// Transform light position to view space
	prog.setUniform("Spotlight.Position", vec3(view * lightPos));

	// Calculate light direction: Point from light position to origin (0, 0, 0)
	vec3 lightDir = normalize(vec3(0.0f) - vec3(lightPos)); // Direction toward origin
	mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
	prog.setUniform("Spotlight.Direction", normalMatrix * lightDir);

	// render skybox

	prog.setUniform("IsSkyBox", true);
	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, 11.0f, 0.0f));
	setMatrices();
	skybox.render();


	// render platform

	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 25.f);
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
	prog.setUniform("Material.Shininess", 25.f);
	prog.setUniform("IsTextured", true);
	prog.setUniform("IsSkyBox", false);


	model = mat4(1.0f);
	model = glm::scale(model, vec3(5.f));
	model = glm::translate(model, vec3(0.0f, 0.8f, 0.0f));
	model = glm::rotate(model, glm::radians(carRotation), vec3(0.0f, 1.0f, 0.0f));

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

//sets up the fbo for rendering to a texture
void SceneBasic_Uniform::setupFBO() {
	// Generate and bind the framebuffer
	glGenFramebuffers(1, &hdrFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
	// Create the texture object
	glGenTextures(1, &hdrTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
	// Bind the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTex, 0);
	// Create the depth buffer
	GLuint depthBuf;
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	// Bind the depth buffer to the FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, depthBuf);
	// Set the targets for the fragment output variables
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	// Create an FBO for the bright-pass filter and blur
	glGenFramebuffers(1, &blurFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
	// Create two texture objects to ping-pong for the bright-pass filter
	// and the two-pass blur
	bloomBufWidth = width / 8;
	bloomBufHeight = height / 8;
	glGenTextures(1, &tex1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);
	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &tex2);
	glBindTexture(GL_TEXTURE_2D, tex2);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);
	// Bind tex1 to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);
	glDrawBuffers(1, drawBuffers);
	// Unbind the framebuffer, and revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void SceneBasic_Uniform::computeLogAveLuminance()
{
	int size = width * height;
	std::vector<GLfloat> texData(size * 3);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData.data());
	float sum = 0.0f;
	for (int i = 0; i < size; i++)
	{
		float lum = dot(vec3(texData[i * 3 + 0], texData[i * 3 + 1], texData[i * 3 + 2]), vec3(0.2126f, 0.7152f, 0.0722f));
		sum += logf(lum + 0.00001f);
	}
	prog.use();
	prog.setUniform("AveLum", expf(sum / size));
}

float SceneBasic_Uniform::gauss(float x, float sigma2)
{
	double coeff = 1.0 / (glm::two_pi<float>() * sigma2);
	double exponent = -(x * x) / (2.0 * sigma2);
	return (float)(coeff * exp(exponent));
}

void SceneBasic_Uniform::SetCarRotation()
{
    static bool playAnimation = true;
    static bool spaceKeyPressed = false;
    static bool isNegative = false;
    static double lastFrameTime = glfwGetTime(); // Time tracking

    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - lastFrameTime);
    lastFrameTime = currentTime;

    // Rotation speed in degrees per second (adjust as needed)
    const float ROTATION_SPEED = 10.0f; // Equivalent to 0.25f/frame at 60 FPS

    // Space key toggles animation
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!spaceKeyPressed) {
            spaceKeyPressed = true;
            playAnimation = !playAnimation;
        }
    } else {
        spaceKeyPressed = false;
    }

    // Automatic rotation
    if (playAnimation) {
        carRotation += (isNegative ? -ROTATION_SPEED : ROTATION_SPEED) * deltaTime;
    }

    // Manual rotation controls
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
        carRotation += ROTATION_SPEED * deltaTime;
        isNegative = false;
    }
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT) == GLFW_PRESS) {
        carRotation -= ROTATION_SPEED * deltaTime;
        isNegative = true;
    }
}

void SceneBasic_Uniform::handleTextureSelection()
{
	// Non-static array using member texture variables
	std::array<GLuint, 7> colorTextures = {
		orange,      // Key 1
		black,       // Key 2
		blue,        // Key 3
		darkblue,    // Key 4
		darkgrey,    // Key 5
		grey,        // Key 6
		red          // Key 7
	};

	GLFWwindow* window = glfwGetCurrentContext();

	for (int key = GLFW_KEY_1; key <= GLFW_KEY_8; ++key)
	{
		if (glfwGetKey(window, key) == GLFW_PRESS)
		{
			const int textureIndex = key - GLFW_KEY_1;
			if (textureIndex >= 0 && textureIndex < colorTextures.size())
			{
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, colorTextures[textureIndex]);

				if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					return;

				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, colorTextures[textureIndex]);
				return;
			}
		}
	}
}