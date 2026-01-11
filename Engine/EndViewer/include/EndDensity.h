#ifndef END_DENSITY_H
#define END_DENSITY_H

#include "SimplexNoise.h"
#include <cmath>
#include <algorithm>

namespace EndViewer {

/**
 * End Dimension Density Function
 * 
 * This implements Minecraft's End terrain generation algorithm.
 * Returns positive values for solid (end stone) and negative for air.
 * 
 * The terrain consists of:
 * 1. Main island at origin (roughly 0-500 blocks)
 * 2. Exclusion zone (500-1024 blocks) - no terrain
 * 3. Outer islands (1024+ blocks) - scattered floating islands
 * 4. Ring structure emerges from island distribution at cosmic scale
 */
class EndDensity {
public:
    // Noise generators (initialized with world seed)
    SimplexNoise islandNoise;      // For island placement
    SimplexNoise detailNoise;      // For surface detail
    SimplexNoise erosionNoise;     // For erosion patterns
    
    // Configuration constants (matching Minecraft)
    static constexpr double MAIN_ISLAND_RADIUS = 500.0;
    static constexpr double EXCLUSION_ZONE_START = 500.0;
    static constexpr double EXCLUSION_ZONE_END = 1024.0;
    static constexpr double SEA_LEVEL = 64.0;
    
    // Noise scales
    static constexpr double MAIN_NOISE_SCALE = 0.02;
    static constexpr double DETAIL_NOISE_SCALE = 0.05;
    static constexpr double ISLAND_CHECK_SCALE = 0.5;  // Chunk-level
    
    /**
     * Initialize with world seed
     */
    EndDensity(int64_t seed) 
        : islandNoise(seed), 
          detailNoise(seed + 1), 
          erosionNoise(seed + 2) {}
    
    /**
     * Main density function
     * @param x, y, z World coordinates (y is vertical in Minecraft)
     * @return Positive = solid, Negative = air, 0 = surface
     */
    double sample(double x, double y, double z) const {
        double horizontalDist = std::sqrt(x * x + z * z);
        
        // Inside main island region
        if (horizontalDist < EXCLUSION_ZONE_START) {
            return mainIslandDensity(x, y, z, horizontalDist);
        }
        
        // Exclusion zone - always air
        if (horizontalDist < EXCLUSION_ZONE_END) {
            return -1.0;
        }
        
        // Outer islands region
        return outerIslandDensity(x, y, z, horizontalDist);
    }
    
    /**
     * Check if a chunk should generate an outer island
     * Used for LOD and optimization
     */
    bool shouldGenerateIsland(int chunkX, int chunkZ) const {
        double dist = std::sqrt(static_cast<double>(chunkX * chunkX + chunkZ * chunkZ)) * 16.0;
        
        // Must be outside exclusion zone
        if (dist <= EXCLUSION_ZONE_END) {
            return false;
        }
        
        // Use 2D noise for island placement
        double noise = islandNoise.sample2D(
            chunkX * ISLAND_CHECK_SCALE, 
            chunkZ * ISLAND_CHECK_SCALE
        );
        
        // Threshold varies with distance - more islands at medium distances
        double threshold = -0.8 + (dist / 3000.0);
        threshold = std::clamp(threshold, -0.8, -0.5);
        
        return noise < threshold;
    }
    
    /**
     * Get island info for a chunk (center position, size)
     */
    struct IslandInfo {
        double centerX, centerZ;
        double radius;
        double height;
        bool exists;
    };
    
