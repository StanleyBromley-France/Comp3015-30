#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "glm/gtc/matrix_transform.hpp"

#include "helper/plane.h"


class SceneBasic_Uniform : public Scene
{
private:
    Plane plane;

    GLSLProgram prog;
    float tPrev;
    float angle;

    void setMatrices();
    void compile();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
