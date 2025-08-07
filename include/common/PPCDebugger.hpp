#ifndef LOVE_PPC_DEBUGGER_HPP
#define LOVE_PPC_DEBUGGER_HPP

#ifdef __WIIU__
#include <coreinit/debug.h>
#include <coreinit/memory.h>
#include <coreinit/thread.h>
#include <coreinit/cache.h>
#include <whb/log.h>
#include <string>
#include <vector>

namespace love
{
    /**
     * Advanced PPC (PowerPC) debugger for deep-level Wii U debugging
     * Similar to CEMU's debugging capabilities but runtime-integrated
     * 
     * Debug builds: Full logging enabled
     * Release builds: Only crash/critical error logging
     */
    class PPCDebugger
    {
    public:
        enum DebugMode
        {
            DEBUG_OFF = 0,           // No debugging
            DEBUG_CRASH_ONLY = 1,    // Only crash/critical errors (Release builds)
            DEBUG_BASIC = 2,         // Basic debugging
            DEBUG_DETAILED = 3,      // Detailed debugging (Debug builds)
            DEBUG_EVERYTHING = 4     // Everything including disassembly (Debug builds)
        };
        struct PPCRegisters
        {
            uint32_t gpr[32];        // General Purpose Registers
            uint32_t cr;             // Condition Register
            uint32_t lr;             // Link Register
            uint32_t ctr;            // Count Register
            uint32_t xer;            // Exception Register
            uint32_t msr;            // Machine State Register
            uint32_t pc;             // Program Counter
            double fpr[32];          // Floating Point Registers
        };

        struct MemoryRegion
        {
            uint32_t address;
            uint32_t size;
            std::string name;
            bool readable;
            bool writable;
            bool executable;
        };

        struct InstructionInfo
        {
            uint32_t address;
            uint32_t instruction;
            std::string mnemonic;
            std::string operands;
            std::string description;
        };

        static void Initialize();
        static void Shutdown();
        
        // Core debugging functions
        static void LogFullSystemState(const std::string& context);
        static void LogRegisters(const std::string& context);
        static void LogMemoryDump(uint32_t address, uint32_t size, const std::string& context);
        static void LogCallStack(const std::string& context);
        static void LogDisassembly(uint32_t address, uint32_t count, const std::string& context);
        
        // Memory analysis
        static void LogMemoryMap();
        static void LogHeapInfo();
        static bool IsValidAddress(uint32_t address);
        static std::vector<MemoryRegion> GetMemoryRegions();
        
        // Instruction analysis
        static InstructionInfo DisassembleInstruction(uint32_t address);
        static std::string GetInstructionMnemonic(uint32_t instruction);
        static void LogInstructionHistory(int count = 10);
        
        // Critical points debugging
        static void DebugPoint(const std::string& name, const std::string& details = "");
        static void CriticalError(const std::string& error, bool dumpAll = true);
        
        // Build configuration helpers
        static bool IsDebugBuild();
        static DebugMode GetCurrentDebugMode();
        
        // Graphics-specific debugging
        static void LogGX2State();
        static void LogShaderInfo();
        static void LogTextureInfo();
        static void LogFramebufferInfo();
        
        // Performance monitoring
        static void StartPerformanceTimer(const std::string& name);
        static void EndPerformanceTimer(const std::string& name);
        static void LogPerformanceStats();
        
        // Exception handling
        static void SetupExceptionHandler();
        static void OnException(OSContext* context);
        
    private:
        static bool s_initialized;
        static bool s_enabled;
        static uint32_t s_debugLevel;
        static std::vector<InstructionInfo> s_instructionHistory;
        static std::vector<std::pair<std::string, uint64_t>> s_performanceTimers;
        
        // Internal helpers
        static PPCRegisters GetCurrentRegisters();
        static void LogHexDump(const void* data, size_t size, uint32_t baseAddress);
        static std::string FormatAddress(uint32_t address);
        static std::string FormatInstruction(uint32_t instruction);
        static void LogCacheInfo();
        static void LogThreadInfo();
    };

} // namespace love

#endif // __WIIU__
#endif // LOVE_PPC_DEBUGGER_HPP
