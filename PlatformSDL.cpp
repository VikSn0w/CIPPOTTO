#include "PlatformSDL.h"
#include <iostream>

PlatformSDL::PlatformSDL(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    : window(nullptr), renderer(nullptr), texture(nullptr), textureWidth(textureWidth), textureHeight(textureHeight)
{
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Create window - SDL3 simplified API (no position parameters)
    window = SDL_CreateWindow(
        title,
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        cleanup();
        return;
    }

    // Create renderer - SDL3 API change: second parameter is now a const char* name (can be NULL)
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        cleanup();
        return;
    }

    // Disable any scaling/filtering on the renderer
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);

    // Create texture for CHIP-8 display
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        textureWidth,
        textureHeight
    );

    if (!texture) {
        std::cerr << "Unable to create texture! SDL Error: " << SDL_GetError() << std::endl;
        cleanup();
        return;
    }

    // Set texture to use nearest neighbor filtering (no blurring)
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    std::cout << "SDL initialized successfully" << std::endl;
    std::cout << "Window size: " << windowWidth << "x" << windowHeight << std::endl;
    std::cout << "Texture size: " << textureWidth << "x" << textureHeight << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  CHIP-8 Keypad -> Keyboard Mapping:" << std::endl;
    std::cout << "  1 2 3 C -> 1 2 3 4" << std::endl;
    std::cout << "  4 5 6 D -> Q W E R" << std::endl;
    std::cout << "  7 8 9 E -> A S D F" << std::endl;
    std::cout << "  A 0 B F -> Z X C V" << std::endl;
    std::cout << "  ESC to quit" << std::endl;
    std::cout << std::endl;
}

PlatformSDL::~PlatformSDL() {
    cleanup();
}

void PlatformSDL::cleanup() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void PlatformSDL::Update(void const* buffer, int pitch) {
    if (!renderer || !texture) {
        return;
    }

    const uint32_t* pixels = static_cast<const uint32_t*>(buffer);

    // Update texture with CHIP-8 display data
    void* texturePixels;
    int texturePitch;

    // SDL3: LockTexture returns bool instead of int
    if (SDL_LockTexture(texture, nullptr, &texturePixels, &texturePitch)) {
        uint32_t* targetPixels = static_cast<uint32_t*>(texturePixels);

        // Convert CHIP-8 pixels to RGBA format
        for (int y = 0; y < textureHeight; ++y) {
            for (int x = 0; x < textureWidth; ++x) {
                int sourceIndex = y * textureWidth + x;
                int targetIndex = y * (texturePitch / sizeof(uint32_t)) + x;

                // CHIP-8 pixel is on if non-zero, off if zero
                // Use pure white (0xFFFFFFFF) and pure black (0x000000FF) in RGBA format
                if (pixels[sourceIndex] != 0) {
                    targetPixels[targetIndex] = 0xFFFFFFFF; // White (RGBA: FF FF FF FF)
                } else {
                    targetPixels[targetIndex] = 0x000000FF; // Black (RGBA: 00 00 00 FF)
                }
            }
        }

        SDL_UnlockTexture(texture);
    }

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render texture - SDL3: RenderTexture instead of RenderCopy
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);

    // Present to screen
    SDL_RenderPresent(renderer);
}

bool PlatformSDL::ProcessInput(uint8_t* keys) {
    SDL_Event e;

    // Clear all keys first
    for (int i = 0; i < 16; ++i) {
        keys[i] = 0;
    }

    // Check for events
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            return true;
        }

        if (e.type == SDL_EVENT_KEY_DOWN) {
            // SDL3: e.key.key instead of e.key.keysym.sym
            if (e.key.key == SDLK_ESCAPE) {
                return true;
            }
        }
    }

    // Get current keyboard state
    const bool* keyState = SDL_GetKeyboardState(nullptr);

    // Map keys to CHIP-8 keypad
    // CHIP-8 keypad layout:
    // 1 2 3 C
    // 4 5 6 D
    // 7 8 9 E
    // A 0 B F

    if (keyState[SDL_SCANCODE_1]) keys[0x1] = 1;
    if (keyState[SDL_SCANCODE_2]) keys[0x2] = 1;
    if (keyState[SDL_SCANCODE_3]) keys[0x3] = 1;
    if (keyState[SDL_SCANCODE_4]) keys[0xC] = 1;

    if (keyState[SDL_SCANCODE_Q]) keys[0x4] = 1;
    if (keyState[SDL_SCANCODE_W]) keys[0x5] = 1;
    if (keyState[SDL_SCANCODE_E]) keys[0x6] = 1;
    if (keyState[SDL_SCANCODE_R]) keys[0xD] = 1;

    if (keyState[SDL_SCANCODE_A]) keys[0x7] = 1;
    if (keyState[SDL_SCANCODE_S]) keys[0x8] = 1;
    if (keyState[SDL_SCANCODE_D]) keys[0x9] = 1;
    if (keyState[SDL_SCANCODE_F]) keys[0xE] = 1;

    if (keyState[SDL_SCANCODE_Z]) keys[0xA] = 1;
    if (keyState[SDL_SCANCODE_X]) keys[0x0] = 1;
    if (keyState[SDL_SCANCODE_C]) keys[0xB] = 1;
    if (keyState[SDL_SCANCODE_V]) keys[0xF] = 1;

    return false;
}