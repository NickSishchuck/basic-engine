/**
 * End Dimension Viewer - Test Utility
 * 
 * This standalone test verifies the CPU implementations of:
 * - Simplex noise
 * - End density function
 * 
 * Run this before integrating with the renderer to ensure correctness.
 * 
 * Compile:
 *   g++ -std=c++17 -o test_density test_density.cpp -lm
 * 
 * Run:
 *   ./test_density
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>

// Include the implementations
#include "../Engine/EndViewer/include/SimplexNoise.h"
#include "../Engine/EndViewer/include/EndDensity.h"

using namespace EndViewer;

// ============================================================================
// TEST FRAMEWORK
// ============================================================================

int testsRun = 0;
int testsPassed = 0;

void test(const std::string& name, bool condition) {
    testsRun++;
    if (condition) {
        testsPassed++;
        std::cout << "  [PASS] " << name << std::endl;
    } else {
        std::cout << "  [FAIL] " << name << std::endl;
    }
}

void testApprox(const std::string& name, double actual, double expected, double tolerance = 0.01) {
    testsRun++;
    bool pass = std::abs(actual - expected) < tolerance;
    if (pass) {
        testsPassed++;
        std::cout << "  [PASS] " << name << " (got " << actual << ", expected " << expected << ")" << std::endl;
    } else {
        std::cout << "  [FAIL] " << name << " (got " << actual << ", expected " << expected << ")" << std::endl;
    }
}

// ============================================================================
// SIMPLEX NOISE TESTS
// ============================================================================

void testSimplexNoise() {
    std::cout << "\n=== Simplex Noise Tests ===" << std::endl;
    
    SimplexNoise noise(12345);  // Fixed seed for reproducibility
    
    // Test 1: Noise should be deterministic
    double v1 = noise.sample3D(1.0, 2.0, 3.0);
    double v2 = noise.sample3D(1.0, 2.0, 3.0);
    test("Deterministic output", v1 == v2);
    
    // Test 2: Noise should be in range [-1, 1]
    bool inRange = true;
    for (int i = 0; i < 1000; i++) {
        double x = i * 0.1;
        double y = i * 0.17;
        double z = i * 0.23;
        double val = noise.sample3D(x, y, z);
        if (val < -1.0 || val > 1.0) {
            inRange = false;
            break;
        }
    }
    test("3D noise in range [-1, 1]", inRange);
    
    // Test 3: 2D noise in range
    inRange = true;
    for (int i = 0; i < 1000; i++) {
        double x = i * 0.1;
        double y = i * 0.17;
        double val = noise.sample2D(x, y);
        if (val < -1.0 || val > 1.0) {
            inRange = false;
            break;
        }
    }
    test("2D noise in range [-1, 1]", inRange);
    
    // Test 4: Different positions give different values
    double v3 = noise.sample3D(1.0, 2.0, 3.0);
    double v4 = noise.sample3D(1.1, 2.0, 3.0);
    test("Different positions give different values", v3 != v4);
    
    // Test 5: Noise is continuous (small changes = small differences)
    double v5 = noise.sample3D(5.0, 5.0, 5.0);
    double v6 = noise.sample3D(5.001, 5.0, 5.0);
    test("Noise is continuous", std::abs(v5 - v6) < 0.1);
    
    // Test 6: Octave noise averages well
    double octaveVal = noise.octave3D(1.0, 2.0, 3.0, 4, 0.5, 2.0);
    test("Octave noise in range [-1, 1]", octaveVal >= -1.0 && octaveVal <= 1.0);
    
    // Test 7: Different seeds give different results
    SimplexNoise noise2(54321);
    double v7 = noise.sample3D(1.0, 2.0, 3.0);
    double v8 = noise2.sample3D(1.0, 2.0, 3.0);
    test("Different seeds give different results", v7 != v8);
}

// ============================================================================
// END DENSITY TESTS
// ============================================================================

void testEndDensity() {
    std::cout << "\n=== End Density Tests ===" << std::endl;
    
    EndDensity density(0);  // Seed 0 for testing
    
    // Test 1: Main island center should be solid
    double mainCenter = density.sample(0.0, 64.0, 0.0);
    test("Main island center is solid", mainCenter > 0.0);
    std::cout << "    (density = " << mainCenter << ")" << std::endl;
    
    // Test 2: High above main island should be air
    double highAbove = density.sample(0.0, 200.0, 0.0);
    test("High above main island is air", highAbove < 0.0);
    std::cout << "    (density = " << highAbove << ")" << std::endl;
    
    // Test 3: Deep void should be air
    double deepVoid = density.sample(0.0, 0.0, 0.0);
    test("Deep void is air", deepVoid < 0.0);
    std::cout << "    (density = " << deepVoid << ")" << std::endl;
    
    // Test 4: Edge of main island should be near surface
    double mainEdge = density.sample(400.0, 64.0, 0.0);
    test("Edge of main island near surface", std::abs(mainEdge) < 20.0);
    std::cout << "    (density = " << mainEdge << ")" << std::endl;
    
    // Test 5: Exclusion zone should be air
    double exclusion = density.sample(700.0, 64.0, 0.0);
    test("Exclusion zone is air", exclusion < 0.0);
    std::cout << "    (density = " << exclusion << ")" << std::endl;
    
    // Test 6: Some point in outer islands should be solid (probabilistic)
    int solidCount = 0;
    for (int i = 0; i < 100; i++) {
        double x = 2000.0 + i * 50.0;
        double d = density.sample(x, 64.0, 0.0);
        if (d > 0.0) solidCount++;
    }
    test("Outer islands region has some solid", solidCount > 0);
    std::cout << "    (found " << solidCount << " solid samples out of 100)" << std::endl;
    
    // Test 7: Very far out should be mostly air
    solidCount = 0;
    for (int i = 0; i < 100; i++) {
        double x = 50000.0 + i * 100.0;
        double d = density.sample(x, 64.0, 0.0);
        if (d > 0.0) solidCount++;
    }
    test("Far outer region is sparse", solidCount < 30);
    std::cout << "    (found " << solidCount << " solid samples out of 100)" << std::endl;
}

// ============================================================================
// ISLAND DISTRIBUTION TESTS
// ============================================================================

void testIslandDistribution() {
    std::cout << "\n=== Island Distribution Tests ===" << std::endl;
    
    EndDensity density(0);
    
    // Count islands at different distances
    auto countIslandsAtRadius = [&](double radius) -> int {
        int count = 0;
        int samples = 72;  // Every 5 degrees
        for (int i = 0; i < samples; i++) {
            double angle = i * 2.0 * 3.14159265 / samples;
            int chunkX = static_cast<int>(std::cos(angle) * radius / 16.0);
            int chunkZ = static_cast<int>(std::sin(angle) * radius / 16.0);
            if (density.shouldGenerateIsland(chunkX, chunkZ)) {
                count++;
            }
        }
        return count;
    };
    
    // Test island density at different radii
    std::cout << "  Island counts at different radii:" << std::endl;
    
    int count500 = countIslandsAtRadius(500);
    std::cout << "    500 blocks (exclusion): " << count500 << " islands" << std::endl;
    test("No islands in exclusion zone (r=500)", count500 == 0);
    
    int count1500 = countIslandsAtRadius(1500);
    std::cout << "    1500 blocks (near ring): " << count1500 << " islands" << std::endl;
    test("Some islands near ring start (r=1500)", count1500 > 0);
    
    int count3000 = countIslandsAtRadius(3000);
    std::cout << "    3000 blocks (ring peak): " << count3000 << " islands" << std::endl;
    test("Peak islands at ring (r=3000)", count3000 > count1500);
    
    int count10000 = countIslandsAtRadius(10000);
    std::cout << "    10000 blocks (outer): " << count10000 << " islands" << std::endl;
    
    // The ring structure: density should peak around 2000-4000 blocks
    test("Ring structure visible (peak at medium distance)", 
         count3000 > count1500 && count3000 >= count10000);
}

// ============================================================================
// PERFORMANCE TEST
// ============================================================================

void testPerformance() {
    std::cout << "\n=== Performance Tests ===" << std::endl;
    
    SimplexNoise noise(12345);
    EndDensity density(0);
    
    // Time simplex noise
    const int iterations = 100000;
    
    auto start = std::chrono::high_resolution_clock::now();
    double sum = 0.0;
    for (int i = 0; i < iterations; i++) {
        sum += noise.sample3D(i * 0.01, i * 0.02, i * 0.03);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "  Simplex 3D: " << iterations << " samples in " 
              << duration.count() / 1000.0 << " ms" << std::endl;
    std::cout << "    (" << duration.count() * 1000.0 / iterations << " ns/sample)" << std::endl;
    
    // Time density function
    start = std::chrono::high_resolution_clock::now();
    sum = 0.0;
    for (int i = 0; i < iterations / 10; i++) {  // Fewer iterations (density is slower)
        sum += density.sample(i * 0.5, 64.0, i * 0.3);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "  End Density: " << iterations / 10 << " samples in " 
              << duration.count() / 1000.0 << " ms" << std::endl;
    std::cout << "    (" << duration.count() * 1000.0 / (iterations / 10) << " ns/sample)" << std::endl;
    
    // Estimate ray march performance
    // Assume 256 steps per ray, 1920x1080 pixels
    double samplesPerFrame = 256 * 1920 * 1080;
    double nsPerSample = duration.count() * 1000.0 / (iterations / 10);
    double msPerFrame = samplesPerFrame * nsPerSample / 1e6;
    
    std::cout << "\n  Estimated CPU ray march time: " << msPerFrame << " ms/frame" << std::endl;
    std::cout << "  (This is why we use GPU - GPU should be 100-1000x faster)" << std::endl;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "  End Dimension Viewer - Test Suite" << std::endl;
    std::cout << "============================================" << std::endl;
    
    testSimplexNoise();
    testEndDensity();
    testIslandDistribution();
    testPerformance();
    
    std::cout << "\n============================================" << std::endl;
    std::cout << "  Results: " << testsPassed << "/" << testsRun << " tests passed" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return (testsPassed == testsRun) ? 0 : 1;
}
