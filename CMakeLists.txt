cmake_minimum_required(VERSION 3.31)
project(CIPPOTTO)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your executable first
add_executable(CIPPOTTO
        main.cpp
        chip8.cpp
        chip8.h
        PlatformConsole.cpp
        PlatformConsole.h
        DebugConsoleManager.cpp
        DebugConsoleManager.h
)

# Copy assets

