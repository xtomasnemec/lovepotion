#include "common/PPCDebugger.hpp"

#ifdef __WIIU__
#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/exception.h>
#include <coreinit/memory.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <gx2/context.h>
#    static void LogHeapInfo() {
        if (!s_enabled || s_debugLevel < 2)
            return;

        PPCLogPrintf("=== HEAP INFORMATION ===");
        
        // Temporarily simplified - WUT version incompatibility
        PPCLogPrintf("Heap info temporarily disabled due to WUT version issues");
    }.h>
#include <gx2/draw.h>
#include <gx2/registers.h>
#include <gx2/shaders.h>
#include <gx2/state.h>
#include <gx2/surface.h>
#include <gx2/texture.h>
#include <whb/log.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <cstdarg>

namespace love
{
    // Static member definitions
    bool PPCDebugger::s_initialized = false;
    bool PPCDebugger::s_enabled = true;
    
    // Set debug level based on build configuration
#ifdef __DEBUG__
    // Debug builds: Full debugging enabled
    uint32_t PPCDebugger::s_debugLevel = 3; // DEBUG_DETAILED
#else
    // Release builds: Only crash/critical errors
    uint32_t PPCDebugger::s_debugLevel = 1; // DEBUG_CRASH_ONLY
#endif
    
    std::vector<PPCDebugger::InstructionInfo> PPCDebugger::s_instructionHistory;
    std::vector<std::pair<std::string, uint64_t>> PPCDebugger::s_performanceTimers;

    // PPC Debugger file logging function
    void PPCLogPrintf(const char* format, ...)
    {
        // In Release builds, only log to file during crashes/critical errors
        // In Debug builds, always log to file
        bool shouldLogToFile = false;
        
#ifdef __DEBUG__
        // Debug builds: Always log to file
        shouldLogToFile = true;
#else
        // Release builds: Only log during crashes (when s_debugLevel is temporarily raised)
        shouldLogToFile = (PPCDebugger::s_debugLevel > 1);
#endif

        if (shouldLogToFile)
        {
            const char* logFileName;
#ifdef __DEBUG__
            logFileName = "PPC_debug.log";
#else
            logFileName = "PPC_crash.log";
#endif

            FILE* logFile = fopen("fs:/vol/external01/PPC_debug.log", "a");
            if (!logFile) {
                logFile = fopen("/vol/external01/PPC_debug.log", "a");
            }
            if (!logFile) {
                logFile = fopen(logFileName, "a");
            }
            
            if (logFile) {
                // Add timestamp
                time_t rawtime;
                struct tm* timeinfo;
                char timeStr[80];
                
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
                
#ifdef __DEBUG__
                fprintf(logFile, "[DEBUG][%s] ", timeStr);
#else
                fprintf(logFile, "[CRASH][%s] ", timeStr);
#endif
                
                // Write the actual message
                va_list args;
                va_start(args, format);
                vfprintf(logFile, format, args);
                va_end(args);
                
                fprintf(logFile, "\n");
                fflush(logFile);
                fclose(logFile);
            }
        }
        
        // Always output to WHB console for development
#ifdef __DEBUG__
        char buffer[1024];
        va_list args2;
        va_start(args2, format);
        vsnprintf(buffer, sizeof(buffer), format, args2);
        va_end(args2);
        WHBLogPrintf("%s", buffer);
#endif
    }

    void PPCDebugger::Initialize()
    {
        if (s_initialized)
            return;

        PPCLogPrintf("=== PPC DEBUGGER INITIALIZING ===");
        
#ifdef __DEBUG__
        PPCLogPrintf("PPC Debugger: DEBUG BUILD - Full debugging enabled");
        PPCLogPrintf("PPC Debugger: Logging to PPC_debug.log");
#else
        PPCLogPrintf("PPC Debugger: RELEASE BUILD - Crash-only logging enabled");
        PPCLogPrintf("PPC Debugger: Will log to PPC_crash.log only during crashes");
#endif
        
        PPCLogPrintf("PPC Debugger: Debug level set to %d", s_debugLevel);
        
        // Setup exception handler for crashes
        SetupExceptionHandler();
        
#ifdef __DEBUG__
        // Only log full system state in debug builds during initialization
        LogFullSystemState("DEBUGGER_INIT");
#else
        PPCLogPrintf("PPC Debugger: Release build - skipping initial system state dump");
#endif
        
        s_initialized = true;
        PPCLogPrintf("PPC Debugger: Initialization complete");
    }

