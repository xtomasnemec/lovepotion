#include "ShaderCompiler.hpp"

#ifdef __WIIU__
#include <gx2/shaders.h>
#include <gx2/mem.h>
#include <coreinit/memory.h>
#include <coreinit/memdefaultheap.h>
#include <gx2/registers.h>
#include <cstdlib>
#endif

#include <regex>
#include <sstream>

namespace love
{
    std::map<std::string, int> ShaderCompiler::uniformLocations;
    std::map<std::string, GX2VertexShader*> ShaderCompiler::compiledVertexShaders;
    std::map<std::string, GX2PixelShader*> ShaderCompiler::compiledPixelShaders;

    std::string ShaderCompiler::compileVertexShader(const std::string& source)
    {
#ifdef __WIIU__
        try {
            // Enhanced GLSL to GX2 vertex shader conversion
            std::string gx2Source = processShaderDefines(source);
            
            // Convert GLSL attributes to GX2 attributes
            gx2Source = std::regex_replace(gx2Source, std::regex("attribute"), "in");
            gx2Source = std::regex_replace(gx2Source, std::regex("varying"), "out");
            
            // Add required Love2D vertex shader uniforms and attributes
            std::string loveVertexHeader = R"(
#version 330 core
// Love2D vertex shader attributes
layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec2 VertexTexCoord;
layout(location = 2) in vec4 VertexColor;

// Love2D standard uniforms
uniform mat4 TransformMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 TransformProjectionMatrix;
uniform mat4 NormalMatrix;

// Love2D screen info
uniform vec4 love_ScreenSize;

// Standard varyings for fragment shader
out vec2 VaryingTexCoord;
out vec4 VaryingColor;

// User-defined varyings will be added here
)";

        // If no main function found, add default Love2D vertex main
        if (gx2Source.find("main") == std::string::npos)
        {
            gx2Source += R"(
void main()
{
    VaryingTexCoord = VertexTexCoord;
    VaryingColor = VertexColor;
    gl_Position = TransformProjectionMatrix * VertexPosition;
}
)";
        }
        
        return loveVertexHeader + "\n" + gx2Source;
        } catch (const std::exception& e) {
            return "// Vertex shader compilation failed: " + std::string(e.what());
        }
#else
        return source;
#endif
    }

    std::string ShaderCompiler::compileFragmentShader(const std::string& source)
    {
#ifdef __WIIU__
        try {
            // Enhanced GLSL to GX2 pixel shader conversion
            std::string gx2Source = processShaderDefines(source);
            
            // Convert GLSL varying to in for fragment shaders
            gx2Source = std::regex_replace(gx2Source, std::regex("varying"), "in");
            
            // Add required Love2D pixel shader header
            std::string lovePixelHeader = R"(
#version 330 core
// Love2D pixel shader inputs
in vec2 VaryingTexCoord;
in vec4 VaryingColor;

// Love2D standard uniforms
uniform sampler2D MainTex;
uniform vec4 love_ScreenSize;

// Love2D standard output
out vec4 love_Pixelcolor;

// User-defined uniforms and functions will be added here
)";

            // If no main function found, add default Love2D pixel main
            if (gx2Source.find("main") == std::string::npos)
            {
                gx2Source += R"(
void main()
{
    love_Pixelcolor = texture(MainTex, VaryingTexCoord) * VaryingColor;
}
)";
            }
            else
            {
                // Replace common Love2D shader patterns
                gx2Source = std::regex_replace(gx2Source, std::regex("gl_FragColor"), "love_Pixelcolor");
                gx2Source = std::regex_replace(gx2Source, std::regex("texture2D"), "texture");
            }
            
            return lovePixelHeader + "\n" + gx2Source;
        } catch (const std::exception& e) {
            return "// Fragment shader compilation failed: " + std::string(e.what());
        }
#else
        return source;