    IslandInfo getIslandInfo(int chunkX, int chunkZ) const {
        IslandInfo info;
        info.exists = shouldGenerateIsland(chunkX, chunkZ);
        
        if (!info.exists) {
            return info;
        }
        
        // Island center is offset from chunk center
        double offsetX = detailNoise.sample2D(chunkX * 0.7, chunkZ * 0.3) * 6.0;
        double offsetZ = detailNoise.sample2D(chunkX * 0.3, chunkZ * 0.7) * 6.0;
        
        info.centerX = chunkX * 16.0 + 8.0 + offsetX;
        info.centerZ = chunkZ * 16.0 + 8.0 + offsetZ;
        
        // Island size varies
        double sizeNoise = erosionNoise.sample2D(chunkX * 0.5, chunkZ * 0.5);
        info.radius = 20.0 + sizeNoise * 15.0;  // 5-35 blocks
        info.height = 10.0 + sizeNoise * 10.0;  // 5-20 blocks
        
        return info;
    }

private:
    /**
     * Density for the main central island
     */
    double mainIslandDensity(double x, double y, double z, double horizontalDist) const {
        // Base shape: dome that falls off with distance
        double heightAtDist = getMainIslandHeight(horizontalDist);
        double baseDensity = heightAtDist - (y - SEA_LEVEL);
        
        // Add noise for natural surface variation
        double noiseVal = islandNoise.octave3D(
            x * MAIN_NOISE_SCALE,
            y * MAIN_NOISE_SCALE * 2.0,  // Stretch vertically
            z * MAIN_NOISE_SCALE,
            4,   // octaves
            0.5, // persistence
            2.0  // lacunarity
        );
        
        baseDensity += noiseVal * 8.0;
        
        // Add detail noise for small-scale features
        double detailVal = detailNoise.octave3D(
            x * DETAIL_NOISE_SCALE,
            y * DETAIL_NOISE_SCALE,
            z * DETAIL_NOISE_SCALE,
            2,
            0.5,
            2.0
        );
        
        baseDensity += detailVal * 2.0;
        
        // Floor cutoff (prevent terrain below a certain Y)
        if (y < 4.0) {
            baseDensity -= (4.0 - y) * 2.0;
        }
        
        return baseDensity;
    }
    
    /**
     * Height profile of main island (dome shape)
     */
    double getMainIslandHeight(double dist) const {
        if (dist > MAIN_ISLAND_RADIUS) {
            return -100.0;  // Outside island
        }
        
        // Cosine falloff for natural dome shape
        double t = dist / MAIN_ISLAND_RADIUS;
        double falloff = std::cos(t * 3.14159265 * 0.5);
        falloff = falloff * falloff;  // Sharper falloff
        
        // Maximum height at center, falls to 0 at edge
        return 40.0 * falloff;
    }
    
    /**
     * Density for outer floating islands
     */
    double outerIslandDensity(double x, double y, double z, double horizontalDist) const {
        // Convert to chunk coordinates
        int chunkX = static_cast<int>(std::floor(x / 16.0));
        int chunkZ = static_cast<int>(std::floor(z / 16.0));
        
        // Check this chunk and neighbors for islands
        double density = -1.0;  // Default: air
        
        // Check 3x3 chunk area for nearby islands
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                IslandInfo island = getIslandInfo(chunkX + dx, chunkZ + dz);
                if (island.exists) {
                    double islandDensity = sampleIsland(x, y, z, island);
                    density = std::max(density, islandDensity);
                }
            }
        }
        
        return density;
    }
    
    /**
     * Sample density contribution from a single island
     */
    double sampleIsland(double x, double y, double z, const IslandInfo& island) const {
        // Distance from island center (2D)
        double dx = x - island.centerX;
        double dz = z - island.centerZ;
        double horizDist = std::sqrt(dx * dx + dz * dz);
        
        // Outside island radius
        if (horizDist > island.radius * 1.5) {
            return -1.0;
        }
        
        // Vertical distance from sea level
        double dy = y - SEA_LEVEL;
        
        // Island shape: flattened ellipsoid with noise
        double normalizedHoriz = horizDist / island.radius;
        double maxHeight = island.height * (1.0 - normalizedHoriz * normalizedHoriz);
        maxHeight = std::max(0.0, maxHeight);
        
        // Base density
        double density = maxHeight - std::abs(dy);
        
        // Add noise for organic shape
        double noiseVal = detailNoise.octave3D(
            x * 0.08 + island.centerX * 0.01,
            y * 0.1,
            z * 0.08 + island.centerZ * 0.01,
            3,
            0.5,
            2.0
        );
        
        density += noiseVal * 4.0;
        
        // Smooth falloff at edges
        double edgeFalloff = 1.0 - smoothstep(0.7, 1.0, normalizedHoriz);
        density *= edgeFalloff;
        
        return density;
    }
    
    /**
     * Smooth interpolation (hermite)
     */
    static double smoothstep(double edge0, double edge1, double x) {
        double t = std::clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return t * t * (3.0 - 2.0 * t);
    }
};

} // namespace EndViewer

#endif // END_DENSITY_H
