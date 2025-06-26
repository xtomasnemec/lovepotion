#pragma once

#include <string>
#include <vector>
#include <map>

#ifdef __WIIU__
struct GX2VertexShader;
struct GX2PixelShader;
#endif

namespace love
{
    class ShaderCompiler
    {
    public:
        // Compile GLSL shader to GX2 format for Wii U
        static std::string compileVertexShader(const std::string& source);
        static std::string compileFragmentShader(const std::string& source);
        
        // Enhanced shader compilation with caching
        #ifdef __WIIU__
        static struct GX2VertexShader* compileAndCacheVertexShader(const std::string& source);
        static struct GX2PixelShader* compileAndCachePixelShader(const std::string& source);
        #endif
        
        // Enhanced shader functions from LOVE2D 11.5
        static bool validateShader(const std::string& source, const std::string& type);
        static std::string getShaderWarnings(const std::string& source);
        
        // Shader uniform handling
        static void setUniform(const std::string& name, float value);
        static void setUniform(const std::string& name, const std::vector<float>& values);
        static void setUniform(const std::string& name, int value);
        static void setUniform(const std::string& name, const std::vector<int>& values);
        static void setUniform(const std::string& name, float x, float y);
        static void setUniform(const std::string& name, float x, float y, float z);
        static void setUniform(const std::string& name, float x, float y, float z, float w);
        
        // Matrix uniforms
        static void setUniformMatrix(const std::string& name, const float* matrix);
        
        // Texture uniforms
        static void setTextureUniform(const std::string& name, int textureUnit);
        
        // Cache management
        static void clearCache();
        static bool hasShaderSupport();
        
    private:
        static std::string processShaderDefines(const std::string& source);
        static std::map<std::string, int> uniformLocations;
        
        #ifdef __WIIU__
        static std::map<std::string, struct GX2VertexShader*> compiledVertexShaders;
        static std::map<std::string, struct GX2PixelShader*> compiledPixelShaders;
        #endif
    };
}
