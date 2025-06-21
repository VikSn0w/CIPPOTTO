#include "DebugSDL.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdarg>
#include <algorithm>

DebugSDL::DebugSDL() 
    : window(nullptr), renderer(nullptr), font(nullptr), smallFont(nullptr),
      enabled(false), initialized(false), chip8Ptr(nullptr),
      windowWidth(1200), windowHeight(800), fontSize(16), lineHeight(20),
      sectionPadding(10), columnWidth(280) {

    SetupColors();

    // Initialize memory view
    memoryView.startAddress = 0x200;
    memoryView.endAddress = 0x300;
    memoryView.bytesPerRow = 16;
    memoryView.followPC = true;
    memoryView.showAscii = true;

    // Initialize disassembly view
    disassemblyView.currentAddress = 0x200;
    disassemblyView.instructionsToShow = 20;
    disassemblyView.followPC = true;
}

DebugSDL::~DebugSDL() {
    Shutdown();
}
bool DebugSDL::Initialize(const char* title, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    if (!InitializeSDL()) {
        return false;
    }



    // Create window
    window = SDL_CreateWindow(title, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Failed to create debug window: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Failed to create debug renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!InitializeFonts()) {
        return false;
    }

    // Enable alpha blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SetupLayout();

    initialized = true;
    enabled = true;

    std::cout << "SDL Debug window initialized successfully" << std::endl;
    return true;
}

void DebugSDL::Shutdown() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    if (smallFont) {
        TTF_CloseFont(smallFont);
        smallFont = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    initialized = false;
    enabled = false;
}

bool DebugSDL::InitializeSDL() {
    // SDL3 should already be initialized by the main application
    return true;
}

bool DebugSDL::InitializeFonts() {
    // Initialize TTF
    if (TTF_Init() < 0) {
        std::cerr << "Failed to initialize TTF: " << SDL_GetError() << std::endl;
        return false;
    }

    // Try to load a monospace font - you can replace this path with your preferred font
    // Common system font paths
    const char* fontPaths[] = {
        // Windows
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
        // Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        // macOS
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Monaco.ttf",
        // Fallback - you might need to place a font file in your project directory
        "fonts/DejaVuSansMono.ttf",
        "DejaVuSansMono.ttf"
    };

    font = nullptr;
    for (const char* fontPath : fontPaths) {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) {
            std::cout << "Loaded font: " << fontPath << std::endl;
            loadedFontPath = fontPath;
            break;
        }
    }

    if (!font) {
        std::cerr << "Failed to load any font. Trying to create a built-in bitmap font fallback." << std::endl;

        // If no TTF font is available, we could implement a simple bitmap font
        // For now, let's continue without proper text rendering
        // This will still show rectangles, but at least the debug window will work
        return true;
    }

    // Load smaller font for details
    smallFont = TTF_OpenFont(loadedFontPath.c_str(), fontSize - 2);
    if (!smallFont) {
        smallFont = font; // Use same font if smaller version fails
    }

    return true;
}

void DebugSDL::SetupLayout() {
    CalculateLayout();
}

void DebugSDL::SetupColors() {
    bgColor = {20, 20, 25, 255};
    textColor = {220, 220, 220, 255};
    headerColor = {100, 150, 255, 255};
    highlightColor = {255, 255, 100, 255};
    borderColor = {80, 80, 90, 255};
    pcColor = {255, 100, 100, 255};
    activeColor = {100, 255, 100, 255};
}

// Add these helper functions to the DebugSDL class

// Ensure minimum section sizes
void DebugSDL::EnforceMinimumSizes() {
    const float minSectionWidth = 200.0f;
    const float minSectionHeight = 100.0f;

    // Enforce minimum widths
    if (registersSection.rect.w < minSectionWidth) registersSection.rect.w = minSectionWidth;
    if (memorySection.rect.w < minSectionWidth) memorySection.rect.w = minSectionWidth;
    if (disassemblySection.rect.w < minSectionWidth) disassemblySection.rect.w = minSectionWidth;
    if (graphicsSection.rect.w < minSectionWidth) graphicsSection.rect.w = minSectionWidth;

    // Enforce minimum heights
    if (registersSection.rect.h < minSectionHeight) registersSection.rect.h = minSectionHeight;
    if (memorySection.rect.h < minSectionHeight) memorySection.rect.h = minSectionHeight;
    if (disassemblySection.rect.h < minSectionHeight) disassemblySection.rect.h = minSectionHeight;
    if (stackSection.rect.h < minSectionHeight) stackSection.rect.h = minSectionHeight;
    if (keypadSection.rect.h < minSectionHeight) keypadSection.rect.h = minSectionHeight;
    if (graphicsSection.rect.h < minSectionHeight) graphicsSection.rect.h = minSectionHeight;
}

