/*
 * ShaderProgram.cpp
 * Shader program implementation for Love Potion Wii U
 * Provides minimal shader support compatible with existing header
 */

#include "ShaderProgram.hpp"

namespace love
{
    ShaderProgram::ShaderProgram()
    {
        // Simple initialization - header defines minimal interface
    }

    ShaderProgram::~ShaderProgram()
    {
        cleanup();
    }

    void ShaderProgram::use()
    {
        // Minimal implementation for existing interface
        // On Wii U, this would set up the shader for rendering
    }

    void ShaderProgram::cleanup()
    {
        // Clean up shader resources
        // Implementation depends on actual shader storage
    }

    void ShaderProgram::setUniform(const std::string& name, float value)
    {
        // Set float uniform - stub implementation
        (void)name; // Suppress unused parameter warning
        (void)value;
    }

    void ShaderProgram::setUniform(const std::string& name, const std::vector<float>& values)
    {
        // Set float array uniform - stub implementation  
        (void)name; // Suppress unused parameter warning
        (void)values;
    }

    void ShaderProgram::setUniform(const std::string& name, int value)
    {
        // Set int uniform - stub implementation
        (void)name; // Suppress unused parameter warning
        (void)value;
    }

    void ShaderProgram::setUniform(const std::string& name, const std::vector<int>& values)
    {
        // Set int array uniform - stub implementation
        (void)name; // Suppress unused parameter warning
        (void)values;
    }
}