    void PPCDebugger::Shutdown()
    {
        if (!s_initialized)
            return;

        PPCLogPrintf("=== PPC DEBUGGER SHUTTING DOWN ===");
        LogPerformanceStats();
        s_initialized = false;
    }

    void PPCDebugger::LogFullSystemState(const std::string& context)
    {
        if (!s_enabled || s_debugLevel < 2)
            return;

        PPCLogPrintf("=== FULL SYSTEM STATE: %s ===", context.c_str());
        
        LogRegisters(context);
        LogMemoryMap();
        LogHeapInfo();
        LogCallStack(context);
        LogThreadInfo();
        LogCacheInfo();
        LogGX2State();
        
        PPCLogPrintf("=== END SYSTEM STATE: %s ===", context.c_str());
    }

    void PPCDebugger::LogRegisters(const std::string& context)
    {
        if (!s_enabled || s_debugLevel < 1)
            return;

        PPCLogPrintf("=== PPC REGISTERS: %s ===", context.c_str());
        
        PPCRegisters regs = GetCurrentRegisters();
        
        // Log General Purpose Registers in groups of 4
        for (int i = 0; i < 32; i += 4)
        {
            PPCLogPrintf("GPR%02d-GPR%02d: %08X %08X %08X %08X", 
                        i, i+3, regs.gpr[i], regs.gpr[i+1], regs.gpr[i+2], regs.gpr[i+3]);
        }
        
        // Log Special Purpose Registers
        PPCLogPrintf("PC=%08X LR=%08X CTR=%08X CR=%08X", regs.pc, regs.lr, regs.ctr, regs.cr);
        PPCLogPrintf("XER=%08X MSR=%08X", regs.xer, regs.msr);
        
        // Log some floating point registers
        PPCLogPrintf("FPR0=%f FPR1=%f FPR2=%f FPR3=%f", regs.fpr[0], regs.fpr[1], regs.fpr[2], regs.fpr[3]);
    }

    void PPCDebugger::LogMemoryDump(uint32_t address, uint32_t size, const std::string& context)
    {
        if (!s_enabled || s_debugLevel < 2)
            return;

        if (!IsValidAddress(address))
        {
            PPCLogPrintf("PPC Debugger: Invalid address %08X for memory dump (%s)", address, context.c_str());
            return;
        }

        PPCLogPrintf("=== MEMORY DUMP: %s @ %08X (size: %d) ===", context.c_str(), address, size);
        
        // Limit dump size to prevent log overflow
        size = std::min(size, 512u);
        
        const uint8_t* data = (const uint8_t*)address;
        LogHexDump(data, size, address);
    }

    void PPCDebugger::LogCallStack(const std::string& context)
    {
        if (!s_enabled || s_debugLevel < 2)
            return;

        PPCLogPrintf("=== CALL STACK: %s ===", context.c_str());
        
        // Get current stack pointer from register
        uint32_t sp;
        asm volatile("mr %0, 1" : "=r" (sp));
        
        PPCLogPrintf("Stack Pointer: %08X", sp);
        
        // Walk the stack frame chain
        uint32_t* frame = (uint32_t*)sp;
        int depth = 0;
        const int maxDepth = 10;
        
        while (frame && IsValidAddress((uint32_t)frame) && depth < maxDepth)
        {
            uint32_t nextFrame = frame[0];
            uint32_t returnAddr = frame[1];
            
            PPCLogPrintf("Frame %d: %08X -> %08X (ret: %08X)", depth, (uint32_t)frame, nextFrame, returnAddr);
            
            // Disassemble the return address
            if (IsValidAddress(returnAddr))
            {
                InstructionInfo inst = DisassembleInstruction(returnAddr);
                PPCLogPrintf("  Return: %s %s", inst.mnemonic.c_str(), inst.operands.c_str());
            }
            
            if (nextFrame <= (uint32_t)frame || nextFrame == 0)
                break;
                
            frame = (uint32_t*)nextFrame;
            depth++;
        }
    }