// Improved CalculateLayout with minimum size enforcement
void DebugSDL::CalculateLayout() {
    // Ensure minimum window size
    const int minWindowWidth = 800;
    const int minWindowHeight = 600;

    if (windowWidth < minWindowWidth) windowWidth = minWindowWidth;
    if (windowHeight < minWindowHeight) windowHeight = minWindowHeight;

    float padding = (float)sectionPadding;
    float totalWidth = (float)windowWidth;
    float totalHeight = (float)windowHeight;

    // Calculate available space
    float availableWidth = totalWidth - padding * 4.0f;
    float availableHeight = totalHeight - padding * 2.0f;

    // Define column widths
    float leftColumnWidth = availableWidth * 0.25f;
    float middleColumnWidth = availableWidth * 0.35f;
    float rightColumnWidth = availableWidth * 0.35f;

    // Left column sections
    float leftX = padding;
    float currentY = padding;

    // Registers section
    float registersHeight = std::max((float)lineHeight * 12.0f + padding, 150.0f);
    registersSection = {{leftX, currentY, leftColumnWidth, registersHeight}, "Registers & State", true, false};
    currentY += registersHeight + padding;

    // Keypad section
    float keypadHeight = std::max((float)lineHeight * 8.0f + padding, 180.0f);
    keypadSection = {{leftX, currentY, leftColumnWidth, keypadHeight}, "Keypad", true, false};
    currentY += keypadHeight + padding;

    // Stack section
    float stackHeight = std::max(totalHeight - currentY - padding, 100.0f);
    stackSection = {{leftX, currentY, leftColumnWidth, stackHeight}, "Stack", true, false};

    // Middle column sections
    float middleX = leftX + leftColumnWidth + padding;
    currentY = padding;

    // Memory section
    float memoryHeight = std::max(availableHeight * 0.6f, 200.0f);
    memorySection = {{middleX, currentY, middleColumnWidth, memoryHeight}, "Memory View", true, false};
    currentY += memoryHeight + padding;

    // Disassembly section
    float disassemblyHeight = std::max(totalHeight - currentY - padding, 150.0f);
    disassemblySection = {{middleX, currentY, middleColumnWidth, disassemblyHeight}, "Disassembly", true, false};

    // Right column
    float rightX = middleX + middleColumnWidth + padding;
    graphicsSection = {{rightX, padding, rightColumnWidth, availableHeight}, "Graphics Display", true, false};

    // Enforce minimum sizes
    EnforceMinimumSizes();
}


void DebugSDL::Update(chip8* emulator) {
    if (!enabled || !initialized) return;

    chip8Ptr = emulator;

    // Update memory view to follow PC if enabled
    if (memoryView.followPC && chip8Ptr) {
        uint16_t pc = chip8Ptr->program_counter;
        memoryView.startAddress = (pc >= 32) ? pc - 32 : 0;
        memoryView.endAddress = std::min(4096, (int)(pc + 64));
    }

    // Update disassembly view
    if (disassemblyView.followPC && chip8Ptr) {
        disassemblyView.currentAddress = chip8Ptr->program_counter;
    }
}

void DebugSDL::Render() {
    if (!enabled || !initialized || !chip8Ptr) return;

    RenderBackground();

    if (registersSection.visible) RenderRegisters();
    if (memorySection.visible) RenderMemory();
    if (stackSection.visible) RenderStack();
    if (disassemblySection.visible) RenderDisassembly();
    if (keypadSection.visible) RenderKeypad();
    if (graphicsSection.visible) RenderGraphics();

    SDL_RenderPresent(renderer);
}