#endif
    }

    std::string ShaderCompiler::processShaderDefines(const std::string& source)
    {
        std::string processed = source;
        
        // Process #ifdef LOVE_SHADER_VERTEX and similar
        processed = std::regex_replace(processed, 
            std::regex("#ifdef LOVE_SHADER_VERTEX[\\s\\S]*?#endif"), "");
        processed = std::regex_replace(processed, 
            std::regex("#ifdef LOVE_SHADER_PIXEL[\\s\\S]*?#endif"), "");
            
        return processed;
    }

    GX2VertexShader* ShaderCompiler::compileAndCacheVertexShader(const std::string& source)
    {
#ifdef __WIIU__
        try {
            // Check cache first
            auto it = compiledVertexShaders.find(source);
            if (it != compiledVertexShaders.end())
                return it->second;

            // Compile GLSL to GX2 compatible code
            std::string compiledSource = compileVertexShader(source);
            
            // Check if compilation failed
            if (compiledSource.find("// Vertex shader compilation failed:") == 0) {
                return nullptr; // Return null for failed compilation
            }
            
            // For now, return nullptr to avoid crashes until proper GX2 compilation is implemented
            // TODO: Implement real GLSL-to-GX2 compilation
            
            // Cache null result to avoid repeated failed attempts
            compiledVertexShaders[source] = nullptr;
            
            return nullptr;
        } catch (const std::exception& e) {
            return nullptr;
        }
#else
        return nullptr;
#endif
    }

    GX2PixelShader* ShaderCompiler::compileAndCachePixelShader(const std::string& source)
    {
#ifdef __WIIU__
        try {
            // Check cache first
            auto it = compiledPixelShaders.find(source);
            if (it != compiledPixelShaders.end())
                return it->second;

            // Compile GLSL to GX2 compatible code
            std::string compiledSource = compileFragmentShader(source);
            
            // Check if compilation failed
            if (compiledSource.find("// Fragment shader compilation failed:") == 0) {
                return nullptr; // Return null for failed compilation
            }
            
            // For now, return nullptr to avoid crashes until proper GX2 compilation is implemented
            // TODO: Implement real GLSL-to-GX2 compilation
            
            // Cache null result to avoid repeated failed attempts
            compiledPixelShaders[source] = nullptr;
            
            return nullptr;
        } catch (const std::exception& e) {
            return nullptr;
        }
#else
        return nullptr;
#endif
    }

    bool ShaderCompiler::validateShader(const std::string& source, const std::string& type)
    {
        // Enhanced GLSL validation for Love2D compatibility
        if (source.empty())
            return false;
            
        // Check for required functions
        if (source.find("main") == std::string::npos)
        {
            // For Love2D, missing main() is acceptable as we provide defaults
            return true;
        }
            
        // Check for Love2D specific requirements
        if (type == "vertex")
        {
            // Vertex shaders should have position output
            if (source.find("gl_Position") == std::string::npos && 
                source.find("main") != std::string::npos)
                return false;
        }
        
        if (type == "fragment" || type == "pixel")
        {
            // Fragment shaders should have color output
            if (source.find("love_Pixelcolor") == std::string::npos && 
                source.find("gl_FragColor") == std::string::npos &&
                source.find("main") != std::string::npos)
                return false;
        }
            
        return true;
    }

    std::string ShaderCompiler::getShaderWarnings(const std::string& source)
    {
        std::string warnings;
        
        // Check for deprecated GLSL features
        if (source.find("gl_FragColor") != std::string::npos)
            warnings += "Warning: gl_FragColor is deprecated, use love_Pixelcolor\n";
            
        if (source.find("texture2D") != std::string::npos)
            warnings += "Warning: texture2D is deprecated, use texture()\n";
            
        if (source.find("varying") != std::string::npos)
            warnings += "Warning: varying is deprecated, use in/out\n";
            
        if (source.find("attribute") != std::string::npos)
            warnings += "Warning: attribute is deprecated, use in\n";
            
        // Check for Love2D specific warnings
        if (source.find("love_ScreenSize") != std::string::npos)
            warnings += "Info: Using Love2D screen size uniform\n";
            
        return warnings;
    }

    void ShaderCompiler::setUniform(const std::string& name, float value)
    {
#ifdef __WIIU__
        // Enhanced GX2 uniform setting with proper location lookup
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
        {
            // Set uniform via GX2
            GX2SetVertexUniformReg(it->second, 1, &value);
            GX2SetPixelUniformReg(it->second, 1, &value);
        }
#endif
    }

    void ShaderCompiler::setUniform(const std::string& name, float x, float y)
    {
#ifdef __WIIU__
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
        {
            float values[2] = {x, y};
            GX2SetVertexUniformReg(it->second, 2, values);
            GX2SetPixelUniformReg(it->second, 2, values);
        }
#endif
    }

    void ShaderCompiler::setUniform(const std::string& name, float x, float y, float z)
    {
#ifdef __WIIU__
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
        {
            float values[3] = {x, y, z};
            GX2SetVertexUniformReg(it->second, 3, values);
            GX2SetPixelUniformReg(it->second, 3, values);
        }
#endif
    }

    void ShaderCompiler::setUniform(const std::string& name, float x, float y, float z, float w)
    {
#ifdef __WIIU__
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
        {
            float values[4] = {x, y, z, w};
            GX2SetVertexUniformReg(it->second, 4, values);
            GX2SetPixelUniformReg(it->second, 4, values);
        }
#endif
    }

    void ShaderCompiler::setUniformMatrix(const std::string& name, const float* matrix)
    {
#ifdef __WIIU__
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
        {
            // Assuming 4x4 matrix
            GX2SetVertexUniformReg(it->second, 16, matrix);
            GX2SetPixelUniformReg(it->second, 16, matrix);
        }
#endif
    }

    void ShaderCompiler::setTextureUniform(const std::string& name, int textureUnit)
    {
#ifdef __WIIU__
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
        {
            // Set texture sampler unit
            GX2SetPixelSampler((GX2Sampler*)nullptr, textureUnit);
        }
#endif
    }

    void ShaderCompiler::clearCache()
    {
#ifdef __WIIU__
        // Clean up cached shaders
        for (auto& pair : compiledVertexShaders)
        {
            if (pair.second)
                free(pair.second);
        }
        
        for (auto& pair : compiledPixelShaders)
        {
            if (pair.second)
                free(pair.second);
        }
        
        compiledVertexShaders.clear();
        compiledPixelShaders.clear();
#endif
        uniformLocations.clear();
    }

    bool ShaderCompiler::hasShaderSupport()
    {
#ifdef __WIIU__
        return true; // Wii U supports custom shaders via GX2
#else
        return false;
#endif
    }
}
