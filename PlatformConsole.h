#ifndef PLATFORMCONSOLE_H
#define PLATFORMCONSOLE_H

#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
#endif

class PlatformConsole {
public:
    PlatformConsole(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
    ~PlatformConsole();

    void Update(void const* buffer, int pitch);
    bool ProcessInput(uint8_t* keys);

private:
    int textureWidth;
    int textureHeight;
    std::vector<uint8_t> displayBuffer;

#ifdef _WIN32
    HANDLE hConsole;
    DWORD originalConsoleMode;
#else
    struct termios originalTermios;
#endif

    void setupConsole();
    void restoreConsole();
    void clearScreen();
    char getKey();
    void drawDisplay();
};

#endif //PLATFORMCONSOLE_H