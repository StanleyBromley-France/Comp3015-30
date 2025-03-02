#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "glm/gtc/matrix_transform.hpp"

#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/skybox.h"

class SceneBasic_Uniform : public Scene
{
private:
    Plane platform;
    std::unique_ptr<ObjMesh> car;
    SkyBox skybox;

    GLSLProgram prog;
    float tPrev;
    float angle;

    void setMatrices();
    void compile();

    // customisation settings

    bool toonShading = false;
    float carRoation = 45.0f;

    void SetCarRotation();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
