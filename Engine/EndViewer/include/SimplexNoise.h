#ifndef SIMPLEX_NOISE_H
#define SIMPLEX_NOISE_H

#include <cstdint>
#include <cmath>
#include <array>

namespace EndViewer {

/**
 * Simplex Noise Implementation
 * 
 * This implementation matches Minecraft's SimplexNoiseSampler as closely as possible.
 * Used for:
 * 1. CPU-side verification against known Minecraft coordinates
 * 2. Reference for the GPU implementation
 * 
 * Based on Stefan Gustavson's simplex noise and Ken Perlin's improved noise.
 */
class SimplexNoise {
public:
    // Permutation table (256 entries, doubled for wrapping)
    std::array<uint8_t, 512> perm;
    std::array<uint8_t, 512> permMod12;
    
    // Origin offset for this noise instance
    double xo, yo, zo;
    
    /**
     * Initialize noise with a seed
     * Minecraft uses a specific seeding algorithm that we replicate here
     */
    SimplexNoise(int64_t seed);
    
    /**
     * 2D Simplex Noise
     * Returns value in range [-1, 1]
     */
    double sample2D(double x, double y) const;
    
    /**
     * 3D Simplex Noise  
     * Returns value in range [-1, 1]
     */
    double sample3D(double x, double y, double z) const;
    
    /**
     * Octave noise - multiple layers combined
     * @param octaves Number of noise layers
     * @param persistence Amplitude multiplier per octave (usually 0.5)
     * @param lacunarity Frequency multiplier per octave (usually 2.0)
     */
    double octave2D(double x, double y, int octaves, 
                    double persistence = 0.5, double lacunarity = 2.0) const;
    
    double octave3D(double x, double y, double z, int octaves,
                    double persistence = 0.5, double lacunarity = 2.0) const;

private:
    // Gradient vectors for 3D
    static constexpr double GRAD3[12][3] = {
        {1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
        {1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1},
        {0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}
    };
    
    // Gradient vectors for 2D
    static constexpr double GRAD2[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
    };
    
    // Skewing factors for 2D
    static constexpr double F2 = 0.5 * (std::sqrt(3.0) - 1.0);  // ≈ 0.366
    static constexpr double G2 = (3.0 - std::sqrt(3.0)) / 6.0;   // ≈ 0.211
    
    // Skewing factors for 3D
    static constexpr double F3 = 1.0 / 3.0;
    static constexpr double G3 = 1.0 / 6.0;
    
    // Helper functions
    static int fastFloor(double x);
    double dot2(int gi, double x, double y) const;
    double dot3(int gi, double x, double y, double z) const;
    
    // Minecraft's random number generator (Linear Congruential Generator)
    static int64_t lcgNext(int64_t seed);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline int SimplexNoise::fastFloor(double x) {
    int xi = static_cast<int>(x);
    return x < xi ? xi - 1 : xi;
}

inline int64_t SimplexNoise::lcgNext(int64_t seed) {
    // Minecraft's LCG constants
    return seed * 6364136223846793005LL + 1442695040888963407LL;
}

inline SimplexNoise::SimplexNoise(int64_t seed) {
    // Initialize origin offset using Minecraft's method
    seed = lcgNext(seed);
    xo = static_cast<double>(seed) / static_cast<double>(1LL << 53);
    seed = lcgNext(seed);
    yo = static_cast<double>(seed) / static_cast<double>(1LL << 53);
    seed = lcgNext(seed);
    zo = static_cast<double>(seed) / static_cast<double>(1LL << 53);
    
    // Initialize permutation table
    for (int i = 0; i < 256; i++) {
        perm[i] = static_cast<uint8_t>(i);
    }
    
    // Shuffle using Fisher-Yates with Minecraft's LCG
    for (int i = 255; i > 0; i--) {
        seed = lcgNext(seed);
        // Minecraft uses a specific way to get the index
        int j = static_cast<int>((static_cast<uint64_t>(seed) >> 33) % (i + 1));
        std::swap(perm[i], perm[j]);
    }
    
    // Double the permutation table for easy wrapping
    for (int i = 0; i < 256; i++) {
        perm[256 + i] = perm[i];
        permMod12[i] = perm[i] % 12;
        permMod12[256 + i] = perm[i] % 12;
    }
}

inline double SimplexNoise::dot2(int gi, double x, double y) const {
    // Use gradient index mod 8 for 2D
    int g = gi & 7;
    return GRAD2[g][0] * x + GRAD2[g][1] * y;
}

inline double SimplexNoise::dot3(int gi, double x, double y, double z) const {
    return GRAD3[gi][0] * x + GRAD3[gi][1] * y + GRAD3[gi][2] * z;
}

inline double SimplexNoise::sample2D(double x, double y) const {
    // Apply origin offset
    x += xo;
    y += yo;
    
    // Skew input space to determine which simplex cell we're in
    double s = (x + y) * F2;
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);
    
    // Unskew back to get distances
    double t = (i + j) * G2;
    double X0 = i - t;
    double Y0 = j - t;
    double x0 = x - X0;
    double y0 = y - Y0;
    
    // Determine which simplex we're in
    int i1, j1;
    if (x0 > y0) {
        i1 = 1; j1 = 0;  // Lower triangle
    } else {
        i1 = 0; j1 = 1;  // Upper triangle
    }
    
    // Offsets for corners
    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;
    
    // Hash coordinates for gradient indices
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = perm[ii + perm[jj]];
    int gi1 = perm[ii + i1 + perm[jj + j1]];
    int gi2 = perm[ii + 1 + perm[jj + 1]];
    
    // Calculate contributions from each corner
    double n0 = 0.0, n1 = 0.0, n2 = 0.0;
    
    double t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 >= 0.0) {
        t0 *= t0;
        n0 = t0 * t0 * dot2(gi0, x0, y0);
    }
    
