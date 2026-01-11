#ifndef END_CAMERA_H
#define END_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

namespace EndViewer {

/**
 * Double-Precision Camera for Cosmic Scale Viewing
 * 
 * This camera uses double precision (f64) for position storage to maintain
 * accuracy at coordinates up to 100,000+ blocks from origin. For GPU usage,
 * positions are converted to chunk-relative coordinates.
 * 
 * Features:
 * - Logarithmic movement speed (scales with altitude)
 * - Camera-relative rendering for precision
 * - Multiple viewing modes (free flight, orbit)
 * - Smooth interpolation for cinematic movement
 */
class EndCamera {
public:
    // Position in world coordinates (double precision)
    glm::dvec3 position;
    
    // Orientation
    glm::dvec3 orientation;  // Forward direction
    glm::dvec3 up;           // Up direction
    
    // Derived values for GPU (recalculated each frame)
    glm::ivec3 chunkOrigin;  // Integer chunk coordinates
    glm::vec3 localOffset;   // Sub-chunk offset (f32)
    
    // View parameters
    float fov;
    float nearPlane;
    float farPlane;
    int width, height;
    
    // Movement parameters
    float baseSpeed;         // Base speed at ground level (blocks/sec)
    float speedMultiplier;   // User-adjustable multiplier
    float sensitivity;       // Mouse sensitivity
    
    // State
    bool firstMouse;
    double lastMouseX, lastMouseY;
    
    // Movement modes
    enum class Mode {
        FREE_FLIGHT,    // Full 6DOF movement
        ORBIT,          // Orbit around focal point
        CINEMATIC       // Smooth interpolated paths
    };
    Mode mode;
    
    // Orbit mode parameters
    glm::dvec3 orbitTarget;
    double orbitDistance;
    
    /**
     * Initialize camera at starting position
     */
    EndCamera(int width, int height, glm::dvec3 startPos = glm::dvec3(0.0, 100.0, 200.0))
        : position(startPos)
        , orientation(glm::dvec3(0.0, 0.0, -1.0))
        , up(glm::dvec3(0.0, 1.0, 0.0))
        , fov(45.0f)
        , nearPlane(0.1f)
        , farPlane(100000.0f)
        , width(width)
        , height(height)
        , baseSpeed(10.0f)
        , speedMultiplier(1.0f)
        , sensitivity(0.1f)
        , firstMouse(true)
        , lastMouseX(0.0)
        , lastMouseY(0.0)
        , mode(Mode::FREE_FLIGHT)
        , orbitTarget(glm::dvec3(0.0, 64.0, 0.0))
        , orbitDistance(500.0)
    {
        updateChunkRelativePosition();
    }
    
    /**
     * Calculate movement speed based on altitude (logarithmic scaling)
     * This allows smooth navigation from block-scale to cosmic-scale
     */
    float getCurrentSpeed() const {
        double altitude = std::max(0.0, position.y);
        
        // Logarithmic scaling: speed increases with log of altitude
        double scaleFactor = 1.0 + std::log(1.0 + altitude * 0.01);
        
        // Also consider horizontal distance from origin for outer islands
        double horizDist = std::sqrt(position.x * position.x + position.z * position.z);
        double distFactor = 1.0 + std::log(1.0 + horizDist * 0.001);
        
        return baseSpeed * speedMultiplier * static_cast<float>(scaleFactor * distFactor);
    }
    
    /**
     * Process keyboard and mouse input
     */
    void handleInput(GLFWwindow* window, float deltaTime) {
        switch (mode) {
            case Mode::FREE_FLIGHT:
                handleFreeFlight(window, deltaTime);
                break;
            case Mode::ORBIT:
                handleOrbit(window, deltaTime);
                break;
            case Mode::CINEMATIC:
                // Cinematic mode ignores input (automated movement)
                break;
        }
        
        // Update chunk-relative position after any movement
        updateChunkRelativePosition();
    }
    
    /**
     * Update chunk-relative coordinates for GPU
     * This splits the high-precision position into integer chunk + float offset
     */
    void updateChunkRelativePosition() {
        // Calculate chunk coordinates (integer)
        chunkOrigin = glm::ivec3(
            static_cast<int>(std::floor(position.x / 16.0)),
            static_cast<int>(std::floor(position.y / 16.0)),
            static_cast<int>(std::floor(position.z / 16.0))
        );
        
        // Calculate local offset within chunk (float, always in range [0, 16))
        localOffset = glm::vec3(
            static_cast<float>(position.x - chunkOrigin.x * 16.0),
            static_cast<float>(position.y - chunkOrigin.y * 16.0),
            static_cast<float>(position.z - chunkOrigin.z * 16.0)
        );
    }
    
    /**
     * Get view matrix (single precision for GPU)
     */
    glm::mat4 getViewMatrix() const {
        // View is calculated relative to local offset
        glm::vec3 pos = localOffset;
        glm::vec3 target = pos + glm::vec3(orientation);
        return glm::lookAt(pos, target, glm::vec3(up));
    }
    
    /**
     * Get projection matrix
     */
    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(
            glm::radians(fov),
            static_cast<float>(width) / static_cast<float>(height),
            nearPlane,
            farPlane
        );
    }
    
    /**
     * Get inverse of view-projection matrix (for ray generation in shader)
     */
    glm::mat4 getInverseViewProjection() const {
        return glm::inverse(getProjectionMatrix() * getViewMatrix());
    }
    