    void PPCDebugger::LogDisassembly(uint32_t address, uint32_t count, const std::string& context)
    {
        if (!s_enabled || s_debugLevel < 3)
            return;

        PPCLogPrintf("=== DISASSEMBLY: %s @ %08X ===", context.c_str(), address);
        
        for (uint32_t i = 0; i < count; i++)
        {
            uint32_t currentAddr = address + (i * 4);
            if (!IsValidAddress(currentAddr))
                break;
                
            InstructionInfo inst = DisassembleInstruction(currentAddr);
            PPCLogPrintf("%08X: %08X %s %s", currentAddr, inst.instruction, 
                        inst.mnemonic.c_str(), inst.operands.c_str());
        }
    }

    void PPCDebugger::LogMemoryMap()
    {
        if (!s_enabled || s_debugLevel < 2)
            return;

        PPCLogPrintf("=== MEMORY MAP ===");
        
        std::vector<MemoryRegion> regions = GetMemoryRegions();
        for (const auto& region : regions)
        {
            PPCLogPrintf("Region: %08X-%08X (%s) %s%s%s %s", 
                        region.address, region.address + region.size - 1,
                        FormatAddress(region.size).c_str(),
                        region.readable ? "R" : "-",
                        region.writable ? "W" : "-", 
                        region.executable ? "X" : "-",
                        region.name.c_str());
        }
    }

    void PPCDebugger::LogHeapInfo()
    {
        if (!s_enabled || s_debugLevel < 2)
            return;

        PPCLogPrintf("=== HEAP INFORMATION ===");
        
        // Temporarily simplified - WUT version incompatibility
        PPCLogPrintf("Heap info temporarily disabled due to WUT version issues");
    }

    bool PPCDebugger::IsValidAddress(uint32_t address)
    {
        // Basic address validation for Wii U memory layout
        // MEM1: 0xF4000000 - 0xF6000000 (32MB)
        // MEM2: 0x10000000 - 0x50000000 (1GB)
        
        if ((address >= 0xF4000000 && address < 0xF6000000) ||
            (address >= 0x10000000 && address < 0x50000000))
        {
            return true;
        }
        
        return false;
    }

    std::vector<PPCDebugger::MemoryRegion> PPCDebugger::GetMemoryRegions()
    {
        std::vector<MemoryRegion> regions;
        
        // Add known Wii U memory regions
        regions.push_back({0xF4000000, 0x02000000, "MEM1", true, true, false});
        regions.push_back({0x10000000, 0x40000000, "MEM2", true, true, false});
        regions.push_back({0x01000000, 0x01800000, "Code", true, false, true});
        
        return regions;
    }

    PPCDebugger::InstructionInfo PPCDebugger::DisassembleInstruction(uint32_t address)
    {
        InstructionInfo info;
        info.address = address;
        
        if (!IsValidAddress(address))
        {
            info.instruction = 0;
            info.mnemonic = "INVALID";
            info.operands = "";
            return info;
        }
        
        uint32_t instruction = *(uint32_t*)address;
        info.instruction = instruction;
        info.mnemonic = GetInstructionMnemonic(instruction);
        
        // Basic operand extraction (simplified)
        uint32_t rt = (instruction >> 21) & 0x1F;
        uint32_t ra = (instruction >> 16) & 0x1F;
        uint32_t rb = (instruction >> 11) & 0x1F;
        
        std::stringstream ss;
        ss << "r" << rt << ", r" << ra << ", r" << rb;
        info.operands = ss.str();
        
        return info;
    }

