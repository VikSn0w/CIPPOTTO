#ifndef SDL_DEBUG_H
#define SDL_DEBUG_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>  // Add TTF support
#include <string>
#include <vector>
#include <memory>
#include "chip8.h"

struct DebugSection {
    SDL_FRect rect;
    std::string title;
    bool visible;
    bool collapsed;
};

struct MemoryView {
    uint16_t startAddress;
    uint16_t endAddress;
    int bytesPerRow;
    bool followPC;
    bool showAscii;
};

struct DisassemblyView {
    uint16_t currentAddress;
    int instructionsToShow;
    bool followPC;
    std::vector<std::string> disassembledInstructions;
};

class DebugSDL {
public:
    DebugSDL();
    ~DebugSDL();

    bool Initialize(const char* title, int width = 1200, int height = 800);
    void Shutdown();

    bool IsEnabled() const { return enabled; }
    void SetEnabled(bool enable) { enabled = enable; }

    void Update(chip8* emulator);
    void Render();
    bool HandleEvents();

    // Configuration
    void ToggleSection(const std::string& sectionName);
    void SetMemoryView(uint16_t start, uint16_t end, int bytesPerRow = 16);

private:
    // SDL Resources
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;           // Add font support
    TTF_Font* smallFont;      // For smaller text
    std::string loadedFontPath;

    // State
    bool enabled;
    bool initialized;
    chip8* chip8Ptr;

    // UI Layout
    int windowWidth, windowHeight;
    int fontSize;
    int lineHeight;
    int sectionPadding;
    int columnWidth;

    // Debug sections
    DebugSection registersSection;
    DebugSection memorySection;
    DebugSection stackSection;
    DebugSection disassemblySection;
    DebugSection keypadSection;
    DebugSection graphicsSection;

    // Views
    MemoryView memoryView;
    DisassemblyView disassemblyView;

    // Colors
    SDL_Color bgColor;
    SDL_Color textColor;
    SDL_Color headerColor;
    SDL_Color highlightColor;
    SDL_Color borderColor;
    SDL_Color pcColor;
    SDL_Color activeColor;

    // Helper methods
    bool InitializeSDL();
    bool InitializeFonts();   // Add font initialization
    void SetupLayout();
    void SetupColors();

    void EnforceMinimumSizes();

    // Rendering methods
    void RenderBackground();
    void RenderSection(const DebugSection& section);
    void RenderRegisters();
    void RenderMemory();
    void RenderStack();
    void RenderDisassembly();
    void RenderKeypad();
    void RenderGraphics();

    // Text rendering helpers (now with proper TTF support)
    void RenderText(const std::string& text, float x, float y, SDL_Color color = {255, 255, 255, 255});
    void RenderTextF(float x, float y, SDL_Color color, const char* format, ...);
    SDL_FRect RenderSectionHeader(const std::string& title, float x, float y, float width, bool collapsed = false);

    // Additional text helpers
    SDL_Texture* CreateTextTexture(const std::string& text, SDL_Color color, TTF_Font* useFont = nullptr);
    void RenderTextTexture(SDL_Texture* texture, float x, float y);
    SDL_FRect GetTextSize(const std::string& text, TTF_Font* useFont = nullptr);

    // Utility methods
    std::string FormatHex(uint16_t value, int width = 4);
    std::string FormatByte(uint8_t value);
    std::string DisassembleInstruction(uint16_t opcode, uint16_t address);
    std::vector<std::string> GetMemoryDump(uint16_t start, uint16_t end, int bytesPerRow);

    // Input handling
    void HandleMouseClick(float x, float y);
    void HandleKeyPress(SDL_Keycode key);

    // Layout calculations
    void CalculateLayout();
    bool IsPointInRect(float x, float y, const SDL_FRect& rect);
};

#endif // SDL_DEBUG_H