    double t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 >= 0.0) {
        t1 *= t1;
        n1 = t1 * t1 * dot2(gi1, x1, y1);
    }
    
    double t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 >= 0.0) {
        t2 *= t2;
        n2 = t2 * t2 * dot2(gi2, x2, y2);
    }
    
    // Scale to [-1, 1]
    return 70.0 * (n0 + n1 + n2);
}

inline double SimplexNoise::sample3D(double x, double y, double z) const {
    // Apply origin offset
    x += xo;
    y += yo;
    z += zo;
    
    // Skew input space
    double s = (x + y + z) * F3;
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);
    int k = fastFloor(z + s);
    
    // Unskew
    double t = (i + j + k) * G3;
    double X0 = i - t;
    double Y0 = j - t;
    double Z0 = k - t;
    double x0 = x - X0;
    double y0 = y - Y0;
    double z0 = z - Z0;
    
    // Determine which simplex we're in
    int i1, j1, k1;  // Offsets for second corner
    int i2, j2, k2;  // Offsets for third corner
    
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        } else if (x0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 0; k2 = 1;
        } else {
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 1; j2 = 0; k2 = 1;
        }
    } else {
        if (y0 < z0) {
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 0; j2 = 1; k2 = 1;
        } else if (x0 < z0) {
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 0; j2 = 1; k2 = 1;
        } else {
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        }
    }
    
    // Offsets for remaining corners
    double x1 = x0 - i1 + G3;
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;
    double x2 = x0 - i2 + 2.0 * G3;
    double y2 = y0 - j2 + 2.0 * G3;
    double z2 = z0 - k2 + 2.0 * G3;
    double x3 = x0 - 1.0 + 3.0 * G3;
    double y3 = y0 - 1.0 + 3.0 * G3;
    double z3 = z0 - 1.0 + 3.0 * G3;
    
    // Hash coordinates
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = permMod12[ii + perm[jj + perm[kk]]];
    int gi1 = permMod12[ii + i1 + perm[jj + j1 + perm[kk + k1]]];
    int gi2 = permMod12[ii + i2 + perm[jj + j2 + perm[kk + k2]]];
    int gi3 = permMod12[ii + 1 + perm[jj + 1 + perm[kk + 1]]];
    
    // Calculate contributions
    double n0 = 0.0, n1 = 0.0, n2 = 0.0, n3 = 0.0;
    
    double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 >= 0.0) {
        t0 *= t0;
        n0 = t0 * t0 * dot3(gi0, x0, y0, z0);
    }
    
    double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 >= 0.0) {
        t1 *= t1;
        n1 = t1 * t1 * dot3(gi1, x1, y1, z1);
    }
    
    double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 >= 0.0) {
        t2 *= t2;
        n2 = t2 * t2 * dot3(gi2, x2, y2, z2);
    }
    
    double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 >= 0.0) {
        t3 *= t3;
        n3 = t3 * t3 * dot3(gi3, x3, y3, z3);
    }
    
    // Scale to [-1, 1]
    return 32.0 * (n0 + n1 + n2 + n3);
}

inline double SimplexNoise::octave2D(double x, double y, int octaves,
                                      double persistence, double lacunarity) const {
    double total = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++) {
        total += sample2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

inline double SimplexNoise::octave3D(double x, double y, double z, int octaves,
                                      double persistence, double lacunarity) const {
    double total = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++) {
        total += sample3D(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

// Define the static constexpr arrays
constexpr double SimplexNoise::GRAD3[12][3];
constexpr double SimplexNoise::GRAD2[8][2];

} // namespace EndViewer

#endif // SIMPLEX_NOISE_H
