#ifndef DEBUG_CONSOLE_MANAGER_H
#define DEBUG_CONSOLE_MANAGER_H

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include "chip8.h"

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#endif

class DebugConsoleManager {
public:
    DebugConsoleManager();
    ~DebugConsoleManager();
    
    void StartDebugConsoles(chip8* emulator);
    void StopDebugConsoles();
    void UpdateDebugInfo();

private:
    chip8* chip8Ptr;
    std::atomic<bool> running;
    std::thread registersThread;
    std::thread memoryThread;
    std::thread stackThread;
    std::mutex dataMutex;
    
#ifdef _WIN32
    HANDLE registersConsole;
    HANDLE memoryConsole;
    HANDLE stackConsole;
    FILE* registersFile;
    FILE* memoryFile;
    FILE* stackFile;
#endif

    void RegistersConsoleLoop();
    void MemoryConsoleLoop();
    void StackConsoleLoop();
    void UnifiedDebugLoop();  // New method for unified debug display

    void CreateConsoleWindow(const char* title, HANDLE& consoleHandle, FILE*& fileHandle);
    void ClearConsole(FILE* console);
    void SetConsolePosition(HANDLE console, int x, int y);
};

#endif // DEBUG_CONSOLE_MANAGER_H