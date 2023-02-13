#include "KeyboardMovementController.hpp"

// std
#include <limits>

namespace OmniV {

    void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, TransformComponent& transformComponent) {
        glm::vec3 rotate{ 0 };
        if (glfwGetKey(window, m_keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, m_keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, m_keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
        if (glfwGetKey(window, m_keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            transformComponent.rotation += m_lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        transformComponent.rotation.x = glm::clamp(transformComponent.rotation.x, -1.5f, 1.5f);
        transformComponent.rotation.y = glm::mod(transformComponent.rotation.y, glm::two_pi<float>());

        float yaw = transformComponent.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };

        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, m_keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, m_keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, m_keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, m_keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, m_keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, m_keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            transformComponent.position += m_moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}