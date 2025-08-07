#include "driver/display/GX2.hpp"
#include "driver/display/Uniform.hpp"

/* keyboard needs GX2 inited first */
#include "modules/graphics/Shader.hpp"
#include "modules/keyboard/Keyboard.hpp"

#ifdef __WIIU__
#include "WiiUGraphicsOptimizer.hpp"
#endif

#include <gx2/clear.h>
#include <gx2/context.h>
#include <gx2/display.h>
#include <gx2/event.h>
#include <gx2/state.h>
#include <gx2/swap.h>

#include <proc_ui/procui.h>
#include <coreinit/screen.h>

#include <malloc.h>

namespace love
{
#define Keyboard() (Module::getInstance<Keyboard>(Module::M_KEYBOARD))

    GX2::GX2() :
        targets {},
        context {},
        inForeground(false),
        consecutivePresentCalls(0),
        commandBuffer(nullptr),
        state(nullptr),
        dirtyProjection(false)
    {}

    GX2::~GX2()
    {
        this->deInitialize();
    }

    void GX2::deInitialize()
    {
        if (this->inForeground)
            this->onForegroundReleased();

        GX2Shutdown();

        delete this->uniform;

        free(this->state);
        this->state = nullptr;

        free(this->commandBuffer);
        this->commandBuffer = nullptr;
    }

    int GX2::onForegroundAcquired()
    {
        this->inForeground = true;

        auto foregroundHeap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG);
        auto memOneHeap     = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);

        for (auto& target : this->targets)
        {
            if (!target.allocateScanBuffer(foregroundHeap))
                return -1;

            if (!target.invalidateColorBuffer(memOneHeap))
                return -2;

            if (!target.invalidateDepthBuffer(memOneHeap))
                return -3;
        }

        return 0;
    }

    static uint32_t ProcUIAcquired(void*)
    {
        return gx2.onForegroundAcquired();
    }

    int GX2::onForegroundReleased()
    {
        GX2DrawDone();

        auto foregroundHeap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG);
        auto memOneHeap     = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);

        MEMFreeToFrmHeap(foregroundHeap, MEM_FRM_HEAP_FREE_ALL);
        MEMFreeToFrmHeap(memOneHeap, MEM_FRM_HEAP_FREE_ALL);

        this->inForeground = false;

        return 0;
    }

    static uint32_t ProcUIReleased(void*)
    {
        return gx2.onForegroundReleased();
    }

    void GX2::initialize()
    {
        if (this->initialized)
            return;

        this->commandBuffer = memalign(GX2_COMMAND_BUFFER_ALIGNMENT, GX2_COMMAND_BUFFER_SIZE);

        if (!this->commandBuffer)
            throw love::Exception("Failed to allocate GX2 command buffer.");

        // clang-format off
        uint32_t attributes[9] =
        {
            GX2_INIT_CMD_BUF_BASE, (uintptr_t)this->commandBuffer,
            GX2_INIT_CMD_BUF_POOL_SIZE, GX2_COMMAND_BUFFER_SIZE,
            GX2_INIT_ARGC, 0, GX2_INIT_ARGV, 0,
            GX2_INIT_END
        };
        // clang-format on

        GX2Init(attributes);

        this->state = (GX2ContextState*)memalign(GX2_CONTEXT_STATE_ALIGNMENT, sizeof(GX2ContextState));

        if (!this->state)
            throw love::Exception("Failed to allocate GX2 context state.");

        GX2SetupContextStateEx(this->state, false);
        GX2SetContextState(this->state);

        this->createFramebuffers();

        GX2SetDepthOnlyControl(false, false, GX2_COMPARE_FUNC_ALWAYS);
        // GX2SetAlphaTest(false, GX2_COMPARE_FUNC_ALWAYS, 0.0f);

        GX2SetColorControl(GX2_LOGIC_OP_COPY, 0xFF, false, true);
        GX2SetSwapInterval(1);

        ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, ProcUIAcquired, nullptr, 100);
        ProcUIRegisterCallback(PROCUI_CALLBACK_RELEASE, ProcUIReleased, nullptr, 100);

        if (auto result = this->onForegroundAcquired(); result != 0)
            throw love::Exception("Failed to acquire foreground: {:d}", result);

        if (Keyboard())
            Keyboard()->initialize();

        this->context.winding     = GX2_FRONT_FACE_CCW;
        this->context.cullBack    = false;
        this->context.cullFront   = false;
        this->context.depthTest   = false;
        this->context.depthWrite  = true;
        this->context.compareMode = GX2_COMPARE_FUNC_ALWAYS;

        this->uniform             = (Uniform*)memalign(GX2_UNIFORM_BLOCK_ALIGNMENT, sizeof(Uniform));
        this->uniform->modelView  = glm::mat4(1.0f);
        this->uniform->projection = glm::mat4(1.0f);

        this->bindFramebuffer(&this->targets[0].get());

        this->initialized = true;
    }

    void GX2::createFramebuffers()
    {
        const auto info = love::getScreenInfo();

        for (size_t index = 0; index < info.size(); ++index)
            this->targets[index].create(info[index]);
    }

    GX2ColorBuffer& GX2::getInternalBackbuffer()
    {
        return this->targets[love::currentScreen].get();
    }

    GX2DepthBuffer& GX2::getInternalDepthbuffer()
    {
        return this->targets[love::currentScreen].getDepth();
    }

    GX2ColorBuffer* GX2::getFramebuffer()
    {
        return this->context.boundFramebuffer;
    }

    void GX2::destroyFramebuffers()
    {
        for (auto& target : this->targets)
            target.destroy();
    }

    void GX2::ensureInFrame()
    {
#ifdef __WIIU__
        FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile) {
            fprintf(logFile, "GX2::ensureInFrame() called, inFrame = %s\n", this->inFrame ? "true" : "false");
            fflush(logFile);
            fclose(logFile);
        }