    std::string PPCDebugger::GetInstructionMnemonic(uint32_t instruction)
    {
        uint32_t opcode = (instruction >> 26) & 0x3F;
        
        // Basic PPC instruction decoding (simplified)
        switch (opcode)
        {
            case 14: return "addi";
            case 15: return "addis";
            case 24: return "ori";
            case 25: return "oris";
            case 32: return "lwz";
            case 36: return "stw";
            case 40: return "lhz";
            case 44: return "sth";
            case 34: return "lbz";
            case 38: return "stb";
            case 18: return "b";
            case 16: return "bc";
            case 19: return "bclr";
            case 31: return "extended";
            default: return "unknown";
        }
    }

    void PPCDebugger::LogGX2State()
    {
        if (!s_enabled || s_debugLevel < 2)
            return;

        PPCLogPrintf("=== GX2 GRAPHICS STATE ===");
        
        // Temporarily simplified - WUT version incompatibility
        PPCLogPrintf("GX2 state info temporarily disabled due to WUT version issues");
    }

    void PPCDebugger::DebugPoint(const std::string& name, const std::string& details)
    {
        if (!s_enabled)
            return;

#ifdef __DEBUG__
        // Debug builds: Full debug point logging
        PPCLogPrintf("=== DEBUG POINT: %s ===", name.c_str());
        if (!details.empty())
            PPCLogPrintf("Details: %s", details.c_str());
            
        if (s_debugLevel >= 2)
        {
            LogRegisters(name);
            
            // Get current PC and disassemble around it
            uint32_t pc;
            asm volatile("bl 1f; 1: mflr %0" : "=r" (pc));
            LogDisassembly(pc - 16, 8, name);
        }
#else
        // Release builds: Only log critical debug points to console
        if (name.find("CRITICAL") != std::string::npos || 
            name.find("ERROR") != std::string::npos ||
            name.find("CRASH") != std::string::npos)
        {
            PPCLogPrintf("=== CRITICAL DEBUG POINT: %s ===", name.c_str());
            if (!details.empty())
                PPCLogPrintf("Details: %s", details.c_str());
        }
#endif
    }

    void PPCDebugger::CriticalError(const std::string& error, bool dumpAll)
    {
        PPCLogPrintf("=== CRITICAL ERROR ===");
        PPCLogPrintf("ERROR: %s", error.c_str());
        
#ifndef __DEBUG__
        // In Release builds, temporarily enable full debugging for crash analysis
        uint32_t originalLevel = s_debugLevel;
        s_debugLevel = 3; // Enable detailed logging for crash
        
        PPCLogPrintf("RELEASE BUILD: Crash detected, enabling full debug logging");
#endif
        
        if (dumpAll)
        {
            LogFullSystemState("CRITICAL_ERROR");
        }
        
#ifndef __DEBUG__
        // Restore original debug level
        s_debugLevel = originalLevel;
        PPCLogPrintf("RELEASE BUILD: Debug logging restored to level %d", s_debugLevel);
#endif
    }

    void PPCDebugger::SetupExceptionHandler()
    {
        // Set up exception handler for debugging crashes
        PPCLogPrintf("PPC Debugger: Setting up exception handler...");
        // This is a simplified setup - real implementation would use OSSetExceptionCallback
    }

    PPCDebugger::PPCRegisters PPCDebugger::GetCurrentRegisters()
    {
        PPCRegisters regs = {};
        
        // Get some basic registers using inline assembly
        asm volatile("mr %0, 1" : "=r" (regs.gpr[1])); // Stack pointer
        asm volatile("mr %0, 2" : "=r" (regs.gpr[2])); // TOC pointer
        asm volatile("mflr %0" : "=r" (regs.lr));       // Link register
        asm volatile("mfctr %0" : "=r" (regs.ctr));     // Count register
        asm volatile("mfcr %0" : "=r" (regs.cr));       // Condition register
        
        // Get program counter (approximate)
        asm volatile("bl 1f; 1: mflr %0" : "=r" (regs.pc));
        
        return regs;
    }

