#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>

namespace Engine {
namespace Logic {

// Forward declaration
class Entity;

class Component {
protected:
    Entity* owner = nullptr;
    bool enabled = true;

public:
    virtual ~Component() = default;

    // Core lifecycle methods
    virtual void Initialize() {}
    virtual void Update(float deltaTime) = 0;
    virtual void Destroy() {}

    // Component state management
    virtual void SetEnabled(bool isEnabled) { enabled = isEnabled; }
    virtual bool IsEnabled() const { return enabled; }

    // Owner management (called by Entity when component is added)
    virtual void SetOwner(Entity* entity) { owner = entity; }
    virtual Entity* GetOwner() const { return owner; }

    // Debug/inspection
    virtual std::string GetTypeName() const { return "Component"; }
    virtual std::string GetDebugInfo() const { return "Base Component"; }
};

} // namespace Logic
} // namespace Engine

#endif