    /**
     * Get current altitude for LOD calculations
     */
    double getAltitude() const {
        return position.y;
    }
    
    /**
     * Get distance from origin for LOD calculations
     */
    double getDistanceFromOrigin() const {
        return glm::length(position);
    }
    
    /**
     * Calculate LOD factor (0 = highest detail, 4 = lowest detail)
     */
    float getLODFactor() const {
        double dist = std::max(getAltitude(), getDistanceFromOrigin());
        return static_cast<float>(std::clamp(std::log2(dist / 100.0 + 1.0), 0.0, 4.0));
    }
    
    /**
     * Teleport to specific world coordinates
     */
    void teleportTo(const glm::dvec3& newPos) {
        position = newPos;
        updateChunkRelativePosition();
    }
    
    /**
     * Reset to starting position
     */
    void reset() {
        position = glm::dvec3(0.0, 100.0, 200.0);
        orientation = glm::dvec3(0.0, 0.0, -1.0);
        up = glm::dvec3(0.0, 1.0, 0.0);
        updateChunkRelativePosition();
    }
    
    /**
     * Update viewport size
     */
    void setViewportSize(int w, int h) {
        width = w;
        height = h;
    }
    
    /**
     * Set viewing mode
     */
    void setMode(Mode newMode) {
        mode = newMode;
        
        // Reset state for new mode
        if (mode == Mode::ORBIT) {
            orbitDistance = glm::length(position - orbitTarget);
        }
    }
    
private:
    /**
     * Handle free flight movement
     */
    void handleFreeFlight(GLFWwindow* window, float deltaTime) {
        float speed = getCurrentSpeed() * deltaTime;
        
        // Calculate right vector
        glm::dvec3 right = glm::normalize(glm::cross(orientation, up));
        
        // WASD movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            position += orientation * static_cast<double>(speed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            position -= orientation * static_cast<double>(speed);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            position -= right * static_cast<double>(speed);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            position += right * static_cast<double>(speed);
        }
        
        // Vertical movement (Space/Shift)
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            position += up * static_cast<double>(speed);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            position -= up * static_cast<double>(speed);
        }
        
        // Speed adjustment (scroll wheel handled separately)
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
            speedMultiplier = std::min(speedMultiplier * 1.1f, 100.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
            speedMultiplier = std::max(speedMultiplier * 0.9f, 0.1f);
        }
        
        // Reset to origin
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            reset();
        }
        
        // Mouse look (when right mouse button held)
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            handleMouseLook(window);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
    }
    
    /**
     * Handle mouse look
     */
    void handleMouseLook(GLFWwindow* window) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        
        if (firstMouse) {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
            return;
        }
        
        double deltaX = mouseX - lastMouseX;
        double deltaY = lastMouseY - mouseY;  // Reversed: y-up
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        
        deltaX *= sensitivity;
        deltaY *= sensitivity;
        
        // Yaw (rotation around up axis)
        glm::dmat4 yawRotation = glm::rotate(glm::dmat4(1.0), glm::radians(-deltaX), up);
        orientation = glm::dvec3(yawRotation * glm::dvec4(orientation, 0.0));
        
        // Pitch (rotation around right axis)
        glm::dvec3 right = glm::normalize(glm::cross(orientation, up));
        glm::dmat4 pitchRotation = glm::rotate(glm::dmat4(1.0), glm::radians(deltaY), right);
        glm::dvec3 newOrientation = glm::dvec3(pitchRotation * glm::dvec4(orientation, 0.0));
        
        // Prevent flipping (limit pitch to avoid gimbal lock)
        if (std::abs(glm::dot(newOrientation, up)) < 0.99) {
            orientation = newOrientation;
        }
        
        orientation = glm::normalize(orientation);
    }
    
    /**
     * Handle orbit mode movement
     */
    void handleOrbit(GLFWwindow* window, float deltaTime) {
        // Orbit distance adjustment
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            orbitDistance = std::max(10.0, orbitDistance * 0.98);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            orbitDistance = std::min(100000.0, orbitDistance * 1.02);
        }
        
        // Orbit angles from mouse
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            
            if (!firstMouse) {
                double deltaX = (mouseX - lastMouseX) * sensitivity * 0.01;
                double deltaY = (lastMouseY - mouseY) * sensitivity * 0.01;
                
                // Rotate around target
                glm::dvec3 offset = position - orbitTarget;
                
                // Horizontal rotation (yaw)
                glm::dmat4 yawRot = glm::rotate(glm::dmat4(1.0), -deltaX, up);
                offset = glm::dvec3(yawRot * glm::dvec4(offset, 0.0));
                
                // Vertical rotation (pitch) - limited
                glm::dvec3 right = glm::normalize(glm::cross(glm::normalize(offset), up));
                glm::dmat4 pitchRot = glm::rotate(glm::dmat4(1.0), deltaY, right);
                glm::dvec3 newOffset = glm::dvec3(pitchRot * glm::dvec4(offset, 0.0));
                
                // Limit vertical angle
                double vertAngle = std::asin(newOffset.y / glm::length(newOffset));
                if (std::abs(vertAngle) < glm::radians(85.0)) {
                    offset = newOffset;
                }
                
                position = orbitTarget + glm::normalize(offset) * orbitDistance;
                orientation = glm::normalize(orbitTarget - position);
            }
            
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
        
        // Update position to maintain orbit distance
        position = orbitTarget - orientation * orbitDistance;
    }
};

} // namespace EndViewer

#endif // END_CAMERA_H
