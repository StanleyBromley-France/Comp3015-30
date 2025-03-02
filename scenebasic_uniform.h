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

    GLuint hdrFBO = 0, quad, hdrTex, avgTex;


    GLSLProgram prog;
    float tPrev;
    float angle;

    void setMatrices();
    void compile();

    void setupFBO();
    void pass1();
    void pass2();
    void pass3();
    void pass4();
    void drawScene();
    void computeLogAveLuminance();

    // customisation settings

    bool toonShading = false;
    float carRotation = 45.0f;

    void SetCarRotation();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
