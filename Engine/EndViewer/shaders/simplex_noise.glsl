// simplex_noise.glsl
// GPU implementation of Simplex Noise for the End Dimension Viewer
// This file is included in other shaders via #include or copy-paste

// ============================================================================
// CONSTANTS
// ============================================================================

// Skewing factors for 2D
const float F2 = 0.36602540378;  // 0.5 * (sqrt(3.0) - 1.0)
const float G2 = 0.21132486540;  // (3.0 - sqrt(3.0)) / 6.0

// Skewing factors for 3D
const float F3 = 0.33333333333;  // 1.0 / 3.0
const float G3 = 0.16666666667;  // 1.0 / 6.0

// ============================================================================
// PERMUTATION TABLE
// ============================================================================

// We use a texture-based approach for the permutation table on GPU
// Alternatively, we can use a mathematical hash function for simplicity

// Hash function that approximates the permutation table
// This is a compromise: faster than texture lookups, but not identical to MC
float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)),
             dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}

vec3 hash3(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453);
}

// ============================================================================
// GRADIENT FUNCTIONS
// ============================================================================

// 2D gradient from hash
vec2 gradient2D(vec2 p) {
    float h = hash(dot(p, vec2(127.1, 311.7)));
    float angle = h * 6.28318530718;  // 2 * PI
    return vec2(cos(angle), sin(angle));
}

// 3D gradient vectors (12 directions pointing to edges of a cube)
vec3 gradient3D(vec3 p) {
    // Hash to get an index 0-11
    float h = fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
    int idx = int(h * 12.0);
    
    // 12 gradient directions
    const vec3 gradients[12] = vec3[12](
        vec3(1, 1, 0), vec3(-1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0),
        vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
        vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, 1, -1), vec3(0, -1, -1)
    );
    
    return gradients[idx];
}

// ============================================================================
// 2D SIMPLEX NOISE
// ============================================================================

float simplex2D(vec2 p) {
    // Skew input space
    float s = (p.x + p.y) * F2;
    vec2 i = floor(p + s);
    
    // Unskew back
    float t = (i.x + i.y) * G2;
    vec2 x0 = p - (i - t);
    
    // Determine which simplex
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    
    // Corner offsets
    vec2 x1 = x0 - i1 + G2;
    vec2 x2 = x0 - 1.0 + 2.0 * G2;
    
    // Gradients
    vec2 g0 = gradient2D(i);
    vec2 g1 = gradient2D(i + i1);
    vec2 g2 = gradient2D(i + 1.0);
    
    // Contributions
    float n0 = 0.0, n1 = 0.0, n2 = 0.0;
    
    float t0 = 0.5 - dot(x0, x0);
    if (t0 >= 0.0) {
        t0 *= t0;
        n0 = t0 * t0 * dot(g0, x0);
    }
    
    float t1 = 0.5 - dot(x1, x1);
    if (t1 >= 0.0) {
        t1 *= t1;
        n1 = t1 * t1 * dot(g1, x1);
    }
    
    float t2 = 0.5 - dot(x2, x2);
    if (t2 >= 0.0) {
        t2 *= t2;
        n2 = t2 * t2 * dot(g2, x2);
    }
    
    // Scale to [-1, 1]
    return 70.0 * (n0 + n1 + n2);
}

// ============================================================================
// 3D SIMPLEX NOISE
// ============================================================================

float simplex3D(vec3 p) {
    // Skew input space
    float s = (p.x + p.y + p.z) * F3;
    vec3 i = floor(p + s);
    
    // Unskew back
    float t = (i.x + i.y + i.z) * G3;
    vec3 x0 = p - (i - t);
    
    // Determine which simplex (one of 6 possibilities)
    vec3 i1, i2;
    
    if (x0.x >= x0.y) {
        if (x0.y >= x0.z) {
            i1 = vec3(1, 0, 0);
            i2 = vec3(1, 1, 0);
        } else if (x0.x >= x0.z) {
            i1 = vec3(1, 0, 0);
            i2 = vec3(1, 0, 1);
        } else {
            i1 = vec3(0, 0, 1);
            i2 = vec3(1, 0, 1);
        }
    } else {
        if (x0.y < x0.z) {
            i1 = vec3(0, 0, 1);
            i2 = vec3(0, 1, 1);
        } else if (x0.x < x0.z) {
            i1 = vec3(0, 1, 0);
            i2 = vec3(0, 1, 1);
        } else {
            i1 = vec3(0, 1, 0);
            i2 = vec3(1, 1, 0);
        }
    }
    
    // Corner offsets
    vec3 x1 = x0 - i1 + G3;
    vec3 x2 = x0 - i2 + 2.0 * G3;
    vec3 x3 = x0 - 1.0 + 3.0 * G3;
    
    // Gradients
    vec3 g0 = gradient3D(i);
    vec3 g1 = gradient3D(i + i1);
    vec3 g2 = gradient3D(i + i2);
    vec3 g3 = gradient3D(i + 1.0);
    
    // Contributions
    float n0 = 0.0, n1 = 0.0, n2 = 0.0, n3 = 0.0;
    
    float t0 = 0.6 - dot(x0, x0);
    if (t0 >= 0.0) {
        t0 *= t0;
        n0 = t0 * t0 * dot(g0, x0);
    }
    
    float t1 = 0.6 - dot(x1, x1);
    if (t1 >= 0.0) {
        t1 *= t1;
        n1 = t1 * t1 * dot(g1, x1);
    }
    
    float t2 = 0.6 - dot(x2, x2);
    if (t2 >= 0.0) {
        t2 *= t2;
        n2 = t2 * t2 * dot(g2, x2);
    }
    
    float t3 = 0.6 - dot(x3, x3);
    if (t3 >= 0.0) {
        t3 *= t3;
        n3 = t3 * t3 * dot(g3, x3);
    }
    
    // Scale to [-1, 1]
    return 32.0 * (n0 + n1 + n2 + n3);
}

// ============================================================================
// OCTAVE NOISE (FBM - Fractal Brownian Motion)
// ============================================================================

float octaveNoise2D(vec2 p, int octaves, float persistence, float lacunarity) {
    float total = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++) {
        total += simplex2D(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

float octaveNoise3D(vec3 p, int octaves, float persistence, float lacunarity) {
    float total = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++) {
        total += simplex3D(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

// ============================================================================
// MINECRAFT-ACCURATE NOISE (Optional - uses permutation texture)
// ============================================================================

// If you want exact Minecraft noise, you need to pass in a permutation texture
// and the origin offsets. This version uses the texture for lookups.

#ifdef USE_PERM_TEXTURE
uniform sampler1D uPermTexture;      // 256-entry permutation table
uniform vec3 uNoiseOrigin;            // Origin offset from seed

float mcSimplex3D(vec3 p) {
    p += uNoiseOrigin;
    
    // Same algorithm as above, but use texture lookup for gradients
    float s = (p.x + p.y + p.z) * F3;
    vec3 i = floor(p + s);
    
    float t = (i.x + i.y + i.z) * G3;
    vec3 x0 = p - (i - t);
    
    // ... (rest of the algorithm using texelFetch for permutation)
    // This is more accurate but requires texture setup on CPU side
    
    return 0.0;  // Placeholder
}
#endif

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Remap noise from [-1, 1] to [0, 1]
float noiseToUnit(float n) {
    return n * 0.5 + 0.5;
}

// Remap noise from [-1, 1] to custom range
float noiseToRange(float n, float minVal, float maxVal) {
    return mix(minVal, maxVal, n * 0.5 + 0.5);
}
