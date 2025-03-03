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

    GLuint fsQuad = 0, hdrFbo = 0, blurFbo = 0, hdrTex = 0, tex1 = 0, tex2 = 0;
    GLuint linearSampler, nearestSampler;
    int bloomBufWidth, bloomBufHeight;

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
    void pass5();
    void drawScene();
    float gauss(float, float);
    void computeLogAveLuminance();

    // customisation settings

    bool toonShading = false;
    bool hdrMode = true;
    bool normalMode = true;
    float carRotation = 45.0f;
    GLuint orange;
    GLuint black;
    GLuint blue;
    GLuint darkblue;
    GLuint darkgrey;
    GLuint grey;
    GLuint red;

    void handleTextureSelection();
    void SetCarRotation();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
