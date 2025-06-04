#include "DebugConsoleManager.h"
#include <iomanip>
#include <sstream>

DebugConsoleManager::DebugConsoleManager() 
    : chip8Ptr(nullptr), running(false) {
#ifdef _WIN32
    registersConsole = nullptr;
    memoryConsole = nullptr;
    stackConsole = nullptr;
    registersFile = nullptr;
    memoryFile = nullptr;
    stackFile = nullptr;
#endif
}

DebugConsoleManager::~DebugConsoleManager() {
    StopDebugConsoles();
}

void DebugConsoleManager::StartDebugConsoles(chip8* emulator) {
    chip8Ptr = emulator;
    running = true;

#ifdef _WIN32
    // Create a single large debug console
    AllocConsole();
    registersConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitle("CHIP-8 Debug Console");

    // Set console to CP437 (Extended ASCII) for proper box drawing characters
    SetConsoleOutputCP(437);
    SetConsoleCP(437);

    // Redirect stdout to this console
    freopen_s(&registersFile, "CONOUT$", "w", stdout);

    // Set a larger console size to accommodate all debug info
    COORD bufferSize = {120, 50};
    SetConsoleScreenBufferSize(registersConsole, bufferSize);

    SMALL_RECT windowSize = {0, 0, 119, 49};
    SetConsoleWindowInfo(registersConsole, TRUE, &windowSize);

    // Position the debug window
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow) {
        SetWindowPos(consoleWindow, 0, 100, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
#endif

    // Start a single debug thread that displays all information
    registersThread = std::thread(&DebugConsoleManager::UnifiedDebugLoop, this);
}

void DebugConsoleManager::StopDebugConsoles() {
    running = false;

    if (registersThread.joinable()) registersThread.join();

#ifdef _WIN32
    if (registersFile) {
        fclose(registersFile);
        registersFile = nullptr;
    }
    FreeConsole();
#endif
}

void DebugConsoleManager::UnifiedDebugLoop() {
    while (running) {
        if (chip8Ptr) {
            std::lock_guard<std::mutex> lock(dataMutex);

#ifdef _WIN32
            if (registersFile) {
                // Clear screen
                fprintf(registersFile, "\033[2J\033[H");

                // Title - using CP437 characters
                putc(201, registersFile); // ╔
                for (int i = 0; i < 110; i++) putc(205, registersFile); // ═
                putc(187, registersFile); // ╗
                putc('\n', registersFile);

                fprintf(registersFile, "%c                                          CHIP-8 DEBUG CONSOLE                                                   %c\n", 186, 186); // ║

                putc(200, registersFile); // ╚
                for (int i = 0; i < 110; i++) putc(205, registersFile); // ═
                putc(188, registersFile); // ╝
                putc('\n', registersFile);

                // Registers Section
                putc(218, registersFile); // ┌
                putc(196, registersFile); putc(196, registersFile); putc(196, registersFile); // ───
                fprintf(registersFile, " REGISTERS ");
                for (int i = 0; i < 75; i++) putc(196, registersFile); // ─
                putc(191, registersFile); // ┐
                putc('\n', registersFile);

                fprintf(registersFile, "%c PC: 0x%04X    I: 0x%04X    SP: %02X    Opcode: 0x%04X                                    %c\n", 179,
                       chip8Ptr->program_counter, chip8Ptr->index_register, chip8Ptr->stack_pointer, chip8Ptr->opcode, 179);
                fprintf(registersFile, "%c                                                                                          %c\n", 179, 179);
                fprintf(registersFile, "%c Registers V0-VF:                          Timers:                                       %c\n", 179, 179);
                for (int i = 0; i < 16; i += 4) {
                    if (i == 0) {
                        fprintf(registersFile, "%c V%X: %02X  V%X: %02X  V%X: %02X  V%X: %02X             Delay: %02X                                        %c\n", 179,
                               i, chip8Ptr->registers_V[i], i+1, chip8Ptr->registers_V[i+1],
                               i+2, chip8Ptr->registers_V[i+2], i+3, chip8Ptr->registers_V[i+3], chip8Ptr->delay_timer, 179);
                    } else if (i == 4) {
                        fprintf(registersFile, "%c V%X: %02X  V%X: %02X  V%X: %02X  V%X: %02X             Sound: %02X                                        %c\n", 179,
                               i, chip8Ptr->registers_V[i], i+1, chip8Ptr->registers_V[i+1],
                               i+2, chip8Ptr->registers_V[i+2], i+3, chip8Ptr->registers_V[i+3], chip8Ptr->sound_timer, 179);
                    } else {
                        fprintf(registersFile, "%c V%X: %02X  V%X: %02X  V%X: %02X  V%X: %02X                                                             %c\n", 179,
                               i, chip8Ptr->registers_V[i], i+1, chip8Ptr->registers_V[i+1],
                               i+2, chip8Ptr->registers_V[i+2], i+3, chip8Ptr->registers_V[i+3], 179);
                    }
                }
                putc(192, registersFile); // └
                for (int i = 0; i < 90; i++) putc(196, registersFile); // ─
                putc(217, registersFile); // ┘
                putc('\n', registersFile);

                // Memory Section
                putc('\n', registersFile);
                putc(218, registersFile); // ┌
                putc(196, registersFile); putc(196, registersFile); putc(196, registersFile); // ───
                fprintf(registersFile, " MEMORY AROUND PC ");
                for (int i = 0; i < 70; i++) putc(196, registersFile); // ─
                putc(191, registersFile); // ┐
                putc('\n', registersFile);

                uint16_t pc = chip8Ptr->program_counter;
                uint16_t start = (pc >= 16) ? pc - 16 : 0;
                uint16_t end = std::min(4096, (int)(pc + 24));

                for (uint16_t addr = start; addr < end; addr += 8) {
                    putc(179, registersFile); // │
                    if (addr == pc) {
                        fprintf(registersFile, " -> ");
                    } else {
                        fprintf(registersFile, "    ");
                    }

                    fprintf(registersFile, "%04X: ", addr);
                    for (int i = 0; i < 8 && addr + i < end; ++i) {
                        fprintf(registersFile, "%02X ", chip8Ptr->memory[addr + i]);
                    }

                    // Pad to align the right border
                    int remaining = 8 - std::min(8, (int)(end - addr));
                    for (int i = 0; i < remaining; ++i) {
                        fprintf(registersFile, "   ");
                    }
                    fprintf(registersFile, "                                       ");
                    putc(179, registersFile); // │
                    putc('\n', registersFile);
                }
                putc(192, registersFile); // └
                for (int i = 0; i < 90; i++) putc(196, registersFile); // ─
                putc(217, registersFile); // ┘
                putc('\n', registersFile);

                // Stack Section
                putc('\n', registersFile);
                putc(218, registersFile); // ┌
                putc(196, registersFile); putc(196, registersFile); putc(196, registersFile); // ───
                fprintf(registersFile, " STACK ");
                for (int i = 0; i < 81; i++) putc(196, registersFile); // ─
                putc(191, registersFile); // ┐
                putc('\n', registersFile);

                fprintf(registersFile, "%c Stack Pointer: %d                                                                       %c\n", 179, chip8Ptr->stack_pointer, 179);
                fprintf(registersFile, "%c                                                                                          %c\n", 179, 179);

                // Show only the relevant part of the stack
                int stackStart = std::max(0, (int)chip8Ptr->stack_pointer - 4);
                int stackEnd = std::min(16, (int)chip8Ptr->stack_pointer + 4);

                for (int i = stackEnd - 1; i >= stackStart; --i) {
                    putc(179, registersFile); // │
                    if (i == chip8Ptr->stack_pointer - 1 && chip8Ptr->stack_pointer > 0) {
                        fprintf(registersFile, " -> [%02d]: 0x%04X", i, chip8Ptr->stack[i]);
                    } else if (i < chip8Ptr->stack_pointer) {
                        fprintf(registersFile, "    [%02d]: 0x%04X", i, chip8Ptr->stack[i]);
                    } else {
                        fprintf(registersFile, "    [%02d]: ----", i);
                    }
                    fprintf(registersFile, "                                                                       ");
                    putc(179, registersFile); // │
                    putc('\n', registersFile);
                }
                putc(192, registersFile); // └
                for (int i = 0; i < 90; i++) putc(196, registersFile); // ─
                putc(217, registersFile); // ┘
                putc('\n', registersFile);

                // Keypad Section
                putc('\n', registersFile);
                putc(218, registersFile); // ┌
                putc(196, registersFile); putc(196, registersFile); putc(196, registersFile); // ───
                fprintf(registersFile, " KEYPAD STATE ");
                for (int i = 0; i < 74; i++) putc(196, registersFile); // ─
                putc(191, registersFile); // ┐
                putc('\n', registersFile);

                fprintf(registersFile, "%c CHIP-8 Keypad Layout:                    Current State:                                 %c\n", 179, 179);
                fprintf(registersFile, "%c                                                                                          %c\n", 179, 179);
                fprintf(registersFile, "%c [1] [2] [3] [C]                          1:%s 2:%s 3:%s C:%s                          %c\n", 179,
                       chip8Ptr->keypad[1] ? "ON " : "OFF", chip8Ptr->keypad[2] ? "ON " : "OFF",
                       chip8Ptr->keypad[3] ? "ON " : "OFF", chip8Ptr->keypad[0xC] ? "ON " : "OFF", 179);
                fprintf(registersFile, "%c [4] [5] [6] [D]                          4:%s 5:%s 6:%s D:%s                          %c\n", 179,
                       chip8Ptr->keypad[4] ? "ON " : "OFF", chip8Ptr->keypad[5] ? "ON " : "OFF",
                       chip8Ptr->keypad[6] ? "ON " : "OFF", chip8Ptr->keypad[0xD] ? "ON " : "OFF", 179);
                fprintf(registersFile, "%c [7] [8] [9] [E]                          7:%s 8:%s 9:%s E:%s                          %c\n", 179,
                       chip8Ptr->keypad[7] ? "ON " : "OFF", chip8Ptr->keypad[8] ? "ON " : "OFF",
                       chip8Ptr->keypad[9] ? "ON " : "OFF", chip8Ptr->keypad[0xE] ? "ON " : "OFF", 179);
                fprintf(registersFile, "%c [A] [0] [B] [F]                          A:%s 0:%s B:%s F:%s                          %c\n", 179,
                       chip8Ptr->keypad[0xA] ? "ON " : "OFF", chip8Ptr->keypad[0] ? "ON " : "OFF",
                       chip8Ptr->keypad[0xB] ? "ON " : "OFF", chip8Ptr->keypad[0xF] ? "ON " : "OFF", 179);
                putc(192, registersFile); // └
                for (int i = 0; i < 90; i++) putc(196, registersFile); // ─
                putc(217, registersFile); // ┘
                putc('\n', registersFile);

                fprintf(registersFile, "\nPress ESC in main window to quit...\n");

                fflush(registersFile);
            }
#endif
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Dummy implementations for the methods that are no longer used
void DebugConsoleManager::RegistersConsoleLoop() {}
void DebugConsoleManager::MemoryConsoleLoop() {}
void DebugConsoleManager::StackConsoleLoop() {}

#ifdef _WIN32
void DebugConsoleManager::CreateConsoleWindow(const char* title, HANDLE& consoleHandle, FILE*& fileHandle) {}
void DebugConsoleManager::SetConsolePosition(HANDLE console, int x, int y) {}
void DebugConsoleManager::ClearConsole(FILE* console) {}
#endif

void DebugConsoleManager::UpdateDebugInfo() {
    // This method can be called from the main loop for manual updates
    // Currently, the thread handles automatic updates
}