#endif
        
        GX2SetContextState(this->state);

        if (!this->inFrame)
        {
#ifdef __WIIU__
            FILE* logFile2 = fopen("fs:/vol/external01/simple_debug.log", "a");
            if (logFile2) {
                fprintf(logFile2, "GX2::ensureInFrame() - starting new frame\n");
                fflush(logFile2);
                fclose(logFile2);
            }
            // Reset consecutive present calls counter at start of new frame
            this->consecutivePresentCalls = 0;
            
            // Reset draw call optimization counters for new frame
            #ifdef __WIIU__
            WiiUGraphicsOptimizer::startFrame();
            #endif
#endif
            this->inFrame = true;
        }
        
#ifdef __WIIU__
        FILE* logFile3 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile3) {
            fprintf(logFile3, "GX2::ensureInFrame() completed, inFrame = true\n");
            fflush(logFile3);
            fclose(logFile3);
        }
#endif
    }

    void GX2::copyCurrentScanBuffer()
    {
        Graphics::flushBatchedDrawsGlobal();
        Graphics::advanceStreamBuffersGlobal();

        this->targets[love::currentScreen].copyScanBuffer();

        GX2Flush();
        GX2WaitForFlip();
    }

    void GX2::clear(const Color& color)
    {
#ifdef __WIIU__
        FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile) {
            fprintf(logFile, "GX2::clear() called with color: R=%.2f G=%.2f B=%.2f A=%.2f, inFrame = %s\n", 
                   color.r, color.g, color.b, color.a, this->inFrame ? "true" : "false");
            fflush(logFile);
            fclose(logFile);
        }
#endif
        
        if (!this->inFrame)
        {
#ifdef __WIIU__
            FILE* logFile2 = fopen("fs:/vol/external01/simple_debug.log", "a");
            if (logFile2) {
                fprintf(logFile2, "GX2::clear() - not in frame, calling ensureInFrame\n");
                fflush(logFile2);
                fclose(logFile2);
            }
#endif
            this->ensureInFrame();
        }

