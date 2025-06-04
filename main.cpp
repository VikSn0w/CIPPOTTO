//#include <SFML/Window.hpp>
#include "chip8.h"
#include "PlatformConsole.h"
#include "DebugConsoleManager.h"
#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char** argv)
{
    if (argc < 4 || argc > 5)
    {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM> [debug]\n";
        std::cerr << "  Scale: Display scale factor (not used in CMD version, but kept for compatibility)\n";
        std::cerr << "  Delay: Cycle delay in milliseconds (recommended: 1-10)\n";
        std::cerr << "  ROM: Path to the CHIP-8 ROM file\n";
        std::cerr << "  debug: Optional - add 'debug' to enable debug windows\n";
        std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    char const* romFilename = argv[3];
    bool enableDebug = (argc == 5 && std::string(argv[4]) == "debug");

    PlatformConsole platform("CHIP-8 Emulator (Command Line)",
                           VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale,
                           VIDEO_WIDTH, VIDEO_HEIGHT);

    chip8 chip8;
    chip8.LoadROM(romFilename);

    std::unique_ptr<DebugConsoleManager> debugManager;
    if (enableDebug) {
        debugManager = std::make_unique<DebugConsoleManager>();
        debugManager->StartDebugConsoles(&chip8);
        std::cout << "Debug consoles enabled! Check separate windows for debug info." << std::endl;
    }

    int videoPitch = sizeof(chip8.graphics[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    std::cout << "Starting emulation..." << std::endl;
    if (enableDebug) {
        std::cout << "Debug mode enabled - additional windows will show registers, memory, and stack info" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    while (!quit)
    {
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;

            chip8.emulateCycle();

            platform.Update(chip8.graphics, videoPitch);

            if (debugManager) {
                debugManager->UpdateDebugInfo();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    if (debugManager) {
        debugManager->StopDebugConsoles();
    }

    std::cout << "Emulation stopped. Goodbye!" << std::endl;
    return 0;
}