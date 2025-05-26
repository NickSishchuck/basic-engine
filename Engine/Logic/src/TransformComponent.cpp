#include "../include/TransformComponent.h"
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <sstream>
#include <iomanip>

namespace Engine{
namespace Logic{

    TransformComponent::TransformComponent(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
        : position(pos), rotation(rot), scale(scl), transformMatrix(1.0f), isDirty(true) {
    }

    void TransformComponent::Update(float deltaTime) {
        // Transform components typically don't need per-frame updates
        // unless you're doing interpolation or animations
        // For now, this can remain empty or handle interpolation if needed
    }

    std::string TransformComponent::GetDebugInfo() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
        oss << "Rotation: (" << rotation.x << ", " << rotation.y << ", " << rotation.z << ")\n";
        oss << "Scale: (" << scale.x << ", " << scale.y << ", " << scale.z << ")";
        return oss.str();
    }

    void TransformComponent::SetPosition(const glm::vec3& pos) {
        position = pos;
        MarkDirty();
    }

    void TransformComponent::Translate(const glm::vec3& delta) {
        position += delta;
        MarkDirty();
    }

    void TransformComponent::SetRotation(const glm::vec3& rot) {
        rotation = rot;
        MarkDirty();
    }

    void TransformComponent::Rotate(const glm::vec3& delta) {
        rotation += delta;
        MarkDirty();
    }

    void TransformComponent::SetScale(const glm::vec3& scl) {
        scale = scl;
        MarkDirty();
    }

    void TransformComponent::SetScale(float uniformScale) {
        scale = glm::vec3(uniformScale);
        MarkDirty();
    }

    const glm::mat4& TransformComponent::GetTransformMatrix() const {
        if (isDirty) {
            UpdateMatrix();
            isDirty = false;
        }
        return transformMatrix;
    }

    glm::vec3 TransformComponent::GetForward() const {
        const glm::mat4& transform = GetTransformMatrix();
        return -glm::normalize(glm::vec3(transform[2])); // Negative Z is forward in OpenGL
    }

    glm::vec3 TransformComponent::GetRight() const {
        const glm::mat4& transform = GetTransformMatrix();
        return glm::normalize(glm::vec3(transform[0])); // Positive X is right
    }

    glm::vec3 TransformComponent::GetUp() const {
        const glm::mat4& transform = GetTransformMatrix();
        return glm::normalize(glm::vec3(transform[1])); // Positive Y is up
    }

    void TransformComponent::UpdateMatrix() const {
        // Create transformation matrix: T * R * S
        transformMatrix = glm::mat4(1.0f);

        // Apply translation
        transformMatrix = glm::translate(transformMatrix, position);

        // Apply rotation (assuming rotation is in radians)
        if (rotation.x != 0.0f) transformMatrix = glm::rotate(transformMatrix, rotation.x, glm::vec3(1, 0, 0));
        if (rotation.y != 0.0f) transformMatrix = glm::rotate(transformMatrix, rotation.y, glm::vec3(0, 1, 0));
        if (rotation.z != 0.0f) transformMatrix = glm::rotate(transformMatrix, rotation.z, glm::vec3(0, 0, 1));

        // Apply scale
        transformMatrix = glm::scale(transformMatrix, scale);
    }

} // namespace Logic
} // namespace Engine
