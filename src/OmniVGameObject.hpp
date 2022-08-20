#pragma once

#include "common.hpp"
#include "OmniVModel.hpp"

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

    struct PointLightComponent {
        float lightIntensity = 1.0f;
        bool drawBillboard = false;
    };

    class OmniVGameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, OmniVGameObject>;

        static OmniVGameObject createGameObject() {
            static id_t currentId = 0;
            return OmniVGameObject{ currentId++ };
        }

        static OmniVGameObject makeSimplePointLight(bool drawBillboard = false, glm::vec3 color = glm::vec3(1.f), float intensity = 5.f, float radius = 0.1f);
        static OmniVGameObject makeLightFromNode(pugi::xml_node lightNode, bool drawBillboard);

        OmniVGameObject(const OmniVGameObject&) = delete;
        OmniVGameObject& operator=(const OmniVGameObject&) = delete;
        OmniVGameObject(OmniVGameObject&&) = default;
        OmniVGameObject& operator=(OmniVGameObject&&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};
        TransformComponent transform{};

        // Optional pointer components
        std::shared_ptr<OmniVModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        OmniVGameObject(id_t objId) : id{ objId } {}

        id_t id;
    };
}