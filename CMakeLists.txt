cmake_minimum_required(VERSION 3.31)
project(CIPPOTTO)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your executable first
add_executable(CIPPOTTO
        main.cpp
        chip8.cpp
        chip8.h
        Platform.cpp
        Platform.h
        PlatformConsole.cpp
        PlatformConsole.h
        DebugConsoleManager.cpp
        DebugConsoleManager.h
)

# Set SFML paths
set(SFML_ROOT c:/SFML)
set(SFML_DIR c:/SFML/lib/cmake/SFML)

# Manual SFML 3.0 setup (skip find_package)
target_include_directories(CIPPOTTO PRIVATE c:/SFML/include)
target_link_directories(CIPPOTTO PRIVATE c:/SFML/lib)

# Link SFML 3.0 libraries manually
target_link_libraries(CIPPOTTO PRIVATE
        sfml-system
        sfml-window
        sfml-graphics
        opengl32    # Required for graphics
        winmm       # Required for system
        gdi32       # Required for window
)

# Copy assets
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Copy SFML DLLs to build directory
if(WIN32)
    file(GLOB SFML_DLLS "c:/SFML/bin/*.dll")
    if(SFML_DLLS)
        file(COPY ${SFML_DLLS} DESTINATION ${CMAKE_BINARY_DIR})
    endif()

    # Copy MinGW DLLs if needed
    file(GLOB MINGW_DEP_DLLS "C:/mingw64/bin/*.dll")
    if(MINGW_DEP_DLLS)
        file(COPY ${MINGW_DEP_DLLS} DESTINATION ${CMAKE_BINARY_DIR})
    endif()
endif()