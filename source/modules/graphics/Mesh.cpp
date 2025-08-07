#include "modules/graphics/Mesh.hpp"
#include "common/Exception.hpp"
#include <cstring>

using namespace love;

love::Type MeshBase::type("Mesh", &Drawable::type);

static const char* drawModeNames[] = {
    "fan",
    "strip", 
    "triangles",
    "points"
};

MeshBase::MeshBase(const std::vector<Vertex>& vertices, DrawMode mode, BufferDataUsage usage)
    : vertices(vertices)
    , drawMode(mode)
    , usage(usage)
    , texture(nullptr)
    , hasDrawRange(false)
    , drawRangeStart(0)
    , drawRangeCount(0)
{
}

MeshBase::MeshBase(int vertexCount, DrawMode mode, BufferDataUsage usage)
    : drawMode(mode)
    , usage(usage)
    , texture(nullptr)
    , hasDrawRange(false)
    , drawRangeStart(0)
    , drawRangeCount(0)
{
    vertices.resize(vertexCount);
}

MeshBase::~MeshBase()
{
    if (texture)
        texture->release();
}

void MeshBase::setVertices(const std::vector<Vertex>& verts)
{
    vertices = verts;
}

const std::vector<Vertex>& MeshBase::getVertices() const
{
    return vertices;
}

void MeshBase::setVertex(size_t index, const Vertex& vertex)
{
    if (index >= vertices.size())
        throw Exception("Mesh vertex index %zu out of range (mesh has %zu vertices)", index, vertices.size());
    
    vertices[index] = vertex;
}

Vertex MeshBase::getVertex(size_t index) const
{
    if (index >= vertices.size())
        throw Exception("Mesh vertex index %zu out of range (mesh has %zu vertices)", index, vertices.size());
    
    return vertices[index];
}

void MeshBase::setVertexMap(const std::vector<uint32_t>& map)
{
    vertexMap = map;
}

void MeshBase::setVertexMap()
{
    vertexMap.clear();
}

const std::vector<uint32_t>& MeshBase::getVertexMap() const
{
    return vertexMap;
}

void MeshBase::setTexture(TextureBase* tex)
{
    if (texture)
        texture->release();
    
    texture = tex;
    
    if (texture)
        texture->retain();
}

TextureBase* MeshBase::getTexture() const
{
    return texture;
}

void MeshBase::setDrawMode(DrawMode mode)
{
    drawMode = mode;
}

MeshBase::DrawMode MeshBase::getDrawMode() const
{
    return drawMode;
}

void MeshBase::setDrawRange(int start, int count)
{
    if (start < 0 || count < 0)
        throw Exception("Invalid draw range values");
    
    drawRangeStart = start;
    drawRangeCount = count;
    hasDrawRange = true;
}

void MeshBase::setDrawRange()
{
    hasDrawRange = false;
    drawRangeStart = 0;
    drawRangeCount = 0;
}

bool MeshBase::getDrawRange(int& start, int& count) const
{
    if (hasDrawRange)
    {
        start = drawRangeStart;
        count = drawRangeCount;
        return true;
    }
    return false;
}

size_t MeshBase::getVertexCount() const
{
    return vertices.size();
}

bool MeshBase::getConstant(const char* in, DrawMode& out)
{
    for (int i = 0; i < MESHDRAWMODE_MAX_ENUM; i++)
    {
        if (strcmp(drawModeNames[i], in) == 0)
        {
            out = (DrawMode) i;
            return true;
        }
    }
    return false;
}

bool MeshBase::getConstant(DrawMode in, const char*& out)
{
    if (in < 0 || in >= MESHDRAWMODE_MAX_ENUM)
        return false;
    
    out = drawModeNames[in];
    return true;
}

std::vector<std::string> MeshBase::getConstants(DrawMode)
{
    std::vector<std::string> constants;
    for (int i = 0; i < MESHDRAWMODE_MAX_ENUM; i++)
        constants.push_back(std::string(drawModeNames[i]));
    return constants;
}

void MeshBase::draw(GraphicsBase* graphics, const Matrix4& transform)
{
    // Basic stub implementation - should be overridden by platform-specific classes
    // For now, just do nothing to prevent the class from being abstract
    (void)graphics; // Suppress unused parameter warning
    (void)transform; // Suppress unused parameter warning
}
