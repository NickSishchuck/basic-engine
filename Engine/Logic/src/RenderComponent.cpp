#include "../include/RenderComponent.h"
#include <sstream>
#include <iomanip>

namespace Engine {
namespace Logic {

RenderComponent::RenderComponent(PrimitiveType type, const glm::vec3& col, bool isVisible)
    : primitiveType(type), color(col), visible(isVisible) {
}

void RenderComponent::Update(float deltaTime) {
    // Render components typically don't need updates
    // unless doing animations or material changes
}

std::string RenderComponent::GetDebugInfo() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    std::string typeStr;
    switch (primitiveType) {
        case PrimitiveType::CUBE: typeStr = "Cube"; break;
        case PrimitiveType::SPHERE: typeStr = "Sphere"; break;
        case PrimitiveType::PLANE: typeStr = "Plane"; break;
        case PrimitiveType::CUSTOM_MESH: typeStr = "Custom Mesh"; break;
        case PrimitiveType::CIRCLE: typeStr = "Circle"; break;
    }

    oss << "Type: " << typeStr << "\n";
    oss << "Color: (" << color.r << ", " << color.g << ", " << color.b << ")\n";
    oss << "Visible: " << (visible ? "true" : "false");

    if (!meshPath.empty()) {
        oss << "\nMesh: " << meshPath;
    }

    return oss.str();
}

} // namespace Logic
} // namespace Engine
