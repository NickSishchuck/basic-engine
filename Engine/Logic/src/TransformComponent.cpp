#include "../include/TransformComponent.h"
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <sstream>
#include <iomanip>


namespace Engine{
namespace Logic{

    TransformComponent::TransformComponent(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
    : position(pos), scale(scl), transformMatrix(1.0f){
        SetRotation(rot);
    }

    void TransformComponent::Update(float deltaTime){
     //TODO
    }

    void TransformComponent::SetPosition(const glm::vec3& pos){
        position = pos;
        MarkDirty();
    }

    void TransformComponent::Translate(const glm::vec3& delta) {
        position += delta;
        MarkDirty();
    }

//TODO
}
}
