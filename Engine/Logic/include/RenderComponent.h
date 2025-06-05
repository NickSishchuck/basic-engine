#ifndef RENDER_COMPONENT_H
#define RENDER_COMPONENT_H

#include "Component.h"
#include <glm/glm.hpp>
#include <string>

namespace Engine {
namespace Logic {

enum class PrimitiveType {
    CUBE,
    SPHERE,
    PLANE,
    CIRCLE,
    CUSTOM_MESH
};

class RenderComponent : public Component {
private:
    PrimitiveType primitiveType;
    glm::vec3 color;
    bool visible;
    std::string meshPath; // For custom meshes (future use)

public:
    RenderComponent(PrimitiveType type = PrimitiveType::CUBE,
                   const glm::vec3& col = glm::vec3(1.0f),
                   bool isVisible = true);

    // Component interface
    void Update(float deltaTime) override;
    std::string GetTypeName() const override { return "RenderComponent"; }
    std::string GetDebugInfo() const override;

    // Rendering properties
    void SetPrimitiveType(PrimitiveType type) { primitiveType = type; }
    PrimitiveType GetPrimitiveType() const { return primitiveType; }

    void SetColor(const glm::vec3& col) { color = col; }
    const glm::vec3& GetColor() const { return color; }

    void SetVisible(bool isVisible) { visible = isVisible; }
    bool IsVisible() const { return visible; }

    void SetMeshPath(const std::string& path) { meshPath = path; }
    const std::string& GetMeshPath() const { return meshPath; }
};

} // namespace Logic
} // namespace Engine

#endif
