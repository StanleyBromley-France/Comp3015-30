#include "helper/scene.h"
#include "helper/scenerunner.h"
#include "scenebasic_uniform.h"
#include "camControls.h"


int main(int argc, char* argv[])
{
	SceneRunner runner("Shader_Basics");

	std::unique_ptr<Scene> scene;

	scene = std::unique_ptr<Scene>(new SceneBasic_Uniform());

	CamControls::initialise(glfwGetCurrentContext());


	return runner.run(*scene);
}