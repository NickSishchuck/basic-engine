#version 330 core

// End Dimension Ray Marching Fragment Shader
// This shader renders Minecraft's End dimension terrain using ray marching

out vec4 FragColor;

// From vertex shader
in vec2 vScreenPos;
in vec3 vRayDir;

// ============================================================================
// UNIFORMS
// ============================================================================

// Camera
uniform vec3 uCameraPos;          // Camera position (chunk-relative)
uniform ivec3 uChunkOrigin;       // Chunk origin for precision handling
uniform float uCameraAltitude;    // For LOD calculations

// Rendering settings
uniform float uMaxDistance;       // Maximum ray march distance
uniform int uMaxSteps;            // Maximum ray march steps
uniform float uTime;              // For subtle animation effects

// Quality settings
uniform int uOctaves;             // Noise octaves (LOD-adjusted)
uniform float uStepMultiplier;    // Step size multiplier (LOD-adjusted)

// Colors
uniform vec3 uEndStoneColor;      // Base color for end stone
uniform vec3 uSkyColor;           // Background color (dark void)
uniform vec3 uFogColor;           // Distance fog color
uniform float uFogDensity;        // Fog density factor

// ============================================================================
// NOISE FUNCTIONS
// ============================================================================

// Skewing factors
const float F2 = 0.36602540378;
const float G2 = 0.21132486540;
const float F3 = 0.33333333333;
const float G3 = 0.16666666667;

// Hash function for gradient generation
vec3 hash3(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453);
}

vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)),
             dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453);
}

// 3D Simplex noise
float simplex3D(vec3 p) {
    // Skew
    float s = (p.x + p.y + p.z) * F3;
    vec3 i = floor(p + s);
    float t = (i.x + i.y + i.z) * G3;
    vec3 x0 = p - (i - t);
    
    // Simplex corners
    vec3 i1, i2;
    if (x0.x >= x0.y) {
        if (x0.y >= x0.z) { i1 = vec3(1,0,0); i2 = vec3(1,1,0); }
        else if (x0.x >= x0.z) { i1 = vec3(1,0,0); i2 = vec3(1,0,1); }
        else { i1 = vec3(0,0,1); i2 = vec3(1,0,1); }
    } else {
        if (x0.y < x0.z) { i1 = vec3(0,0,1); i2 = vec3(0,1,1); }
        else if (x0.x < x0.z) { i1 = vec3(0,1,0); i2 = vec3(0,1,1); }
        else { i1 = vec3(0,1,0); i2 = vec3(1,1,0); }
    }
    
    vec3 x1 = x0 - i1 + G3;
    vec3 x2 = x0 - i2 + 2.0*G3;
    vec3 x3 = x0 - 1.0 + 3.0*G3;
    
    // Gradients
    vec3 g0 = hash3(i);
    vec3 g1 = hash3(i + i1);
    vec3 g2 = hash3(i + i2);
    vec3 g3 = hash3(i + 1.0);
    
    // Contributions
    vec4 w = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    w = w * w * w * w;
    
    return 32.0 * (w.x * dot(g0, x0) + w.y * dot(g1, x1) + 
                   w.z * dot(g2, x2) + w.w * dot(g3, x3));
}

