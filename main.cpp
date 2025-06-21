#include "chip8.h"
#include "PlatformSDL.h"
#include "DebugSDL.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <cstring>

void showSplashScreen() {
    // Initialize SDL for splash screen
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Initialize TTF
    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize! TTF_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    // Create splash screen window
    SDL_Window* splashWindow = SDL_CreateWindow("CIPPOTTO v2.1",
                                               500, 350,
                                               SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP);

    if (!splashWindow) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    SDL_Renderer* splashRenderer = SDL_CreateRenderer(splashWindow, NULL);
    if (!splashRenderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(splashWindow);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Load fonts - try system fonts first, fallback to built-in if not available
    TTF_Font* titleFont = nullptr;
    TTF_Font* authorFont = nullptr;

    // Try to load system fonts (common locations)
    const char* fontPaths[] = {
        "/System/Library/Fonts/Arial.ttf",           // macOS
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", // Linux
        "C:\\Windows\\Fonts\\arial.ttf",             // Windows
        "/usr/share/fonts/TTF/arial.ttf",            // Arch Linux
        nullptr
    };

    for (int i = 0; fontPaths[i] != nullptr; i++) {
        titleFont = TTF_OpenFont(fontPaths[i], 36);
        if (titleFont) {
            authorFont = TTF_OpenFont(fontPaths[i], 18);
            break;
        }
    }

    // If no system font found, we'll create a simple fallback
    bool useFallback = (titleFont == nullptr);

    // Colors
    SDL_Color bgColor = {20, 20, 30, 255};      // Dark blue background
    SDL_Color titleColor = {255, 255, 255, 255}; // White text
    SDL_Color authorColor = {180, 180, 180, 255}; // Light gray text

    // Create text surfaces and textures
    SDL_Surface* titleSurface = nullptr;
    SDL_Surface* authorSurface = nullptr;
    SDL_Texture* titleTexture = nullptr;
    SDL_Texture* authorTexture = nullptr;

    if (!useFallback) {
        // SDL3 TTF_RenderText_Solid requires length parameter
        const char* titleText = "CIPPOTTO v2.1";
        const char* authorText = "by VikSn0w";

        titleSurface = TTF_RenderText_Solid(titleFont, titleText, strlen(titleText), titleColor);
        authorSurface = TTF_RenderText_Solid(authorFont, authorText, strlen(authorText), authorColor);

        if (titleSurface) {
            titleTexture = SDL_CreateTextureFromSurface(splashRenderer, titleSurface);
        }
        if (authorSurface) {
            authorTexture = SDL_CreateTextureFromSurface(splashRenderer, authorSurface);
        }
    }

    auto splashStartTime = std::chrono::high_resolution_clock::now();
    const int splashDuration = 2500; // 2.5 seconds

    while (true) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - splashStartTime).count();

        if (elapsed >= splashDuration) {
            break;
        }

        // Handle events to prevent window from appearing unresponsive
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_KEY_DOWN) {
                goto splash_end; // Exit splash early if user presses key or closes
            }
        }

        // Clear screen with background color
        SDL_SetRenderDrawColor(splashRenderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(splashRenderer);

        // Draw a decorative border
        SDL_SetRenderDrawColor(splashRenderer, 60, 60, 100, 255);
        SDL_FRect outerBorder = {10, 10, 480, 330};
        SDL_RenderRect(splashRenderer, &outerBorder);

        SDL_FRect innerBorder = {15, 15, 470, 320};
        SDL_RenderRect(splashRenderer, &innerBorder);

        if (!useFallback && titleTexture && authorTexture) {
            // Render text using TTF

            // Get text dimensions - SDL3 uses SDL_GetTextureSize with float parameters
            float titleW, titleH, authorW, authorH;
            SDL_GetTextureSize(titleTexture, &titleW, &titleH);
            SDL_GetTextureSize(authorTexture, &authorW, &authorH);

            // Center the title
            SDL_FRect titleRect = {
                (500 - titleW) / 2.0f,
                120,
                titleW,
                titleH
            };

            // Center the author text
            SDL_FRect authorRect = {
                (500 - authorW) / 2.0f,
                180,
                authorW,
                authorH
            };

            // Add a subtle glow effect
            SDL_SetRenderDrawColor(splashRenderer, 40, 40, 80, 100);
            SDL_FRect glowRect = {titleRect.x - 5, titleRect.y - 5, titleRect.w + 10, titleRect.h + 10};
            SDL_RenderFillRect(splashRenderer, &glowRect);

            SDL_RenderTexture(splashRenderer, titleTexture, NULL, &titleRect);
            SDL_RenderTexture(splashRenderer, authorTexture, NULL, &authorRect);

        } else {
            // Fallback: Draw simple text using rectangles
            SDL_SetRenderDrawColor(splashRenderer, titleColor.r, titleColor.g, titleColor.b, titleColor.a);

            // Draw "CIPPOTTO v2.1" as simple blocks
            const char* title = "CIPPOTTO v2.1";
            float startX = 80;
            float startY = 120;

            for (int i = 0; title[i] != '\0'; i++) {
                if (title[i] != ' ') {
                    SDL_FRect charRect = {startX + i * 22, startY, 18, 30};
                    SDL_RenderFillRect(splashRenderer, &charRect);
                }
            }

            // Draw "by VikSn0w"
            SDL_SetRenderDrawColor(splashRenderer, authorColor.r, authorColor.g, authorColor.b, authorColor.a);
            const char* author = "by VikSn0w";
            startX = 150;
            startY = 180;

            for (int i = 0; author[i] != '\0'; i++) {
                if (author[i] != ' ') {
                    SDL_FRect charRect = {startX + i * 18, startY, 14, 20};
                    SDL_RenderFillRect(splashRenderer, &charRect);
                }
            }
        }

        // Add a loading bar effect
        float progress = (float)elapsed / splashDuration;

        // Loading bar background
        SDL_SetRenderDrawColor(splashRenderer, 40, 40, 40, 255);
        SDL_FRect progressBg = {50, 270, 400, 12};
        SDL_RenderFillRect(splashRenderer, &progressBg);

        // Loading bar border
        SDL_SetRenderDrawColor(splashRenderer, 100, 100, 100, 255);
        SDL_RenderRect(splashRenderer, &progressBg);

        // Loading bar fill
        SDL_SetRenderDrawColor(splashRenderer, 100, 200, 100, 255);
        SDL_FRect progressBar = {52, 272, 396 * progress, 8};
        SDL_RenderFillRect(splashRenderer, &progressBar);

        // Add some decorative elements - fix narrowing conversion warning
        SDL_SetRenderDrawColor(splashRenderer, 80, 120, 160, 255);
        for (int i = 0; i < 5; i++) {
            SDL_FRect dot = {100.0f + i * 80.0f, 50, 6, 6};
            SDL_RenderFillRect(splashRenderer, &dot);
        }

        SDL_RenderPresent(splashRenderer);
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

splash_end:
    // Clean up text resources
    if (titleTexture) SDL_DestroyTexture(titleTexture);
    if (authorTexture) SDL_DestroyTexture(authorTexture);
    if (titleSurface) SDL_DestroySurface(titleSurface);
    if (authorSurface) SDL_DestroySurface(authorSurface);
    if (titleFont) TTF_CloseFont(titleFont);
    if (authorFont) TTF_CloseFont(authorFont);

    // Clean up splash screen
    SDL_DestroyRenderer(splashRenderer);
    SDL_DestroyWindow(splashWindow);
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char** argv)
{
    if (argc < 4 || argc > 5)
    {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM> [debug]\n";
        std::cerr << "  Scale: Display scale factor (1-20 recommended)\n";
        std::cerr << "  Delay: Cycle delay in milliseconds (recommended: 1-10)\n";
        std::cerr << "  ROM: Path to the CHIP-8 ROM file\n";
        std::cerr << "  debug: Optional - add 'debug' to enable debug window\n";
        std::exit(EXIT_FAILURE);
    }

    // Show splash screen
    std::cout << "CIPPOTTO v2.1 by VikSn0w" << std::endl;
    showSplashScreen();

    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    char const* romFilename = argv[3];
    bool enableDebug = (argc == 5 && std::string(argv[4]) == "debug");

    // Clamp video scale to reasonable values
    if (videoScale < 1) videoScale = 1;
    if (videoScale > 20) videoScale = 20;

    // Initialize main emulator window
    PlatformSDL platform("CHIP-8 Emulator (SDL)",
                        VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale,
                        VIDEO_WIDTH, VIDEO_HEIGHT);

    // Initialize CHIP-8 emulator
    chip8 chip8;
    chip8.LoadROM(romFilename);

    // Initialize debug window if requested
    std::unique_ptr<DebugSDL> debugWindow;
    if (enableDebug) {
        debugWindow = std::make_unique<DebugSDL>();
        if (debugWindow->Initialize("CHIP-8 Debugger", 1200, 800)) {
            std::cout << "Debug window enabled!" << std::endl;
            std::cout << "Debug Controls:" << std::endl;
            std::cout << "  F1-F6: Toggle debug sections" << std::endl;
            std::cout << "  F: Toggle follow PC mode" << std::endl;
            std::cout << "  Arrow Keys: Navigate memory" << std::endl;
            std::cout << "  Page Up/Down: Large memory navigation" << std::endl;
            std::cout << "  Home: Go to program start" << std::endl;
            std::cout << "  R: Reset to follow PC" << std::endl;
            std::cout << "  Tab/Escape: Toggle debug visibility" << std::endl;
        } else {
            std::cerr << "Failed to initialize debug window" << std::endl;
            debugWindow.reset();
        }
    }

    int videoPitch = sizeof(chip8.graphics[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    std::cout << "Starting emulation..." << std::endl;
    if (debugWindow) {
        std::cout << "Debug mode enabled - separate debug window is available" << std::endl;
    }

    while (!quit)
    {
        // Handle main window input
        quit = platform.ProcessInput(chip8.keypad);

        // Handle debug window events if enabled
        bool debugQuit = false;
        if (debugWindow && debugWindow->IsEnabled()) {
            debugQuit = debugWindow->HandleEvents();
            if (debugQuit) {
                // User closed debug window, but continue emulation
                debugWindow->SetEnabled(false);
                std::cout << "Debug window closed (emulation continues)" << std::endl;
            }
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;

            // Execute one CHIP-8 cycle
            chip8.emulateCycle();

            // Update main display
            platform.Update(chip8.graphics, videoPitch);

            // Update debug window if enabled
            if (debugWindow) {
                debugWindow->Update(&chip8);
            }
        }

        // Render debug window if enabled
        if (debugWindow && debugWindow->IsEnabled()) {
            debugWindow->Render();
        }

        // Reduce CPU usage but keep responsive
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }

    // Cleanup
    if (debugWindow) {
        debugWindow->Shutdown();
        std::cout << "Debug window shut down" << std::endl;
    }

    std::cout << "Emulation stopped. Goodbye!" << std::endl;
    return 0;
}