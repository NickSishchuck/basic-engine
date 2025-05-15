#ifndef COMPONENT_H
#define COMPONENT_H

namespace Engine {
namespace Logic {

class Component {
public:
    virtual ~Component() = default;
    virtual void Update(float deltaTime) = 0;
};

} // namespace Logic
} // namespace Engine

#endif