// 2D Simplex noise
float simplex2D(vec2 p) {
    float s = (p.x + p.y) * F2;
    vec2 i = floor(p + s);
    float t = (i.x + i.y) * G2;
    vec2 x0 = p - (i - t);
    
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec2 x1 = x0 - i1 + G2;
    vec2 x2 = x0 - 1.0 + 2.0 * G2;
    
    vec2 g0 = hash2(i);
    vec2 g1 = hash2(i + i1);
    vec2 g2 = hash2(i + 1.0);
    
    vec3 w = max(0.5 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
    w = w * w * w * w;
    
    return 70.0 * (w.x * dot(g0, x0) + w.y * dot(g1, x1) + w.z * dot(g2, x2));
}

// Octave noise (FBM)
float fbm3D(vec3 p, int octaves) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++) {
        value += simplex3D(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value / maxValue;
}

// ============================================================================
// END TERRAIN DENSITY FUNCTION
// ============================================================================

const float MAIN_ISLAND_RADIUS = 500.0;
const float EXCLUSION_ZONE_START = 500.0;
const float EXCLUSION_ZONE_END = 1024.0;
const float SEA_LEVEL = 64.0;

// Height profile for main island
float mainIslandHeight(float dist) {
    if (dist > MAIN_ISLAND_RADIUS) return -100.0;
    
    float t = dist / MAIN_ISLAND_RADIUS;
    float falloff = cos(t * 3.14159265 * 0.5);
    falloff = falloff * falloff;
    
    return 40.0 * falloff;
}

// Main island density
float mainIslandDensity(vec3 pos, float horizDist) {
    float heightAtDist = mainIslandHeight(horizDist);
    float baseDensity = heightAtDist - (pos.y - SEA_LEVEL);
    
    // Add terrain noise
    float noise = fbm3D(pos * 0.02, uOctaves);
    baseDensity += noise * 8.0;
    
    // Detail noise
    float detail = simplex3D(pos * 0.05) * 2.0;
    baseDensity += detail;
    
    // Floor cutoff
    if (pos.y < 4.0) {
        baseDensity -= (4.0 - pos.y) * 2.0;
    }
    
    return baseDensity;
}

// Check if position is in outer island region
bool shouldHaveIsland(vec2 chunkPos) {
    float dist = length(chunkPos * 16.0);
    if (dist <= EXCLUSION_ZONE_END) return false;
    
    float noise = simplex2D(chunkPos * 0.5);
    float threshold = -0.8 + (dist / 3000.0);
    threshold = clamp(threshold, -0.8, -0.5);
    
    return noise < threshold;
}

// Outer island density
float outerIslandDensity(vec3 pos, float horizDist) {
    // Quick rejection
    if (horizDist < EXCLUSION_ZONE_END) return -1.0;
    
    vec2 chunkPos = floor(pos.xz / 16.0);
    float maxDensity = -1.0;
    
    // Check 3x3 chunk neighborhood
    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            vec2 checkChunk = chunkPos + vec2(dx, dz);
            
            if (shouldHaveIsland(checkChunk)) {
                // Island center with variation
                vec2 offset = vec2(
                    simplex2D(checkChunk * 0.7 + vec2(0.0, 100.0)) * 6.0,
                    simplex2D(checkChunk * 0.3 + vec2(100.0, 0.0)) * 6.0
                );
                vec2 islandCenter = checkChunk * 16.0 + 8.0 + offset;
                
                // Island properties
                float sizeNoise = simplex2D(checkChunk * 0.5);
                float radius = 20.0 + sizeNoise * 15.0;
                float height = 10.0 + sizeNoise * 10.0;
                
                // Distance to this island
                vec2 toIsland = pos.xz - islandCenter;
                float islandDist = length(toIsland);
                
                if (islandDist < radius * 1.5) {
                    float normDist = islandDist / radius;
                    float maxH = height * max(0.0, 1.0 - normDist * normDist);
                    
                    float density = maxH - abs(pos.y - SEA_LEVEL);
                    
                    // Add noise
                    density += fbm3D(pos * 0.08 + vec3(islandCenter.x, 0.0, islandCenter.y) * 0.01, 
                                     max(1, uOctaves - 1)) * 4.0;
                    
                    // Edge falloff
                    float edge = 1.0 - smoothstep(0.7, 1.0, normDist);
                    density *= edge;
                    
                    maxDensity = max(maxDensity, density);
                }
            }
        }
    }
    
    return maxDensity;
}

// Main density function
float endDensity(vec3 worldPos) {
    float horizDist = length(worldPos.xz);
    
    if (horizDist < EXCLUSION_ZONE_START) {
        return mainIslandDensity(worldPos, horizDist);
    }
    
    if (horizDist < EXCLUSION_ZONE_END) {
        return -1.0;  // Exclusion zone
    }
    
    return outerIslandDensity(worldPos, horizDist);
}

