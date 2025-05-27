# BasicEngine

A modular C++ engine built with OpenGL, featuring an Entity-Component System (ECS) architecture and real-time 3D rendering capabilities.

![BasicEngine Demo](https://github.com/user-attachments/assets/ee85d7da-6996-430b-b825-a1198613d134)

## Features

- **Entity-Component System (ECS)**: Flexible architecture for game object management
- **OpenGL Renderer**: OpenGL 3.3+ rendering pipeline
- **Real-time Debugging**: ImGui integration for live parameter tweaking

- **3D Mesh Rendering**: Support for cubes and custom geometry
- **Camera System**: Free-look camera with customizable speed and sensitivity
- **Shader Management**: Vertex and fragment shader support (Intentional no textures)
- **Live Controls**: Real-time adjustment parameters of context with ImGui

## Architecture

```
BasicEngine/
├── Engine/
│   ├── App/           # Application layer and main entry point
│   ├── Common/        # Shared utilities and renderer wrapper
│   └── Logic/         # ECS system and game logic components
└── renderer/          # OpenGL rendering submodule (external)
```

### Key Components

- **RendererWrapper**: Abstraction layer over the OpenGL renderer
- **Entity System**: Base entity class with component management
- **TransformComponent**: 3D transformation handling with position, rotation, and scale
- **Camera**: 3D camera with mouse and keyboard input handling

## Building the Project

### Prerequisites

- **CMake** 3.10 or higher
- **C++17** compatible compiler
- **OpenGL** 3.3+ compatible graphics driver
- **Git** (for submodule management)

### Dependencies
- OpenGL
- GLEW
- GLFW
- GLM
- ImGui (Included)

### Build Instructions

1. **Clone with submodules**:
   ```bash
   git clone --recurse-submodules https://github.com/NickSishchuck/plane-engine
   cd BasicEngine
   ```

   If you already cloned without submodules:
   ```bash
   git submodule init
   git submodule update
   ```

2. **Build the project**:
   ```bash
   mkdir build
   cd build
   cmake ..
   make  # or cmake --build . on Windows
   ```

3. **Run the application**:
   ```bash
   ./Engine/App/BasicApp
   ```

## Usage

### Controls

**Camera Movement:**
- `W/A/S/D` - Move forward/left/backward/right
- `Mouse` - Look around
- `E/Q` - Move up/down

**UI Controls:**
- **Floor Settings**: Toggle floor visibility, adjust size and grid line count
- **Camera Settings**: Modify movement speed and mouse sensitivity
- **Cube Settings**: Control animation speed of the demo cube
- **Performance**: Monitor FPS and frame timing


## Project Structure

### Engine Modules

- **App Module**: Contains the main application entry point and high-level game loop
- **Common Module**: Shared utilities, renderer abstraction, and common interfaces
- **Logic Module**: ECS implementation, components, and game logic systems

### External Dependencies

- **Renderer Submodule**: External OpenGL rendering library providing low-level graphics functionality

## Submodule Management

This project uses Git submodules for the renderer. See `Submodule guide.md` for detailed instructions
