#pragma once

#include "common.hpp"
#include "Model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>
#include <pugixml.hpp>

// std
#include <memory>
#include <unordered_map>

namespace OmniV {

    struct TransformComponent {
        glm::vec3 position{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();

        glm::mat3 normalMatrix();

        void initializeFromNode(pugi::xml_node transformNode);
    };

    struct DirectionalLightComponent {
        float lightIntensity = 1.0f;
        glm::vec3 direction{};
    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;
        bool drawBillboard = false;
    };

    class GameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, GameObject>;

        static GameObject createGameObject() {
            static id_t currentId = 0;
            return GameObject{ currentId++ };
        }

        static GameObject makeSimpleDirectionalLight(glm::vec3 direction, glm::vec3 color = glm::vec3(1.f), float intensity = 5.f);
        static GameObject makeSimplePointLight(bool drawBillboard = false, glm::vec3 color = glm::vec3(1.f), float intensity = 5.f, float radius = 0.1f);
        static GameObject loadLightFromNode(pugi::xml_node lightNode, bool drawBillboard);

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        id_t getObjectID() { return m_objectID; }

        glm::vec3 m_color{};
        TransformComponent m_transform{};

        // Optional pointer components
        std::shared_ptr<Model> m_model{};
        std::unique_ptr<DirectionalLightComponent> m_directionalLight = nullptr;
        std::unique_ptr<PointLightComponent> m_pointLight = nullptr;

    private:
        GameObject(id_t objId) : m_objectID{ objId } {}

        id_t m_objectID;
    };
}