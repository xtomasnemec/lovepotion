#pragma once

#include "modules/graphics/ShaderStage.hpp"
#include "common/Object.hpp"
#include "common/StrongRef.hpp"

#include <gx2/shaders.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace love
{
    class ShaderProgram : public Object
    {
    public:
        ShaderProgram();
        virtual ~ShaderProgram();

        // Create shader program from vertex and fragment shader stages
        bool create(const std::string& vertexSource, const std::string& fragmentSource);
        
        // Use this shader program for rendering
        void use();
        
        // Set uniforms
        void setUniform(const std::string& name, float value);
        void setUniform(const std::string& name, const std::vector<float>& values);
        void setUniform(const std::string& name, int value);
        void setUniform(const std::string& name, const std::vector<int>& values);
        
        // Get uniform location
        int getUniformLocation(const std::string& name);
        
        // Check if program is valid
        bool isValid() const { return vertexShader != nullptr && pixelShader != nullptr; }
        
        // Get native shader objects
        GX2VertexShader* getVertexShader() const { return vertexShader; }
        GX2PixelShader* getPixelShader() const { return pixelShader; }

    private:
        GX2VertexShader* vertexShader;
        GX2PixelShader* pixelShader;
        
        // Uniform locations cache
        std::unordered_map<std::string, int> uniformLocations;
        
        // Helper methods
        void cleanup();
        bool linkProgram();
    };
}
