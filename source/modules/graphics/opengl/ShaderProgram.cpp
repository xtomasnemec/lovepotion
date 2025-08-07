/*
 * ShaderProgram.cpp
 * Enhanced shader program for Wii U (Cafe) platform
 * Provides shader compilation, uniform management, and GX2 integration
 */

#include "ShaderProgram.hpp"
#include "ShaderCompiler.hpp"

#ifdef __WIIU__
#include <gx2/shaders.h>
#include <gx2/draw.h>
#include <gx2/state.h>
#include <coreinit/memory.h>
#endif

#include <cstring>
#include <cstdlib>

namespace love
{
    ShaderProgram::ShaderProgram() :
        vertexShader(nullptr),
        pixelShader(nullptr)
    {
    }

    ShaderProgram::~ShaderProgram()
    {
        cleanup();
    }

    bool ShaderProgram::create(const std::string& vertexSource, const std::string& fragmentSource)
    {
        cleanup();

#ifdef __WIIU__
        // Compile vertex shader
        vertexShader = ShaderCompiler::compileAndCacheVertexShader(vertexSource);
        if (!vertexShader)
        {
            return false;
        }

        // Compile pixel shader
        pixelShader = ShaderCompiler::compileAndCachePixelShader(fragmentSource);
        if (!pixelShader)
        {
            cleanup();
            return false;
        }

        return true;
#else
        // For non-Wii U platforms, just store the source
        return true;
#endif
    }

    void ShaderProgram::use()
    {
#ifdef __WIIU__
        if (isValid())
        {
            GX2SetVertexShader(vertexShader);
            GX2SetPixelShader(pixelShader);
        }
#endif
    }

    void ShaderProgram::setUniform(const std::string& name, float value)
    {
#ifdef __WIIU__
        if (!isValid())
            return;

        // Use ShaderCompiler for uniform management
        ShaderCompiler::setUniform(name, value);
#endif
    }

    void ShaderProgram::setUniform(const std::string& name, const std::vector<float>& values)
    {
#ifdef __WIIU__
        if (!isValid() || values.empty())
            return;

        // Use ShaderCompiler for uniform management
        ShaderCompiler::setUniform(name, values);
#endif
    }

    void ShaderProgram::setUniform(const std::string& name, int value)
    {
#ifdef __WIIU__
        if (!isValid())
            return;

        // Use ShaderCompiler for uniform management
        ShaderCompiler::setUniform(name, value);
#endif
    }

    void ShaderProgram::setUniform(const std::string& name, const std::vector<int>& values)
    {
#ifdef __WIIU__
        if (!isValid() || values.empty())
            return;

        // Use ShaderCompiler for uniform management
        ShaderCompiler::setUniform(name, values);
#endif
    }

    int ShaderProgram::getUniformLocation(const std::string& name)
    {
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
        {
            return it->second;
        }

        // For GX2, uniform locations would need to be determined differently
        // This is a placeholder implementation
        int location = static_cast<int>(uniformLocations.size());
        uniformLocations[name] = location;
        return location;
    }

    void ShaderProgram::cleanup()
    {
#ifdef __WIIU__
        if (vertexShader)
        {
            // Don't free here as shaders are cached in ShaderCompiler
            vertexShader = nullptr;
        }

        if (pixelShader)
        {
            // Don't free here as shaders are cached in ShaderCompiler
            pixelShader = nullptr;
        }
#endif

        uniformLocations.clear();
    }
}
