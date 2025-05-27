#!/bin/bash

# BasicEngine Multi-platform Build Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

show_help() {
    echo "Usage: $0 [OPTION]"
    echo "Build BasicEngine for different platforms"
    echo ""
    echo "Options:"
    echo "  linux     Build for Linux (default)"
    echo "  windows   Cross-compile for Windows"
    echo "  both      Build for both platforms"
    echo "  clean     Clean all build directories"
    echo "  help      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 linux"
    echo "  $0 windows"
    echo "  $0 both"
}

check_mingw() {
    if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
        echo "❌ MinGW-w64 not found. Please install:"
        echo "   sudo pacman -S mingw-w64-gcc"
        echo "   yay -S mingw-w64-glfw mingw-w64-glew mingw-w64-glm"
        exit 1
    fi
}

build_linux() {
    echo "🐧 Building for Linux..."
    cd build
    rm -rf build-linux
    mkdir build-linux
    cd build-linux

    cmake ../.. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)  # Or just 'make' for single-threaded build

    echo "✅ Linux build completed!"
    echo "   Executable: build/build-linux/Engine/App/BasicApp"
    cd ../..
}

build_linux_debug() {
    echo "🐧 Building for Linux..."
    cd build
    rm -rf build-linux
    mkdir build-linux
    cd build-linux

    cmake ../..
    make -j$(nproc)  # Or just 'make' for single-threaded build

    echo "✅ Linux build completed!"
    echo "   Executable: build/build-linux/Engine/App/BasicApp"
    cd ../..
}

build_windows() {
    echo "🪟 Building for Windows..."
    check_mingw

    cd build
    rm -rf build-windows
    mkdir build-windows
    cd build-windows

    if [ -f "/usr/share/mingw/toolchain-x86_64-w64-mingw32.cmake" ]; then
        TOOLCHAIN_FILE="/usr/share/mingw/toolchain-x86_64-w64-mingw32.cmake"
    else
        echo "⚠️  MinGW toolchain file not found, using manual configuration"
        TOOLCHAIN_FILE=""
    fi

    if [ -n "$TOOLCHAIN_FILE" ]; then
        cmake ../.. -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
                 -DCMAKE_BUILD_TYPE=Release
    else
        cmake ../.. -DBUILD_FOR_WINDOWS=ON \
                 -DCMAKE_SYSTEM_NAME=Windows \
                 -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
                 -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
                 -DCMAKE_BUILD_TYPE=Release
    fi

    make -j$(nproc)

    echo "✅ Windows build completed!"
    echo "   Executable: build/build-windows/Engine/App/BasicApp.exe"

    # Check if binary was created successfully
    if [ -f "Engine/App/BasicApp.exe" ]; then
        echo "   File info: $(file Engine/App/BasicApp.exe)"
    fi

    cd ../..
}

clean_builds() {
    echo "🧹 Cleaning build directories..."
    cd build
    rm -rf build-linux build-windows
    echo "✅ Clean completed!"
    cd ..
}

# Main script logic
case "${1:-linux}" in
    "linux")
        build_linux
        ;;
    "windows")
        build_windows
        ;;
    "both")
        build_linux
        build_windows
        ;;
    "clean")
        clean_builds
        ;;
    "help"|"--help"|"-h")
        show_help
        ;;
    *)
        echo "❌ Unknown option: $1"
        show_help
        exit 1
        ;;
esac

echo ""
echo "🎉 Build script completed!"
