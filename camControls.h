#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class CamControls {
public:
    static void initialise(GLFWwindow* window);

    // Callback functions
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void cursorCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    // Get the current view matrix
    static glm::mat4 getViewMatrix();

private:
    static void updateCameraVectors();

    // Camera parameters
    static glm::vec3 cameraPosition;
    static glm::vec3 center;
    static glm::vec3 up;

    // Control variables
    static float radius;
    static float theta;
    static float phi;
    static double lastX;
    static double lastY;
    static bool isDragging;
    static float minY;
    static float maxY;

};
