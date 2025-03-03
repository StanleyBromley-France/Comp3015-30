#include "camControls.h"
#include <iostream>
#include <glm/ext/matrix_transform.hpp>

// Initialize static member variables
glm::vec3 CamControls::cameraPosition = glm::vec3(0);
glm::vec3 CamControls::center = glm::vec3(0.0f, 3.75f, 0.0f);
glm::vec3 CamControls::up = glm::vec3(0.0f, 1.0f, 0.0f);
float CamControls::radius = 30.0f;
float CamControls::theta = glm::radians(25.0f);
float CamControls::phi = glm::radians(65.0f);
double CamControls::lastX = 0.0;
double CamControls::lastY = 0.0;
bool CamControls::isDragging = false;
float CamControls::minY = 0.5f;
float CamControls::maxY = 15.f;


void CamControls::initialise(GLFWwindow* window) {
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    updateCameraVectors();
}

glm::mat4 CamControls::getViewMatrix() {
    return glm::lookAt(cameraPosition, center, up);
}

void CamControls::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    radius -= static_cast<float>(yoffset) * 0.5f;
    radius = glm::clamp(radius, 20.0f, 30.0f);
    updateCameraVectors();
}

void CamControls::cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging) {
        float dx = static_cast<float>(xpos - lastX) * 0.005f;
        float dy = static_cast<float>(ypos - lastY) * 0.005f;

        theta -= dx;
        phi += dy;
        phi = glm::clamp(phi, 0.1f, glm::pi<float>() - 0.1f);
        updateCameraVectors();
    }
    lastX = xpos;
    lastY = ypos;
}

void CamControls::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging = true;
            glfwGetCursorPos(window, &lastX, &lastY);
            // hides the cursor
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (action == GLFW_RELEASE) {
            isDragging = false;
            // shows the cursor
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void CamControls::updateCameraVectors() {

    cameraPosition.x = center.x + radius * sin(phi) * cos(theta);
    cameraPosition.y = center.y + radius * cos(phi);
    cameraPosition.z = center.z + radius * sin(phi) * sin(theta);

    // Clamp cameraPosition.y between minY and maxY
    if (cameraPosition.y < minY || cameraPosition.y > maxY) {
        // Calculate the target cos(phi) based on the clamped Y value
        float targetY = glm::clamp(cameraPosition.y, minY, maxY);
        float targetCosPhi = (targetY - center.y) / radius;
        targetCosPhi = glm::clamp(targetCosPhi, -1.0f, 1.0f); // Ensure valid range for acos

        // Adjust phi to ensure cameraPosition.y is within the bounds
        phi = glm::acos(targetCosPhi);

        // Recalculate the camera position with the adjusted phi
        cameraPosition.x = center.x + radius * sin(phi) * cos(theta);
        cameraPosition.y = center.y + radius * cos(phi);
        cameraPosition.z = center.z + radius * sin(phi) * sin(theta);
    }
}