bool DebugSDL::HandleEvents() {
    if (!enabled || !initialized) return false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                return true;

            case SDL_EVENT_KEY_DOWN:
                HandleKeyPress(e.key.key);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    HandleMouseClick((float)e.button.x, (float)e.button.y);
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                windowWidth = e.window.data1;
                windowHeight = e.window.data2;
                CalculateLayout();
                break;
        }
    }

    return false;
}

void DebugSDL::RenderBackground() {
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);
}

void DebugSDL::RenderSection(const DebugSection& section) {
    // Draw section border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_FRect borderRect = section.rect;
    SDL_RenderRect(renderer, &borderRect);

    // Draw section background
    SDL_SetRenderDrawColor(renderer, bgColor.r + 10, bgColor.g + 10, bgColor.b + 10, 200);
    SDL_FRect fillRect = {section.rect.x + 1.0f, section.rect.y + 1.0f, section.rect.w - 2.0f, section.rect.h - 2.0f};
    SDL_RenderFillRect(renderer, &fillRect);
}

void DebugSDL::RenderRegisters() {
    if (!chip8Ptr) return;

    RenderSection(registersSection);

    float x = registersSection.rect.x + 5.0f;
    float y = registersSection.rect.y + 5.0f;

    // Section header
    SDL_FRect headerRect = RenderSectionHeader("Registers & State", x, y, registersSection.rect.w - 10.0f);
    y += headerRect.h + 5.0f;

    // Program Counter
    RenderTextF(x, y, (chip8Ptr->program_counter >= 0x200) ? textColor : pcColor,
                "PC: 0x%04X", chip8Ptr->program_counter);
    y += (float)lineHeight;

    // Index Register
    RenderTextF(x, y, textColor, "I:  0x%04X", chip8Ptr->index_register);
    y += (float)lineHeight;

    // Stack Pointer
    RenderTextF(x, y, textColor, "SP: %02X", chip8Ptr->stack_pointer);
    y += (float)lineHeight;

    // Current Opcode
    RenderTextF(x, y, highlightColor, "OP: 0x%04X", chip8Ptr->opcode);
    y += (float)lineHeight * 1.5f;

    // Registers V0-VF
    RenderText("Registers V0-VF:", x, y, headerColor);
    y += (float)lineHeight;

    for (int i = 0; i < 16; i += 4) {
        RenderTextF(x, y, textColor, "V%X:%02X V%X:%02X V%X:%02X V%X:%02X",
                   i, chip8Ptr->registers_V[i],
                   i+1, chip8Ptr->registers_V[i+1],
                   i+2, chip8Ptr->registers_V[i+2],
                   i+3, chip8Ptr->registers_V[i+3]);
        y += (float)lineHeight;
    }

    y += (float)lineHeight * 0.5f;

    // Timers
    RenderText("Timers:", x, y, headerColor);
    y += (float)lineHeight;

    RenderTextF(x, y, (chip8Ptr->delay_timer > 0) ? activeColor : textColor,
                "Delay: %02X", chip8Ptr->delay_timer);
    y += (float)lineHeight;

    RenderTextF(x, y, (chip8Ptr->sound_timer > 0) ? activeColor : textColor,
                "Sound: %02X", chip8Ptr->sound_timer);
}

void DebugSDL::RenderMemory() {
    if (!chip8Ptr) return;

    RenderSection(memorySection);

    float x = memorySection.rect.x + 5.0f;
    float y = memorySection.rect.y + 5.0f;

    // Section header
    SDL_FRect headerRect = RenderSectionHeader("Memory View", x, y, memorySection.rect.w - 10.0f);
    y += headerRect.h + 5.0f;

    uint16_t pc = chip8Ptr->program_counter;
    uint16_t start = memoryView.startAddress;
    uint16_t end = std::min((int)memoryView.endAddress, 4096);

    for (uint16_t addr = start; addr < end; addr += memoryView.bytesPerRow) {
        SDL_Color addrColor = (addr == pc || addr == pc - 2) ? pcColor : textColor;

        // Address
        RenderTextF(x, y, addrColor, "%04X:", addr);

        // Hex bytes
        float hexX = x + 50.0f;
        for (int i = 0; i < memoryView.bytesPerRow && addr + i < end; i++) {
            uint8_t byte = chip8Ptr->memory[addr + i];
            SDL_Color byteColor = (addr + i == pc || addr + i == pc + 1) ? pcColor : textColor;

            RenderTextF(hexX + (float)i * 24.0f, y, byteColor, "%02X", byte);
        }

        // ASCII representation
        if (memoryView.showAscii) {
            float asciiX = hexX + (float)memoryView.bytesPerRow * 24.0f + 10.0f;
            std::string ascii = "";
            for (int i = 0; i < memoryView.bytesPerRow && addr + i < end; i++) {
                uint8_t byte = chip8Ptr->memory[addr + i];
                ascii += (byte >= 32 && byte <= 126) ? (char)byte : '.';
            }
            RenderText(ascii, asciiX, y, textColor);
        }

        y += (float)lineHeight;

        // Don't overflow the section
        if (y > memorySection.rect.y + memorySection.rect.h - (float)lineHeight) {
            break;
        }
    }
}

