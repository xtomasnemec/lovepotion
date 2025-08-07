#include "common/CafeGLSL.hpp"

#ifdef USE_CAFEGLSL

#ifdef __WIIU__
#include <coreinit/dynload.h>
#include <coreinit/filesystem.h>
#include <coreinit/debug.h>
#include <whb/log.h>
#else
// Dummy implementations for non-Wii U builds
struct GX2VertexShader {};
struct GX2PixelShader {};
#endif

namespace love
{
    // Static member definitions
    bool CafeGLSLCompiler::s_initialized = false;
    bool CafeGLSLCompiler::s_available = false;

#ifdef __WIIU__
    // Dynamic loading variables
    OSDynLoad_Module CafeGLSLCompiler::s_compilerModule = nullptr;
    
    // Function pointers
    typedef void (*InitGLSLCompilerFunc)();
    typedef GX2VertexShader* (*CompileVertexShaderFunc)(const char* source, char* infoLog, int infoLogSize, int flags);
    typedef GX2PixelShader* (*CompilePixelShaderFunc)(const char* source, char* infoLog, int infoLogSize, int flags);
    typedef void (*FreeVertexShaderFunc)(GX2VertexShader* shader);
    typedef void (*FreePixelShaderFunc)(GX2PixelShader* shader);
    typedef void (*DestroyGLSLCompilerFunc)();
    
    InitGLSLCompilerFunc CafeGLSLCompiler::s_initCompiler = nullptr;
    CompileVertexShaderFunc CafeGLSLCompiler::s_compileVertexShader = nullptr;
    CompilePixelShaderFunc CafeGLSLCompiler::s_compilePixelShader = nullptr;
    FreeVertexShaderFunc CafeGLSLCompiler::s_freeVertexShader = nullptr;
    FreePixelShaderFunc CafeGLSLCompiler::s_freePixelShader = nullptr;
    DestroyGLSLCompilerFunc CafeGLSLCompiler::s_destroyCompiler = nullptr;
#endif

    bool CafeGLSLCompiler::Initialize()
    {
        if (s_initialized)
            return s_available;

        s_initialized = true;

#ifdef __WIIU__
        // Try to load the CafeGLSL library
        const char* possiblePaths[] = {
            "fs:/vol/content/glslcompiler.rpl",
            "/vol/content/glslcompiler.rpl",
            "content/glslcompiler.rpl",
            "fs:/content/glslcompiler.rpl",
            "./content/glslcompiler.rpl",
            "glslcompiler.rpl"
        };
        
        WHBLogPrintf("CafeGLSL: Attempting to initialize shader compiler...");
        
        OSDynLoad_Error result = (OSDynLoad_Error)0;
        for (const char* path : possiblePaths)
        {
            WHBLogPrintf("CafeGLSL: Trying to load from: %s", path);
            result = OSDynLoad_Acquire(path, &s_compilerModule);
            if (result == OS_DYNLOAD_OK)
            {
                WHBLogPrintf("CafeGLSL: Successfully loaded compiler module from: %s", path);
                break;
            }
            else
            {
                WHBLogPrintf("CafeGLSL: Failed to load from %s (error: %d)", path, result);
            }
        }

        if (result != OS_DYNLOAD_OK)
        {
            WHBLogPrintf("CafeGLSL: Failed to load glslcompiler.rpl from any location");
            s_available = false;
            return false;
        }

        // Load all required functions
        bool allFunctionsLoaded = true;
        if (OSDynLoad_FindExport(s_compilerModule, (OSDynLoad_ExportType)0, "GLSL_Init", (void**)&s_initCompiler) != OS_DYNLOAD_OK)
        {
            WHBLogPrintf("CafeGLSL: Failed to find GLSL_Init export");
            allFunctionsLoaded = false;
        }

        if (OSDynLoad_FindExport(s_compilerModule, (OSDynLoad_ExportType)0, "GLSL_CompileVertexShader", (void**)&s_compileVertexShader) != OS_DYNLOAD_OK)
        {
            WHBLogPrintf("CafeGLSL: Failed to find GLSL_CompileVertexShader export");
            allFunctionsLoaded = false;
        }

        if (OSDynLoad_FindExport(s_compilerModule, (OSDynLoad_ExportType)0, "GLSL_CompilePixelShader", (void**)&s_compilePixelShader) != OS_DYNLOAD_OK)
        {
            WHBLogPrintf("CafeGLSL: Failed to find GLSL_CompilePixelShader export");
            allFunctionsLoaded = false;
        }

        if (OSDynLoad_FindExport(s_compilerModule, (OSDynLoad_ExportType)0, "GLSL_FreeVertexShader", (void**)&s_freeVertexShader) != OS_DYNLOAD_OK)
        {
            WHBLogPrintf("CafeGLSL: Failed to find GLSL_FreeVertexShader export");
            allFunctionsLoaded = false;
        }

        if (OSDynLoad_FindExport(s_compilerModule, (OSDynLoad_ExportType)0, "GLSL_FreePixelShader", (void**)&s_freePixelShader) != OS_DYNLOAD_OK)
        {
            WHBLogPrintf("CafeGLSL: Failed to find GLSL_FreePixelShader export");
            allFunctionsLoaded = false;
        }

        if (OSDynLoad_FindExport(s_compilerModule, (OSDynLoad_ExportType)0, "GLSL_Destroy", (void**)&s_destroyCompiler) != OS_DYNLOAD_OK)
        {
            WHBLogPrintf("CafeGLSL: Failed to find GLSL_Destroy export");
            allFunctionsLoaded = false;
        }

        if (!allFunctionsLoaded)
        {
            WHBLogPrintf("CafeGLSL: Not all required functions found, disabling CafeGLSL support");
            OSDynLoad_Release(s_compilerModule);
            s_compilerModule = nullptr;
            s_available = false;
            return false;
        }

        // Initialize the compiler
        if (s_initCompiler)
        {
            s_initCompiler();
            s_available = true;
            WHBLogPrintf("CafeGLSL: Ready for shader compilation");
            return true;
        }
        else
        {
            WHBLogPrintf("CafeGLSL: Failed to initialize CafeGLSL compiler");
            return false;
        }
#else
        WHBLogPrintf("CafeGLSL: CafeGLSL not available on this platform");
        return false;
#endif
    }