// ============================================================================
// SURFACE NORMAL CALCULATION
// ============================================================================

vec3 calculateNormal(vec3 pos) {
    const float eps = 0.5;  // Epsilon for gradient sampling
    
    return normalize(vec3(
        endDensity(pos + vec3(eps, 0, 0)) - endDensity(pos - vec3(eps, 0, 0)),
        endDensity(pos + vec3(0, eps, 0)) - endDensity(pos - vec3(0, eps, 0)),
        endDensity(pos + vec3(0, 0, eps)) - endDensity(pos - vec3(0, 0, eps))
    ));
}

// ============================================================================
// LIGHTING
// ============================================================================

vec3 shade(vec3 pos, vec3 normal) {
    // Simple directional lighting from above
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.2));
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Ambient occlusion approximation based on position
    float ao = 1.0;
    
    // Combine lighting
    vec3 color = uEndStoneColor;
    
    // Add some variation based on position
    float variation = simplex3D(pos * 0.03) * 0.1;
    color += vec3(variation, variation * 0.5, 0.0);
    
    // Apply lighting
    float ambient = 0.3;
    color *= ambient + diffuse * 0.7;
    color *= ao;
    
    return color;
}

// ============================================================================
// RAY MARCHING
// ============================================================================

vec4 rayMarch(vec3 rayOrigin, vec3 rayDir) {
    float t = 0.0;
    float maxDist = uMaxDistance;
    float baseStep = 1.0 * uStepMultiplier;
    
    for (int i = 0; i < uMaxSteps; i++) {
        vec3 pos = rayOrigin + rayDir * t;
        
        // Convert to world coordinates (add chunk origin)
        vec3 worldPos = pos + vec3(uChunkOrigin) * 16.0;
        
        float density = endDensity(worldPos);
        
        if (density > 0.0) {
            // Hit! Refine position with binary search
            float tLow = t - baseStep;
            float tHigh = t;
            
            for (int j = 0; j < 4; j++) {
                float tMid = (tLow + tHigh) * 0.5;
                vec3 midPos = rayOrigin + rayDir * tMid;
                vec3 midWorld = midPos + vec3(uChunkOrigin) * 16.0;
                
                if (endDensity(midWorld) > 0.0) {
                    tHigh = tMid;
                } else {
                    tLow = tMid;
                }
            }
            
            t = tHigh;
            vec3 hitPos = rayOrigin + rayDir * t;
            vec3 hitWorld = hitPos + vec3(uChunkOrigin) * 16.0;
            
            // Calculate normal and shade
            vec3 normal = calculateNormal(hitWorld);
            vec3 color = shade(hitWorld, normal);
            
            // Apply distance fog
            float fogFactor = 1.0 - exp(-t * uFogDensity * 0.0001);
            color = mix(color, uFogColor, fogFactor);
            
            return vec4(color, 1.0);
        }
        
        // Adaptive step size
        // Larger steps when far from surfaces (large negative density)
        // Smaller steps when close to surfaces
        float adaptiveFactor = 1.0 + clamp(-density * 0.1, 0.0, 5.0);
        
        // Also scale with distance from camera for LOD
        float distanceFactor = 1.0 + t * 0.001;
        
        t += baseStep * adaptiveFactor * distanceFactor;
        
        if (t > maxDist) break;
    }
    
    // No hit - return sky color
    return vec4(uSkyColor, 1.0);
}

// ============================================================================
// MAIN
// ============================================================================

void main() {
    vec3 rayDir = normalize(vRayDir);
    
    // Start ray at camera position
    vec3 rayOrigin = uCameraPos;
    
    // Ray march through the scene
    FragColor = rayMarch(rayOrigin, rayDir);
    
    // Optional: Add subtle star effect for deep void
    if (FragColor.rgb == uSkyColor) {
        vec2 starCoord = vScreenPos * 100.0;
        float star = step(0.998, simplex2D(starCoord));
        FragColor.rgb += vec3(star * 0.3);
    }
}