void DebugSDL::RenderStack() {
    if (!chip8Ptr) return;

    RenderSection(stackSection);

    float x = stackSection.rect.x + 5.0f;
    float y = stackSection.rect.y + 5.0f;

    // Section header
    SDL_FRect headerRect = RenderSectionHeader("Stack", x, y, stackSection.rect.w - 10.0f);
    y += headerRect.h + 5.0f;

    RenderTextF(x, y, textColor, "Stack Pointer: %d", chip8Ptr->stack_pointer);
    y += (float)lineHeight * 1.5f;

    // Show stack entries
    int startIdx = std::max(0, (int)chip8Ptr->stack_pointer - 8);
    int endIdx = std::min(16, (int)chip8Ptr->stack_pointer + 2);

    for (int i = endIdx - 1; i >= startIdx; i--) {
        SDL_Color stackColor = textColor;
        std::string prefix = "    ";

        if (i == chip8Ptr->stack_pointer - 1 && chip8Ptr->stack_pointer > 0) {
            stackColor = activeColor;
            prefix = " -> ";
        } else if (i >= chip8Ptr->stack_pointer) {
            stackColor = {100, 100, 100, 255};  // Dimmed for empty slots
        }

        if (i < chip8Ptr->stack_pointer) {
            RenderTextF(x, y, stackColor, "%s[%02d]: 0x%04X", prefix.c_str(), i, chip8Ptr->stack[i]);
        } else {
            RenderTextF(x, y, stackColor, "%s[%02d]: ----", prefix.c_str(), i);
        }

        y += (float)lineHeight;
    }
}

void DebugSDL::RenderDisassembly() {
    if (!chip8Ptr) return;

    RenderSection(disassemblySection);

    float x = disassemblySection.rect.x + 5.0f;
    float y = disassemblySection.rect.y + 5.0f;

    // Section header
    SDL_FRect headerRect = RenderSectionHeader("Disassembly", x, y, disassemblySection.rect.w - 10.0f);
    y += headerRect.h + 5.0f;

    uint16_t pc = chip8Ptr->program_counter;
    uint16_t addr = (pc >= 20) ? pc - 20 : 0x200;

    for (int i = 0; i < disassemblyView.instructionsToShow && addr < 4096 - 1; i++) {
        uint16_t opcode = (chip8Ptr->memory[addr] << 8) | chip8Ptr->memory[addr + 1];
        std::string instruction = DisassembleInstruction(opcode, addr);

        SDL_Color instrColor = (addr == pc) ? pcColor : textColor;
        std::string prefix = (addr == pc) ? ">> " : "   ";

        RenderTextF(x, y, instrColor, "%s%04X: %04X  %s",
                   prefix.c_str(), addr, opcode, instruction.c_str());

        y += (float)lineHeight;
        addr += 2;

        // Don't overflow the section
        if (y > disassemblySection.rect.y + disassemblySection.rect.h - (float)lineHeight) {
            break;
        }
    }
}

