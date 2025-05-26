#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine{
namespace Logic{

class TransformComponent : public Component{
private:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    mutable glm::mat4 transformMatrix;
    mutable bool isDirty = true;

public:
    TransformComponent(const glm::vec3& pos = glm::vec3(0.0f),
                      const glm::vec3& rot = glm::vec3(0.0f),
                      const glm::vec3& scl = glm::vec3(1.0f));

    // Component interface
    void Update(float deltaTime) override;
    std::string GetTypeName() const override { return "TransformComponent"; }
    std::string GetDebugInfo() const override;

    // Position
    void SetPosition(const glm::vec3& pos);
    const glm::vec3& GetPosition() const { return position; }
    void Translate(const glm::vec3& delta);

    // Rotation (in radians)
    void SetRotation(const glm::vec3& rot);
    const glm::vec3& GetRotation() const { return rotation; }
    void Rotate(const glm::vec3& delta);

    // Scale
    void SetScale(const glm::vec3& scl);
    void SetScale(float uniformScale);
    const glm::vec3& GetScale() const { return scale; }

    // Transform matrix
    const glm::mat4& GetTransformMatrix() const;

    // Direction vectors
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

private:
    void UpdateMatrix() const;
    void MarkDirty() { isDirty = true; }
};

} // namespace Logic
} // namespace Engine

#endif
