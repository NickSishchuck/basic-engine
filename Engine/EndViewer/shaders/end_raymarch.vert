#version 330 core

// Full-screen quad vertex shader for ray marching
// Each vertex defines a corner of the screen

layout(location = 0) in vec2 aPos;  // Clip-space position (-1 to 1)

// Output to fragment shader
out vec2 vScreenPos;    // Screen position for this pixel
out vec3 vRayDir;       // Ray direction in world space

// Camera uniforms
uniform mat4 uInvViewProj;  // Inverse of View * Projection matrix
uniform vec3 uCameraPos;    // Camera position (local coordinates)

void main() {
    // Pass through screen position
    gl_Position = vec4(aPos, 0.0, 1.0);
    vScreenPos = aPos;
    
    // Calculate ray direction for this pixel
    // Transform clip-space position to world space
    vec4 clipPos = vec4(aPos, 1.0, 1.0);  // z=1 for far plane
    vec4 worldPos = uInvViewProj * clipPos;
    worldPos /= worldPos.w;  // Perspective divide
    
    // Ray direction is from camera to this world position
    vRayDir = normalize(worldPos.xyz - uCameraPos);
}