#ifdef __WIIU__
        FILE* logFile3 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile3) {
            fprintf(logFile3, "GX2::clear() - clearing framebuffer\n");
            fflush(logFile3);
            fclose(logFile3);
        }
#endif
        
        GX2ClearColor(this->getFramebuffer(), color.r, color.g, color.b, color.a);
        GX2SetContextState(this->state);
        
#ifdef __WIIU__
        FILE* logFile4 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile4) {
            fprintf(logFile4, "GX2::clear() completed\n");
            fflush(logFile4);
            fclose(logFile4);
        }
#endif
    }

    void GX2::clearDepthStencil(int depth, uint8_t mask, double stencil)
    {
        // GX2ClearDepthStencilEx(&this->getInternalDepthbuffer(), depth, stencil, GX2_CLEAR_FLAGS_BOTH);
        // GX2SetContextState(this->state);
    }

    void GX2::bindFramebuffer(GX2ColorBuffer* target)
    {
        bool bindingModified = false;

        if (this->context.boundFramebuffer != target)
        {
            bindingModified                = true;
            this->context.boundFramebuffer = target;
        }

        if (bindingModified)
        {
            GX2SetColorBuffer(target, GX2_RENDER_TARGET_0);
            this->setMode(target->surface.width, target->surface.height);
        }
    }

    void GX2::setMode(int width, int height)
    {
        this->setViewport({ 0, 0, width, height });
        this->setScissor({ 0, 0, width, height });

        auto* newUniform = this->targets[love::currentScreen].getUniform();
        std::memcpy(this->uniform, newUniform, sizeof(Uniform));
    }

    void GX2::setSamplerState(TextureBase* texture, const SamplerState& state)
    {
        auto* sampler = (GX2Sampler*)texture->getSamplerHandle();
        GX2InitSampler(sampler, GX2_TEX_CLAMP_MODE_WRAP, GX2_TEX_XY_FILTER_MODE_LINEAR);

        GX2TexXYFilterMode minFilter;

        if (!GX2::getConstant(state.minFilter, minFilter))
            return;

        GX2TexXYFilterMode magFilter;

        if (!GX2::getConstant(state.magFilter, magFilter))
            return;

        GX2InitSamplerXYFilter(sampler, magFilter, minFilter, GX2_TEX_ANISO_RATIO_NONE);

        GX2TexClampMode wrapU;

        if (!GX2::getConstant(state.wrapU, wrapU))
            return;

        GX2TexClampMode wrapV;

        if (!GX2::getConstant(state.wrapV, wrapV))
            return;

        GX2TexClampMode wrapW;

        if (!GX2::getConstant(state.wrapW, wrapW))
            return;

        GX2InitSamplerClamping(sampler, wrapU, wrapV, wrapW);
        GX2InitSamplerLOD(sampler, state.minLod, state.maxLod, state.lodBias);
    }

    void GX2::prepareDraw(GraphicsBase* graphics)
    {
#ifdef __WIIU__
        FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile) {
            fprintf(logFile, "GX2::prepareDraw() called\n");
            fflush(logFile);
            fclose(logFile);
        }
#endif
        
        // Ensure we're in frame before any drawing operations
        this->ensureInFrame();
        
        if (Shader::current != nullptr)
        {
            auto* shader = (Shader*)ShaderBase::current;
            shader->updateBuiltinUniforms(graphics, this->uniform);
        }
        
#ifdef __WIIU__
        FILE* logFile2 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile2) {
            fprintf(logFile2, "GX2::prepareDraw() completed\n");
            fflush(logFile2);
            fclose(logFile2);
        }