void DebugSDL::RenderKeypad() {
    if (!chip8Ptr) return;

    RenderSection(keypadSection);

    float x = keypadSection.rect.x + 5.0f;
    float y = keypadSection.rect.y + 5.0f;

    // Section header
    SDL_FRect headerRect = RenderSectionHeader("Keypad", x, y, keypadSection.rect.w - 10.0f);
    y += headerRect.h + 5.0f;

    RenderText("CHIP-8 Keypad Layout:", x, y, headerColor);
    y += (float)lineHeight * 1.5f;

    // Draw keypad layout
    const char* keyLayout[4][4] = {
        {"1", "2", "3", "C"},
        {"4", "5", "6", "D"},
        {"7", "8", "9", "E"},
        {"A", "0", "B", "F"}
    };

    const int keyIndices[4][4] = {
        {1, 2, 3, 0xC},
        {4, 5, 6, 0xD},
        {7, 8, 9, 0xE},
        {0xA, 0, 0xB, 0xF}
    };

    float keySize = 30.0f;
    float keySpacing = 35.0f;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float keyX = x + (float)col * keySpacing;
            float keyY = y + (float)row * keySpacing;

            SDL_FRect keyRect = {keyX, keyY, keySize, keySize};

            int keyIdx = keyIndices[row][col];
            bool pressed = chip8Ptr->keypad[keyIdx] != 0;

            // Key background
            if (pressed) {
                SDL_SetRenderDrawColor(renderer, activeColor.r, activeColor.g, activeColor.b, 200);
            } else {
                SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
            }
            SDL_RenderFillRect(renderer, &keyRect);

            // Key border
            SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, 255);
            SDL_RenderRect(renderer, &keyRect);

            // Key label
            SDL_Color keyColor = pressed ? SDL_Color{0, 0, 0, 255} : textColor;
            RenderText(keyLayout[row][col], keyX + 10.0f, keyY + 8.0f, keyColor);
        }
    }
}

void DebugSDL::RenderGraphics() {
    if (!chip8Ptr) return;

    RenderSection(graphicsSection);

    float x = graphicsSection.rect.x + 5.0f;
    float y = graphicsSection.rect.y + 5.0f;

    // Section header
    SDL_FRect headerRect = RenderSectionHeader("Graphics Display", x, y, graphicsSection.rect.w - 10.0f);
    y += headerRect.h + 5.0f;

    // Calculate display area
    float availableWidth = graphicsSection.rect.w - 10.0f;
    float availableHeight = graphicsSection.rect.h - headerRect.h - 15.0f;

    float scaleX = availableWidth / VIDEO_WIDTH;
    float scaleY = availableHeight / VIDEO_HEIGHT;
    float scale = std::min(scaleX, scaleY);

    float displayWidth = VIDEO_WIDTH * scale;
    float displayHeight = VIDEO_HEIGHT * scale;

    // Center the display
    float displayX = x + (availableWidth - displayWidth) / 2.0f;
    float displayY = y + (availableHeight - displayHeight) / 2.0f;

    // Draw CHIP-8 display
    for (int py = 0; py < VIDEO_HEIGHT; py++) {
        for (int px = 0; px < VIDEO_WIDTH; px++) {
            SDL_FRect pixelRect = {
                displayX + (float)px * scale,
                displayY + (float)py * scale,
                scale,
                scale
            };

            uint32_t pixel = chip8Ptr->graphics[py * VIDEO_WIDTH + px];

            if (pixel != 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White for on pixels
            } else {
                SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);    // Dark for off pixels
            }

            SDL_RenderFillRect(renderer, &pixelRect);
        }
    }

    // Draw border around display
    SDL_FRect displayBorder = {displayX - 1.0f, displayY - 1.0f, displayWidth + 2.0f, displayHeight + 2.0f};
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, 255);
    SDL_RenderRect(renderer, &displayBorder);
}

void DebugSDL::RenderText(const std::string& text, float x, float y, SDL_Color color) {
    if (!font || text.empty() || !renderer) {
        return; // Don't render rectangles as fallback
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), text.length(), color);
    if (!textSurface) {
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_DestroySurface(textSurface);

    if (!textTexture) {
        return;
    }

    float w, h;
    SDL_GetTextureSize(textTexture, &w, &h);
    SDL_FRect destRect = {x, y, w, h};
    SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
    SDL_DestroyTexture(textTexture);
}

