#pragma once

#include <vector>
#include "modules/graphics/Mesh.hpp"
#include "common/Matrix.hpp"

namespace love
{
    class GraphicsBase;

    // Platform-specific implementation of Mesh for Cafe (Wii U)
    // Note: The base header defines MeshBase and aliases Mesh = MeshBase
    // For platform-specific implementations, we extend MeshBase directly
}
