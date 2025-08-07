#pragma once

#include "modules/graphics/Drawable.hpp"
#include "modules/graphics/Texture.hpp"
#include "modules/graphics/vertex.hpp"

namespace love
{
    class MeshBase : public Drawable
    {
      public:
        static love::Type type;

        // Alias the global enum for backward compatibility
        using DrawMode = MeshDrawMode;

        enum AttributeStep
        {
            STEP_PER_VERTEX,
            STEP_PER_INSTANCE,
            STEP_MAX_ENUM
        };

        struct AttributeInfo
        {
            std::string name;
            int components;
            CommonFormat format;
            int offset;
            AttributeStep step;
        };

        MeshBase(const std::vector<Vertex>& vertices, DrawMode mode, BufferDataUsage usage);
        MeshBase(int vertexCount, DrawMode mode, BufferDataUsage usage);
        virtual ~MeshBase();

        virtual void setVertices(const std::vector<Vertex>& vertices);
        virtual const std::vector<Vertex>& getVertices() const;
        virtual void setVertex(size_t index, const Vertex& vertex);
        virtual Vertex getVertex(size_t index) const;

        virtual void setVertexMap(const std::vector<uint32_t>& map);
        virtual void setVertexMap();
        virtual const std::vector<uint32_t>& getVertexMap() const;

        virtual void setTexture(TextureBase* texture);
        virtual TextureBase* getTexture() const;

        virtual void setDrawMode(DrawMode mode);
        virtual DrawMode getDrawMode() const;

        virtual void setDrawRange(int start, int count);
        virtual void setDrawRange();
        virtual bool getDrawRange(int& start, int& count) const;

        virtual size_t getVertexCount() const;

        // Implement the draw method from Drawable
        void draw(GraphicsBase* graphics, const Matrix4& transform) override;

        static bool getConstant(const char* in, DrawMode& out);
        static bool getConstant(DrawMode in, const char*& out);
        static std::vector<std::string> getConstants(DrawMode);

      protected:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> vertexMap;
        DrawMode drawMode;
        BufferDataUsage usage;
        TextureBase* texture;
        
        bool hasDrawRange;
        int drawRangeStart;
        int drawRangeCount;
    };

    using Mesh = MeshBase;
}
