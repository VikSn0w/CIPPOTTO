#include "PlatformConsole.h"

PlatformConsole::PlatformConsole(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    : textureWidth(textureWidth), textureHeight(textureHeight),
      displayBuffer(textureWidth * textureHeight, 0)
{
    std::cout << "=== " << title << " ===" << std::endl;
    std::cout << "Display: " << textureWidth << "x" << textureHeight << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  CHIP-8 Keypad -> Keyboard Mapping:" << std::endl;
    std::cout << "  1 2 3 C -> 1 2 3 4" << std::endl;
    std::cout << "  4 5 6 D -> Q W E R" << std::endl;
    std::cout << "  7 8 9 E -> A S D F" << std::endl;
    std::cout << "  A 0 B F -> Z X C V" << std::endl;
    std::cout << "  ESC to quit" << std::endl;
    std::cout << std::endl;

    setupConsole();
    clearScreen();
}

PlatformConsole::~PlatformConsole() {
    restoreConsole();
}

void PlatformConsole::Update(void const* buffer, int pitch) {
    const uint32_t* pixels = static_cast<const uint32_t*>(buffer);

    for (int i = 0; i < textureWidth * textureHeight; ++i) {
        displayBuffer[i] = (pixels[i] != 0) ? 1 : 0;
    }

    clearScreen();
    drawDisplay();
}

bool PlatformConsole::ProcessInput(uint8_t* keys) {
    for (int i = 0; i < 16; ++i) {
        keys[i] = 0;
    }

    char key = getKey();

    if (key == 27 || key == 'q' || key == 'Q') {
        return true;
    }

    switch (key) {
        case '1': keys[0x1] = 1; break;
        case '2': keys[0x2] = 1; break;
        case '3': keys[0x3] = 1; break;
        case '4': keys[0xC] = 1; break;
        case 'q': case 'Q': keys[0x4] = 1; break;
        case 'w': case 'W': keys[0x5] = 1; break;
        case 'e': case 'E': keys[0x6] = 1; break;
        case 'r': case 'R': keys[0xD] = 1; break;
        case 'a': case 'A': keys[0x7] = 1; break;
        case 's': case 'S': keys[0x8] = 1; break;
        case 'd': case 'D': keys[0x9] = 1; break;
        case 'f': case 'F': keys[0xE] = 1; break;
        case 'z': case 'Z': keys[0xA] = 1; break;
        case 'x': case 'X': keys[0x0] = 1; break;
        case 'c': case 'C': keys[0xB] = 1; break;
        case 'v': case 'V': keys[0xF] = 1; break;
    }

    return false;
}

void PlatformConsole::setupConsole() {
#ifdef _WIN32
    hConsole = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hConsole, &originalConsoleMode);
    SetConsoleMode(hConsole, originalConsoleMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
    tcgetattr(STDIN_FILENO, &originalTermios);

    struct termios newTermios = originalTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO);
    newTermios.c_cc[VMIN] = 0;
    newTermios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    std::cout << "\033[?25l";
#endif
}

void PlatformConsole::restoreConsole() {
#ifdef _WIN32
    SetConsoleMode(hConsole, originalConsoleMode);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);

    std::cout << "\033[?25h";
#endif
}

void PlatformConsole::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
}

char PlatformConsole::getKey() {
#ifdef _WIN32
    if (_kbhit()) {
        return _getch();
    }
    return 0;
#else
    char key;
    if (read(STDIN_FILENO, &key, 1) == 1) {
        return key;
    }
    return 0;
#endif
}

void PlatformConsole::drawDisplay() {
    std::cout << "+";
    for (int x = 0; x < textureWidth; ++x) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl;

    for (int y = 0; y < textureHeight; ++y) {
        std::cout << "|";
        for (int x = 0; x < textureWidth; ++x) {
            int pixelIndex = y * textureWidth + x;
            if (displayBuffer[pixelIndex]) {
                std::cout << char(219);
            } else {
                std::cout << " ";
            }
        }
        std::cout << "|" << std::endl;
    }

    std::cout << "+";
    for (int x = 0; x < textureWidth; ++x) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl;

    std::cout << "Press keys to play, ESC/Q to quit" << std::endl;
}