SDL_Texture* DebugSDL::CreateTextTexture(const std::string& text, SDL_Color color, TTF_Font* useFont) {
    if (!useFont) useFont = font;
    if (!useFont || !renderer || text.empty()) return nullptr;

    // Use TTF_RenderText_Solid for SDL3_ttf
    SDL_Surface* textSurface = TTF_RenderText_Solid(useFont, text.c_str(), text.length(), color);
    if (!textSurface) {
        // Optional: Log the error
        // std::cerr << "Failed to create text surface: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_DestroySurface(textSurface);

    if (!textTexture) {
        // Optional: Log the error
        // std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
    }

    return textTexture;
}

void DebugSDL::RenderTextF(float x, float y, SDL_Color color, const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    RenderText(std::string(buffer), x, y, color);
}

void DebugSDL::RenderTextTexture(SDL_Texture* texture, float x, float y) {
    if (!texture) return;

    float w, h;
    if (SDL_GetTextureSize(texture, &w, &h) != 0) {
        // Use fallback dimensions instead of spamming errors
        w = 100.0f; // Reasonable default width
        h = (float)lineHeight; // Use line height as default height

        // Optional: Log error only once if you need debugging
        // static bool errorLogged = false;
        // if (!errorLogged) {
        //     std::cerr << "Warning: Could not get texture size, using fallback dimensions" << std::endl;
        //     errorLogged = true;
        // }
    }

    SDL_FRect destRect = {x, y, w, h};
    SDL_RenderTexture(renderer, texture, nullptr, &destRect);
}

SDL_FRect DebugSDL::GetTextSize(const std::string& text, TTF_Font* useFont) {
    if (!useFont) useFont = font;
    if (!useFont) return {0, 0, (float)text.length() * 8.0f, 12.0f}; // Fallback size

    int w, h;
    // Use TTF_GetStringSize for SDL3_ttf (returns bool, true on success)
    if (!TTF_GetStringSize(useFont, text.c_str(), text.length(), &w, &h)) {
        // Fallback to estimated size if TTF_GetStringSize fails
        return {0, 0, (float)text.length() * 8.0f, (float)lineHeight};
    }

    return {0, 0, (float)w, (float)h};
}

SDL_FRect DebugSDL::RenderSectionHeader(const std::string& title, float x, float y, float width, bool collapsed) {
    SDL_FRect headerRect = {x, y, width, (float)lineHeight + 4.0f};

    // Header background
    SDL_SetRenderDrawColor(renderer, headerColor.r, headerColor.g, headerColor.b, 100);
    SDL_RenderFillRect(renderer, &headerRect);

    // Header border
    SDL_SetRenderDrawColor(renderer, headerColor.r, headerColor.g, headerColor.b, 255);
    SDL_RenderRect(renderer, &headerRect);

    // Header text
    RenderText(title, x + 5.0f, y + 2.0f, headerColor);

    return headerRect;
}

std::string DebugSDL::FormatHex(uint16_t value, int width) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << value;
    return ss.str();
}

std::string DebugSDL::FormatByte(uint8_t value) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)value;
    return ss.str();
}