#endif
    }

    void GX2::bindTextureToUnit(TextureBase* texture, int unit)
    {
        if (texture == nullptr)
            return;

        auto* handle = (GX2Texture*)texture->getHandle();

        if (handle == nullptr)
            return;

        auto* sampler = (GX2Sampler*)texture->getSamplerHandle();

        if (sampler == nullptr)
            return;

        this->bindTextureToUnit(handle, sampler, unit);
    }

    void GX2::bindTextureToUnit(GX2Texture* texture, GX2Sampler* sampler, int unit)
    {
        auto* shader = (Shader*)ShaderBase::current;
        auto* info   = shader->getUniformInfo("texture0");

        if (!info)
            return;

        GX2SetPixelTexture(texture, unit);
        GX2SetPixelSampler(sampler, info->location);
    }

    void GX2::present()
    {
#ifdef __WIIU__
        // CRITICAL: Detect endless loop and trigger fallback
        static bool fallbackTriggered = false;
        
        this->consecutivePresentCalls++;
        
        FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile) {
            fprintf(logFile, "GX2::present() called #%d, inFrame = %s\n", this->consecutivePresentCalls, this->inFrame ? "true" : "false");
            fflush(logFile);
            fclose(logFile);
        }
        
        // DISABLED: Fallback detection to prevent black screen
        // If we've had too many consecutive present calls without a reset, trigger fallback
        // if (this->consecutivePresentCalls > 30 && !fallbackTriggered) {
        //     fallbackTriggered = true;
        //     
        //     FILE* fallbackLog = fopen("fs:/vol/external01/simple_debug.log", "a");
        //     if (fallbackLog) {
        //         fprintf(fallbackLog, "=== ENDLESS LOOP DETECTED: TRIGGERING FALLBACK DIAGNOSTIC SCREEN ===\n");
        //         fprintf(fallbackLog, "Consecutive present calls: %d\n", this->consecutivePresentCalls);
        //         fprintf(fallbackLog, "This indicates lua_resume() is hanging in an endless loop\n");
        //         fflush(fallbackLog);
        //         fclose(fallbackLog);
        //     }
        //     
        //     // Force entry into diagnostic fallback mode
        //     this->showFallbackDiagnosticScreen();
        //     return; // Skip normal present logic
        // }
#endif
        
        if (!this->inFrame)
        {
#ifdef __WIIU__
            FILE* logFile2 = fopen("fs:/vol/external01/simple_debug.log", "a");
            if (logFile2) {
                fprintf(logFile2, "GX2::present() - not in frame, calling ensureInFrame\n");
                fflush(logFile2);
                fclose(logFile2);
            }
#endif
            this->ensureInFrame();
        }
        
#ifdef __WIIU__
        FILE* logFile3 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile3) {
            fprintf(logFile3, "GX2::present() - presenting to both screens\n");
            fflush(logFile3);
            fclose(logFile3);
        }
#endif
        
        // Present to GamePad
        GX2CopyColorBufferToScanBuffer(&this->targets[0].get(), GX2_SCAN_TARGET_DRC);
        
        // Present to TV as well (copy GamePad content to TV)
        GX2CopyColorBufferToScanBuffer(&this->targets[0].get(), GX2_SCAN_TARGET_TV);
        
        // Swap buffers for both screens
        GX2SwapScanBuffers();
        GX2Flush();
        GX2WaitForVsync();
        
        this->inFrame = false;
        
#ifdef __WIIU__
        FILE* logFile4 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile4) {
            fprintf(logFile4, "GX2::present() completed, inFrame = false\n");
            fflush(logFile4);
            fclose(logFile4);
        }
