cmake_minimum_required(VERSION 3.31)
project(CIPPOTTO)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your executable
add_executable(CIPPOTTO
        main.cpp
        chip8.cpp
        chip8.h
        PlatformSDL.cpp
        PlatformSDL.h
        DebugSDL.cpp
        DebugSDL.h
)

# Configurable SDL3 setup with fallback defaults
set(SDL3_ROOT "${SDL3_ROOT}" CACHE PATH "Path to SDL3 installation")
set(SDL3_TTF_ROOT "${SDL3_TTF_ROOT}" CACHE PATH "Path to SDL3_ttf installation")

# Set default paths if not provided
if(NOT SDL3_ROOT)
    if(WIN32)
        set(SDL3_ROOT "C:/SDL3")
    else()
        set(SDL3_ROOT "/usr/local")
    endif()
endif()

if(NOT SDL3_TTF_ROOT)
    if(WIN32)
        set(SDL3_TTF_ROOT "C:/SDL3_ttf")
    else()
        set(SDL3_TTF_ROOT "/usr/local")
    endif()
endif()

# Add include directories
target_include_directories(CIPPOTTO PRIVATE "${SDL3_ROOT}/include")
target_include_directories(CIPPOTTO PRIVATE "${SDL3_TTF_ROOT}/include")

if(WIN32)
    # Find SDL3 libraries
    find_library(SDL3_LIBRARY
            NAMES SDL3 libSDL3
            PATHS "${SDL3_ROOT}/lib"
            NO_DEFAULT_PATH
    )

    find_library(SDL3MAIN_LIBRARY
            NAMES SDL3main libSDL3main
            PATHS "${SDL3_ROOT}/lib"
            NO_DEFAULT_PATH
    )

    # Find SDL3_ttf library
    find_library(SDL3_TTF_LIBRARY
            NAMES SDL3_ttf libSDL3_ttf
            PATHS "${SDL3_TTF_ROOT}/lib"
            NO_DEFAULT_PATH
    )

    # Build library list based on what was found
    set(LINK_LIBRARIES)

    if(SDL3_LIBRARY)
        list(APPEND LINK_LIBRARIES ${SDL3_LIBRARY})
        message(STATUS "Found SDL3 library: ${SDL3_LIBRARY}")
    else()
        # Fallback: add library directory and use library name directly
        target_link_directories(CIPPOTTO PRIVATE "${SDL3_ROOT}/lib")
        list(APPEND LINK_LIBRARIES SDL3)
        message(WARNING "SDL3 library not found, using fallback linking")
    endif()

    if(SDL3MAIN_LIBRARY)
        list(APPEND LINK_LIBRARIES ${SDL3MAIN_LIBRARY})
        message(STATUS "Found SDL3main library: ${SDL3MAIN_LIBRARY}")
    else()
        message(STATUS "SDL3main library not found, skipping")
    endif()

    if(SDL3_TTF_LIBRARY)
        list(APPEND LINK_LIBRARIES ${SDL3_TTF_LIBRARY})
        message(STATUS "Found SDL3_ttf library: ${SDL3_TTF_LIBRARY}")
    else()
        # Fallback: add library directory and use library name directly
        target_link_directories(CIPPOTTO PRIVATE "${SDL3_TTF_ROOT}/lib")
        list(APPEND LINK_LIBRARIES SDL3_ttf)
        message(WARNING "SDL3_ttf library not found, using fallback linking")
    endif()

    # Link all libraries
    target_link_libraries(CIPPOTTO PRIVATE
            ${LINK_LIBRARIES}
            kernel32
            user32
    )

    # Copy SDL3 DLLs to build directory
    file(GLOB SDL3_DLLS "${SDL3_ROOT}/bin/*.dll")
    if(SDL3_DLLS)
        foreach(DLL ${SDL3_DLLS})
            get_filename_component(DLL_NAME ${DLL} NAME)
            configure_file(${DLL} ${CMAKE_BINARY_DIR}/${DLL_NAME} COPYONLY)
        endforeach()
        message(STATUS "Copied SDL3 DLLs to build directory")
    else()
        message(WARNING "No SDL3 DLLs found in ${SDL3_ROOT}/bin/")
    endif()

    # Copy SDL3_ttf DLLs to build directory
    file(GLOB SDL3_TTF_DLLS "${SDL3_TTF_ROOT}/bin/*.dll")
    if(SDL3_TTF_DLLS)
        foreach(DLL ${SDL3_TTF_DLLS})
            get_filename_component(DLL_NAME ${DLL} NAME)
            configure_file(${DLL} ${CMAKE_BINARY_DIR}/${DLL_NAME} COPYONLY)
        endforeach()
        message(STATUS "Copied SDL3_ttf DLLs to build directory")
    else()
        message(WARNING "No SDL3_ttf DLLs found in ${SDL3_TTF_ROOT}/bin/")
    endif()

else()
    # Unix-like systems (Linux, macOS, etc.)
    # Try to find libraries using pkg-config first, then fallback to manual search
    find_package(PkgConfig QUIET)

    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SDL3 QUIET sdl3)
        pkg_check_modules(SDL3_TTF QUIET SDL3_ttf)
    endif()

    # Manual search if pkg-config didn't work
    if(NOT SDL3_FOUND)
        find_library(SDL3_LIBRARY
                NAMES SDL3 libSDL3
                PATHS "${SDL3_ROOT}/lib" /usr/lib /usr/local/lib
        )
    endif()

    if(NOT SDL3_TTF_FOUND)
        find_library(SDL3_TTF_LIBRARY
                NAMES SDL3_ttf libSDL3_ttf
                PATHS "${SDL3_TTF_ROOT}/lib" /usr/lib /usr/local/lib
        )
    endif()

    # Link libraries
    if(SDL3_FOUND)
        target_link_libraries(CIPPOTTO PRIVATE ${SDL3_LIBRARIES})
        target_include_directories(CIPPOTTO PRIVATE ${SDL3_INCLUDE_DIRS})
        target_compile_options(CIPPOTTO PRIVATE ${SDL3_CFLAGS_OTHER})
    else()
        target_link_libraries(CIPPOTTO PRIVATE ${SDL3_LIBRARY})
    endif()

    if(SDL3_TTF_FOUND)
        target_link_libraries(CIPPOTTO PRIVATE ${SDL3_TTF_LIBRARIES})
        target_include_directories(CIPPOTTO PRIVATE ${SDL3_TTF_INCLUDE_DIRS})
        target_compile_options(CIPPOTTO PRIVATE ${SDL3_TTF_CFLAGS_OTHER})
    else()
        target_link_libraries(CIPPOTTO PRIVATE ${SDL3_TTF_LIBRARY})
    endif()
endif()

# Enable console subsystem on Windows (for debug output)
if(WIN32)
    set_property(TARGET CIPPOTTO PROPERTY WIN32_EXECUTABLE FALSE)
endif()

# Print configuration summary
message(STATUS "CIPPOTTO Configuration Summary:")
message(STATUS "  SDL3_ROOT: ${SDL3_ROOT}")
message(STATUS "  SDL3_TTF_ROOT: ${SDL3_TTF_ROOT}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
if(WIN32)
    message(STATUS "  Console Mode: Enabled")
endif()