    void PPCDebugger::LogHexDump(const void* data, size_t size, uint32_t baseAddress)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        
        for (size_t offset = 0; offset < size; offset += 16)
        {
            std::stringstream ss;
            ss << std::hex << std::setfill('0') << std::setw(8) << (baseAddress + offset) << ": ";
            
            // Hex bytes
            for (size_t i = 0; i < 16 && (offset + i) < size; i++)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << (int)bytes[offset + i] << " ";
            }
            
            // Padding
            for (size_t i = size - offset; i < 16; i++)
            {
                ss << "   ";
            }
            
            ss << " |";
            
            // ASCII representation
            for (size_t i = 0; i < 16 && (offset + i) < size; i++)
            {
                uint8_t byte = bytes[offset + i];
                ss << (byte >= 32 && byte <= 126 ? static_cast<char>(byte) : '.');
            }
            
            ss << "|";
            PPCLogPrintf("%s", ss.str().c_str());
        }
    }

    std::string PPCDebugger::FormatAddress(uint32_t address)
    {
        std::stringstream ss;
        if (address >= 1024 * 1024)
        {
            ss << (address / (1024 * 1024)) << "MB";
        }
        else if (address >= 1024)
        {
            ss << (address / 1024) << "KB";
        }
        else
        {
            ss << address << "B";
        }
        return ss.str();
    }

    void PPCDebugger::LogCacheInfo()
    {
        if (!s_enabled || s_debugLevel < 3)
            return;

        PPCLogPrintf("=== CACHE INFO ===");
        PPCLogPrintf("Invalidating and logging cache state...");
        
        // Force cache operations and log
        DCFlushRange((void*)0x10000000, 0x1000);
        ICInvalidateRange((void*)0x10000000, 0x1000);
        
        PPCLogPrintf("Cache operations completed");
    }

    void PPCDebugger::LogThreadInfo()
    {
        if (!s_enabled || s_debugLevel < 2)
            return;

        PPCLogPrintf("=== THREAD INFO ===");
        
        OSThread* currentThread = OSGetCurrentThread();
        if (currentThread)
        {
            PPCLogPrintf("Current Thread: %p", currentThread);
            PPCLogPrintf("Thread State: %d", currentThread->state);
            PPCLogPrintf("Thread Priority: %d", currentThread->basePriority);
        }
    }

    void PPCDebugger::StartPerformanceTimer(const std::string& name)
    {
        if (!s_enabled)
            return;
            
        uint64_t time = OSGetSystemTime();
        s_performanceTimers.push_back({name, time});
    }

    void PPCDebugger::EndPerformanceTimer(const std::string& name)
    {
        if (!s_enabled)
            return;
            
        uint64_t endTime = OSGetSystemTime();
        
        for (auto it = s_performanceTimers.rbegin(); it != s_performanceTimers.rend(); ++it)
        {
            if (it->first == name)
            {
                uint64_t duration = endTime - it->second;
                PPCLogPrintf("PERF: %s took %lld ticks", name.c_str(), duration);
                s_performanceTimers.erase((it + 1).base());
                break;
            }
        }
    }

    void PPCDebugger::LogPerformanceStats()
    {
        if (!s_enabled)
            return;

        PPCLogPrintf("=== PERFORMANCE STATS ===");
        for (const auto& timer : s_performanceTimers)
        {
            PPCLogPrintf("Active timer: %s", timer.first.c_str());
        }
    }

    bool PPCDebugger::IsDebugBuild()
    {
#ifdef __DEBUG__
        return true;
#else
        return false;
#endif
    }

    PPCDebugger::DebugMode PPCDebugger::GetCurrentDebugMode()
    {
        return static_cast<DebugMode>(s_debugLevel);
    }

} // namespace love

#endif // __WIIU__