    void CafeGLSLCompiler::Shutdown()
    {
        if (!s_initialized || !s_available)
            return;

#ifdef __WIIU__
        if (s_destroyCompiler)
        {
            s_destroyCompiler();
            WHBLogPrintf("CafeGLSL: Compiler destroyed");
        }

        if (s_compilerModule)
        {
            OSDynLoad_Release(s_compilerModule);
            s_compilerModule = nullptr;
        }
#endif

        s_available = false;
        s_initialized = false;
    }

    GX2VertexShader* CafeGLSLCompiler::CompileVertexShader(const std::string& source)
    {
        if (!s_available)
        {
            WHBLogPrintf("CafeGLSL: Compiler not available");
            return nullptr;
        }

#ifdef __WIIU__
        char infoLog[1024] = {0};
        GX2VertexShader* shader = s_compileVertexShader(source.c_str(), infoLog, sizeof(infoLog), 0);
        
        if (!shader)
        {
            WHBLogPrintf("CafeGLSL: Failed to compile vertex shader: %s", infoLog);
            return nullptr;
        }
        
        WHBLogPrintf("CafeGLSL: Vertex shader compiled successfully");
        return shader;
#else
        return nullptr;
#endif
    }

    GX2PixelShader* CafeGLSLCompiler::CompilePixelShader(const std::string& source)
    {
        if (!s_available)
        {
            WHBLogPrintf("CafeGLSL: Compiler not available");
            return nullptr;
        }

#ifdef __WIIU__
        char infoLog[1024] = {0};
        GX2PixelShader* shader = s_compilePixelShader(source.c_str(), infoLog, sizeof(infoLog), 0);
        
        if (!shader)
        {
            WHBLogPrintf("CafeGLSL: Failed to compile pixel shader: %s", infoLog);
            return nullptr;
        }
        
        WHBLogPrintf("CafeGLSL: Pixel shader compiled successfully");
        return shader;
#else
        return nullptr;
#endif
    }

    bool CafeGLSLCompiler::IsAvailable()
    {
        return s_available;
    }

    std::string CafeGLSLCompiler::GetDefaultVertexShaderSource()
    {
        return R"(
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

uniform mat4 uProjection;
uniform mat4 uModelView;

out vec2 vTexCoord;
out vec4 vColor;

void main()
{
    gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
        )";
    }

    std::string CafeGLSLCompiler::GetDefaultPixelShaderSource()
    {
        return R"(
#version 330 core

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform bool uHasTexture;

out vec4 FragColor;

void main()
{
    if (uHasTexture)
        FragColor = texture(uTexture, vTexCoord) * vColor;
    else
        FragColor = vColor;
}
        )";
    }

} // namespace love

#endif // USE_CAFEGLSL