std::string DebugSDL::DisassembleInstruction(uint16_t opcode, uint16_t address) {
    std::stringstream ss;

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: ss << "CLS"; break;
                case 0x00EE: ss << "RET"; break;
                default: ss << "SYS " << FormatHex(opcode & 0x0FFF, 3); break;
            }
            break;
        case 0x1000: ss << "JP " << FormatHex(opcode & 0x0FFF, 3); break;
        case 0x2000: ss << "CALL " << FormatHex(opcode & 0x0FFF, 3); break;
        case 0x3000: ss << "SE V" << std::hex << ((opcode & 0x0F00) >> 8) << ", " << FormatByte(opcode & 0x00FF); break;
        case 0x4000: ss << "SNE V" << std::hex << ((opcode & 0x0F00) >> 8) << ", " << FormatByte(opcode & 0x00FF); break;
        case 0x5000: ss << "SE V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
        case 0x6000: ss << "LD V" << std::hex << ((opcode & 0x0F00) >> 8) << ", " << FormatByte(opcode & 0x00FF); break;
        case 0x7000: ss << "ADD V" << std::hex << ((opcode & 0x0F00) >> 8) << ", " << FormatByte(opcode & 0x00FF); break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: ss << "LD V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
                case 0x0001: ss << "OR V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
                case 0x0002: ss << "AND V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
                case 0x0003: ss << "XOR V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
                case 0x0004: ss << "ADD V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
                case 0x0005: ss << "SUB V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
                case 0x0006: ss << "SHR V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x0007: ss << "SUBN V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
                case 0x000E: ss << "SHL V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                default: ss << "UNKNOWN 8xxx"; break;
            }
            break;
        case 0x9000: ss << "SNE V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4); break;
        case 0xA000: ss << "LD I, " << FormatHex(opcode & 0x0FFF, 3); break;
        case 0xB000: ss << "JP V0, " << FormatHex(opcode & 0x0FFF, 3); break;
        case 0xC000: ss << "RND V" << std::hex << ((opcode & 0x0F00) >> 8) << ", " << FormatByte(opcode & 0x00FF); break;
        case 0xD000: ss << "DRW V" << std::hex << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4) << ", " << (opcode & 0x000F); break;
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E: ss << "SKP V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x00A1: ss << "SKNP V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                default: ss << "UNKNOWN Exxx"; break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: ss << "LD V" << std::hex << ((opcode & 0x0F00) >> 8) << ", DT"; break;
                case 0x000A: ss << "LD V" << std::hex << ((opcode & 0x0F00) >> 8) << ", K"; break;
                case 0x0015: ss << "LD DT, V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x0018: ss << "LD ST, V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x001E: ss << "ADD I, V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x0029: ss << "LD F, V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x0033: ss << "LD B, V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x0055: ss << "LD [I], V" << std::hex << ((opcode & 0x0F00) >> 8); break;
                case 0x0065: ss << "LD V" << std::hex << ((opcode & 0x0F00) >> 8) << ", [I]"; break;
                default: ss << "UNKNOWN Fxxx"; break;
            }
            break;
        default:
            ss << "UNKNOWN";
            break;
    }

    return ss.str();
}

std::vector<std::string> DebugSDL::GetMemoryDump(uint16_t start, uint16_t end, int bytesPerRow) {
    if (!chip8Ptr) return {};

    std::vector<std::string> lines;

    for (uint16_t addr = start; addr < end; addr += bytesPerRow) {
        std::stringstream line;
        line << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << addr << ": ";

        // Hex bytes
        for (int i = 0; i < bytesPerRow && addr + i < end; i++) {
            line << std::setfill('0') << std::setw(2) << (int)chip8Ptr->memory[addr + i] << " ";
        }

        // Padding for incomplete rows
        for (int i = (end - addr); i < bytesPerRow; i++) {
            line << "   ";
        }

        line << " ";

        // ASCII representation
        for (int i = 0; i < bytesPerRow && addr + i < end; i++) {
            uint8_t byte = chip8Ptr->memory[addr + i];
            line << (char)((byte >= 32 && byte <= 126) ? byte : '.');
        }

        lines.push_back(line.str());
    }

    return lines;
}

void DebugSDL::HandleMouseClick(float x, float y) {
    // Check if click is on section headers to toggle collapse/expand
    if (IsPointInRect(x, y, {registersSection.rect.x, registersSection.rect.y, registersSection.rect.w, (float)lineHeight + 4.0f})) {
        registersSection.collapsed = !registersSection.collapsed;
    }
    else if (IsPointInRect(x, y, {memorySection.rect.x, memorySection.rect.y, memorySection.rect.w, (float)lineHeight + 4.0f})) {
        memorySection.collapsed = !memorySection.collapsed;
    }
    else if (IsPointInRect(x, y, {stackSection.rect.x, stackSection.rect.y, stackSection.rect.w, (float)lineHeight + 4.0f})) {
        stackSection.collapsed = !stackSection.collapsed;
    }
    else if (IsPointInRect(x, y, {disassemblySection.rect.x, disassemblySection.rect.y, disassemblySection.rect.w, (float)lineHeight + 4.0f})) {
        disassemblySection.collapsed = !disassemblySection.collapsed;
    }
    else if (IsPointInRect(x, y, {keypadSection.rect.x, keypadSection.rect.y, keypadSection.rect.w, (float)lineHeight + 4.0f})) {
        keypadSection.collapsed = !keypadSection.collapsed;
    }
    else if (IsPointInRect(x, y, {graphicsSection.rect.x, graphicsSection.rect.y, graphicsSection.rect.w, (float)lineHeight + 4.0f})) {
        graphicsSection.collapsed = !graphicsSection.collapsed;
    }

    // Handle memory view clicks - could implement address jumping
    if (IsPointInRect(x, y, memorySection.rect) && !memorySection.collapsed) {
        // Could implement memory navigation here
        // For example, clicking on an address could center the view on that address
    }

    // Handle disassembly clicks - could implement breakpoints
    if (IsPointInRect(x, y, disassemblySection.rect) && !disassemblySection.collapsed) {
        // Could implement breakpoint setting here
        // Calculate which instruction was clicked and set/clear breakpoint
    }
}

