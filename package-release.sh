#!/bin/bash
# Package PlaneEngine Windows Release

set -e

VERSION="v0.1.0"
RELEASE_NAME="PlaneEngine-Windows-${VERSION}"
BUILD_DIR="build-windows"
PACKAGE_DIR="release/${RELEASE_NAME}"

echo "📦 Creating Windows release package: ${RELEASE_NAME}"

# Check if Windows build exists
if [ ! -f "${BUILD_DIR}/Engine/App/PlaneApp.exe" ]; then
    echo "❌ Windows build not found. Run './build.sh windows' first!"
    exit 1
fi

# Create release directory
rm -rf release
mkdir -p "${PACKAGE_DIR}"

echo "📋 Copying files..."

# 1. Main executable
echo "  ✓ PlaneApp.exe"
cp "${BUILD_DIR}/Engine/App/PlaneApp.exe" "${PACKAGE_DIR}/"

# 2. Shaders (essential for rendering)
if [ -d "${BUILD_DIR}/Engine/App/shaders" ]; then
    echo "  ✓ shaders/"
    cp -r "${BUILD_DIR}/Engine/App/shaders" "${PACKAGE_DIR}/"
else
    echo "  ⚠️  Shaders not found in build directory, copying from source..."
    cp -r "renderer/shaders" "${PACKAGE_DIR}/"
fi

# 3. Create README for Windows users
echo "  ✓ README-Windows.txt"
cat > "${PACKAGE_DIR}/README-Windows.txt" << 'EOF'
# PlaneEngine - Windows Release

## System Requirements
- Windows 10/11 (64-bit)
- OpenGL 3.3+ compatible graphics card
- Updated graphics drivers

## How to Run
1. Extract all files to a folder
2. Double-click PlaneApp.exe to start
3. Use the controls below to navigate

## Controls
- W/A/S/D: Move camera forward/left/backward/right
- Mouse: Look around (click and drag)
- E/Q: Move camera up/down

## UI Controls
- Floor Settings: Toggle floor, adjust size and grid lines
- Camera Settings: Modify movement speed and sensitivity
- Cube Settings: Control demo cube animation speed
- Performance: Monitor FPS and frame timing

## Troubleshooting

**If the program doesn't start:**
- Make sure you have updated graphics drivers
- Try running as administrator
- Check Windows Event Viewer for error details

**If you see graphics issues:**
- Update your graphics drivers
- Make sure your GPU supports OpenGL 3.3+

**Performance issues:**
- Close other graphics-intensive applications
- Reduce window size by dragging corners
- Lower grid line count in the UI

## Engine Information
- Built with: OpenGL 3.3+, GLFW, GLEW, GLM, ImGui
- Architecture: Entity-Component System (ECS)
- Renderer: Custom OpenGL renderer with real-time debugging

For more information, visit: https://github.com/YourUsername/PlaneEngine
EOF

# 4. Create quick start guide
echo "  ✓ QUICK-START.txt"
cat > "${PACKAGE_DIR}/QUICK-START.txt" << 'EOF'
QUICK START GUIDE
================

1. Make sure all files are in the same folder:
   - PlaneApp.exe
   - shaders/ (folder)

2. Double-click PlaneApp.exe

3. You should see a 3D scene with:
   - A moving colorful cube
   - A gray floor with grid lines
   - A control panel on the right

4. Try these controls:
   - Move mouse to look around
   - W/A/S/D to move
   - Use the UI panel to change settings

That's it! Enjoy exploring the engine.
EOF

# 5. Check for any required DLLs (in case static linking missed something)
echo "🔍 Checking for required DLLs..."
REQUIRED_DLLS=""
if command -v x86_64-w64-mingw32-objdump &> /dev/null; then
    DLLS=$(x86_64-w64-mingw32-objdump -p "${BUILD_DIR}/Engine/App/PlaneApp.exe" | grep "DLL Name:" | awk '{print $3}' | grep -v "KERNEL32.dll\|USER32.dll\|GDI32.dll\|msvcrt.dll\|OPENGL32.dll\|GLU32.dll" || true)

    if [ -n "$DLLS" ]; then
        echo "  ⚠️  Additional DLLs may be required:"
        echo "$DLLS" | while read dll; do
            echo "    - $dll"
            # Try to find and copy the DLL
            DLL_PATH=$(find /usr/x86_64-w64-mingw32 -name "$dll" 2>/dev/null | head -1)
            if [ -n "$DLL_PATH" ]; then
                echo "    ✓ Found and copied: $dll"
                cp "$DLL_PATH" "${PACKAGE_DIR}/"
                REQUIRED_DLLS="$REQUIRED_DLLS $dll"
            fi
        done
    else
        echo "  ✅ No additional DLLs required (static linking successful)"
    fi
fi

# 6. Create file listing
echo "  ✓ FILE-LIST.txt"
cat > "${PACKAGE_DIR}/FILE-LIST.txt" << EOF
PlaneEngine Windows Release Files
=================================

EXECUTABLE:
  PlaneApp.exe                 - Main application

ESSENTIAL FILES:
  shaders/                     - OpenGL shader files (required)
  shaders/default.vert         - Vertex shader
  shaders/default.frag         - Fragment shader

DOCUMENTATION:
  README-Windows.txt           - Detailed information and troubleshooting
  QUICK-START.txt             - Quick start guide
  FILE-LIST.txt               - This file

$(if [ -n "$REQUIRED_DLLS" ]; then
echo "RUNTIME LIBRARIES:"
for dll in $REQUIRED_DLLS; do
    echo "  $dll                     - Required runtime library"
done
fi)

TOTAL FILES: $(find "${PACKAGE_DIR}" -type f | wc -l)
TOTAL SIZE: $(du -sh "${PACKAGE_DIR}" | cut -f1)
EOF

# 7. Create the archive
echo "📦 Creating archive..."
cd release
zip -r "${RELEASE_NAME}.zip" "${RELEASE_NAME}/"
cd ..

# 8. Show results
echo ""
echo "🎉 Windows release package created!"
echo "📁 Location: release/${RELEASE_NAME}.zip"
echo "📊 Package contents:"
find "${PACKAGE_DIR}" -type f | sed 's/.*\//  /' | sort
echo ""
echo "📈 Package size: $(du -sh "release/${RELEASE_NAME}.zip" | cut -f1)"