#endif
    }

    void GX2::setViewport(const Rect& rect)
    {
        Rect view = rect;
        if (rect == Rect::EMPTY)
            view = this->targets[love::currentScreen].getViewport();

        GX2SetViewport(view.x, view.y, view.w, view.h, Framebuffer::Z_NEAR, Framebuffer::Z_FAR);
        this->context.viewport = view;
    }

    void GX2::setScissor(const Rect& rect)
    {
        Rect scissor = rect;
        if (rect == Rect::EMPTY)
            scissor = this->targets[love::currentScreen].getScissor();

        GX2SetScissor(scissor.x, scissor.y, scissor.w, scissor.h);
        this->context.scissor = scissor;
    }

    void GX2::setCullMode(CullMode mode)
    {
        const auto enabled = mode != CullMode::CULL_NONE;

        this->context.cullBack  = (enabled && mode == CullMode::CULL_BACK);
        this->context.cullFront = (enabled && mode == CullMode::CULL_FRONT);

        GX2SetCullOnlyControl(this->context.winding, this->context.cullBack, this->context.cullFront);
    }

    void GX2::setVertexWinding(Winding winding)
    {
        GX2FrontFace windingMode;

        if (!GX2::getConstant(winding, windingMode))
            return;

        GX2SetCullOnlyControl(windingMode, this->context.cullBack, this->context.cullFront);
        this->context.winding = windingMode;
    }

    void GX2::setColorMask(ColorChannelMask mask)
    {
        const auto red   = (GX2_CHANNEL_MASK_R * mask.r);
        const auto green = (GX2_CHANNEL_MASK_G * mask.g);
        const auto blue  = (GX2_CHANNEL_MASK_B * mask.b);
        const auto alpha = (GX2_CHANNEL_MASK_A * mask.a);

        const auto value = GX2ChannelMask(red + green + blue + alpha);
        const auto NONE  = GX2ChannelMask(0);

        GX2SetTargetChannelMasks(value, NONE, NONE, NONE, NONE, NONE, NONE, NONE);
    }

    void GX2::setBlendState(const BlendState& state)
    {
        GX2BlendCombineMode operationRGB;
        if (!GX2::getConstant(state.operationRGB, operationRGB))
            return;

        GX2BlendCombineMode operationA;
        if (!GX2::getConstant(state.operationA, operationA))
            return;

        GX2BlendMode sourceColor;
        if (!GX2::getConstant(state.srcFactorRGB, sourceColor))
            return;

        GX2BlendMode destColor;
        if (!GX2::getConstant(state.dstFactorRGB, destColor))
            return;

        GX2BlendMode sourceAlpha;
        if (!GX2::getConstant(state.srcFactorA, sourceAlpha))
            return;

        GX2BlendMode destAlpha;
        if (!GX2::getConstant(state.dstFactorA, destAlpha))
            return;

        GX2SetBlendControl(GX2_RENDER_TARGET_0, sourceColor, destColor, operationRGB, true, sourceAlpha,
                           destAlpha, operationA);
    }

    void GX2::showFallbackDiagnosticScreen()
    {
#ifdef __WIIU__
        // Force immediate display of diagnostic message using OSScreen
        // This bypasses the normal graphics pipeline which may be stuck
        
        FILE* diagnosticLog = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (diagnosticLog) {
            fprintf(diagnosticLog, "=== SHOWING FALLBACK DIAGNOSTIC SCREEN ===\n");
            fprintf(diagnosticLog, "Attempting to display diagnostic message using OSScreen\n");
            fflush(diagnosticLog);
            fclose(diagnosticLog);
        }
        
        // Try to initialize screen
        OSScreenInit();
        
        // Get buffer sizes
        size_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
        size_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
        
        // Allocate buffers (simplified approach)
        void* tvBuffer = memalign(0x100, tvBufferSize);
        void* drcBuffer = memalign(0x100, drcBufferSize);
        
        if (tvBuffer && drcBuffer) {
            // Set buffers
            OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
            OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
            
            // Enable both screens
            OSScreenEnableEx(SCREEN_TV, true);
            OSScreenEnableEx(SCREEN_DRC, true);
            
            // Clear screens to black
            OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
            OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);
            
            // Display diagnostic text with more detailed information
            const char* line1 = "LOVE POTION DIAGNOSTIC MODE";
            const char* line2 = "*** ENDLESS GRAPHICS LOOP DETECTED ***";
            const char* line3 = "lua_resume() was called but never returned";
            const char* line4 = "Lua VM stuck in infinite loop";
            const char* line5 = "Possible causes:";
            const char* line6 = "- Infinite loop in love.run() or love.update()";
            const char* line7 = "- Recursive function calls (stack overflow)";
            const char* line8 = "- Blocking operation in main loop";
            const char* line9 = "Check: fs:/vol/external01/simple_debug.log";
            const char* line10 = "Press HOME to exit to Wii U menu";
            
            // Get current system time for display
            OSCalendarTime calendarTime;
            OSTicksToCalendarTime(OSGetTime(), &calendarTime);
            
            char timeStr[64];
            snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d:%02d | Present calls: %d", 
                     calendarTime.tm_hour, calendarTime.tm_min, calendarTime.tm_sec, this->consecutivePresentCalls);
            
            // Put text on TV (more detailed)
            OSScreenPutFontEx(SCREEN_TV, 5, 2, line1);
            OSScreenPutFontEx(SCREEN_TV, 5, 4, line2);
            OSScreenPutFontEx(SCREEN_TV, 5, 5, line3);
            OSScreenPutFontEx(SCREEN_TV, 5, 6, line4);
            OSScreenPutFontEx(SCREEN_TV, 5, 8, line5);
            OSScreenPutFontEx(SCREEN_TV, 5, 9, line6);
            OSScreenPutFontEx(SCREEN_TV, 5, 10, line7);
            OSScreenPutFontEx(SCREEN_TV, 5, 11, line8);
            OSScreenPutFontEx(SCREEN_TV, 5, 13, line9);
            OSScreenPutFontEx(SCREEN_TV, 5, 14, timeStr);
            OSScreenPutFontEx(SCREEN_TV, 5, 16, line10);
            
            // Put text on GamePad (condensed)
            OSScreenPutFontEx(SCREEN_DRC, 2, 1, "LOVE POTION DIAGNOSTIC");
            OSScreenPutFontEx(SCREEN_DRC, 2, 3, "*** ENDLESS LOOP DETECTED ***");
            OSScreenPutFontEx(SCREEN_DRC, 2, 4, "lua_resume() never returned");
            OSScreenPutFontEx(SCREEN_DRC, 2, 5, "Lua VM stuck in infinite loop");
            OSScreenPutFontEx(SCREEN_DRC, 2, 7, "Possible causes:");
            OSScreenPutFontEx(SCREEN_DRC, 2, 8, "- Infinite loop in love.run()");
            OSScreenPutFontEx(SCREEN_DRC, 2, 9, "- Recursive function calls");
            OSScreenPutFontEx(SCREEN_DRC, 2, 10, "- Blocking operation");
            OSScreenPutFontEx(SCREEN_DRC, 2, 12, "Check: simple_debug.log");
            OSScreenPutFontEx(SCREEN_DRC, 2, 13, timeStr);
            OSScreenPutFontEx(SCREEN_DRC, 2, 15, "Press HOME to exit");
            
            // Flip buffers to display
            OSScreenFlipBuffersEx(SCREEN_TV);
            OSScreenFlipBuffersEx(SCREEN_DRC);
            
            FILE* successLog = fopen("fs:/vol/external01/simple_debug.log", "a");
            if (successLog) {
                fprintf(successLog, "Fallback diagnostic screen displayed successfully\n");
                fflush(successLog);
                fclose(successLog);
            }
            
            // Keep the diagnostic screen visible and enter a simple loop
            // that doesn't hang like the graphics loop
            while (true) {
                // Simple delay to prevent 100% CPU usage
                for (volatile int i = 0; i < 1000000; i++);
                
                // Refresh the diagnostic display periodically
                OSScreenFlipBuffersEx(SCREEN_TV);
                OSScreenFlipBuffersEx(SCREEN_DRC);
            }
        } else {
            FILE* errorLog = fopen("fs:/vol/external01/simple_debug.log", "a");
            if (errorLog) {
                fprintf(errorLog, "FAILED to allocate OSScreen buffers for diagnostic display\n");
                fflush(errorLog);
                fclose(errorLog);
            }
        }
#endif
    }

} // namespace love

// Global instance definition
love::GX2 love::gx2;