void DebugSDL::HandleKeyPress(SDL_Keycode key) {
    switch (key) {
        case SDLK_F1:
            registersSection.visible = !registersSection.visible;
            break;
        case SDLK_F2:
            memorySection.visible = !memorySection.visible;
            break;
        case SDLK_F3:
            stackSection.visible = !stackSection.visible;
            break;
        case SDLK_F4:
            disassemblySection.visible = !disassemblySection.visible;
            break;
        case SDLK_F5:
            keypadSection.visible = !keypadSection.visible;
            break;
        case SDLK_F6:
            graphicsSection.visible = !graphicsSection.visible;
            break;
        case SDLK_F:
            // Toggle follow PC for memory view
            memoryView.followPC = !memoryView.followPC;
            disassemblyView.followPC = !disassemblyView.followPC;
            break;
        case SDLK_UP:
            // Scroll memory up
            if (memoryView.startAddress >= 16) {
                memoryView.startAddress -= 16;
                memoryView.endAddress -= 16;
                memoryView.followPC = false;
            }
            break;
        case SDLK_DOWN:
            // Scroll memory down
            if (memoryView.endAddress < 4096 - 16) {
                memoryView.startAddress += 16;
                memoryView.endAddress += 16;
                memoryView.followPC = false;
            }
            break;
        case SDLK_PAGEUP:
            // Scroll memory up by larger amount
            if (memoryView.startAddress >= 256) {
                memoryView.startAddress -= 256;
                memoryView.endAddress -= 256;
                memoryView.followPC = false;
            }
            break;
        case SDLK_PAGEDOWN:
            // Scroll memory down by larger amount
            if (memoryView.endAddress < 4096 - 256) {
                memoryView.startAddress += 256;
                memoryView.endAddress += 256;
                memoryView.followPC = false;
            }
            break;
        case SDLK_HOME:
            // Go to beginning of program memory
            memoryView.startAddress = 0x200;
            memoryView.endAddress = 0x300;
            memoryView.followPC = false;
            break;
        case SDLK_R:
            // Reset memory view to follow PC
            memoryView.followPC = true;
            disassemblyView.followPC = true;
            break;
        case SDLK_ESCAPE:
        case SDLK_TAB:
            // Toggle debug window visibility
            enabled = !enabled;
            break;
        default:
            break;
    }
}

bool DebugSDL::IsPointInRect(float x, float y, const SDL_FRect& rect) {
    return (x >= rect.x && x < rect.x + rect.w &&
            y >= rect.y && y < rect.y + rect.h);
}

void DebugSDL::ToggleSection(const std::string& sectionName) {
    if (sectionName == "registers") {
        registersSection.visible = !registersSection.visible;
    }
    else if (sectionName == "memory") {
        memorySection.visible = !memorySection.visible;
    }
    else if (sectionName == "stack") {
        stackSection.visible = !stackSection.visible;
    }
    else if (sectionName == "disassembly") {
        disassemblySection.visible = !disassemblySection.visible;
    }
    else if (sectionName == "keypad") {
        keypadSection.visible = !keypadSection.visible;
    }
    else if (sectionName == "graphics") {
        graphicsSection.visible = !graphicsSection.visible;
    }
}

void DebugSDL::SetMemoryView(uint16_t start, uint16_t end, int bytesPerRow) {
    memoryView.startAddress = start;
    memoryView.endAddress = end;
    memoryView.bytesPerRow = bytesPerRow;
    memoryView.followPC = false;  // Disable auto-follow when manually setting view
}